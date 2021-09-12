// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"    // IWYU pragma: associated
#include "1_Internal.hpp"  // IWYU pragma: associated
#include "ui/blockchainaccountstatus/BlockchainAccountStatus.hpp"  // IWYU pragma: associated

#include <algorithm>
#include <chrono>
#include <exception>
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <sstream>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include "opentxs/api/Endpoints.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/api/HDSeed.hpp"
#include "opentxs/api/client/Blockchain.hpp"
#include "opentxs/api/client/Manager.hpp"
#include "opentxs/api/network/Blockchain.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/crypto/Account.hpp"
#include "opentxs/blockchain/crypto/HD.hpp"
#include "opentxs/blockchain/crypto/PaymentCode.hpp"
#include "opentxs/blockchain/crypto/Subaccount.hpp"
#include "opentxs/blockchain/crypto/SubaccountType.hpp"
#include "opentxs/blockchain/crypto/Subchain.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/blockchain/node/HeaderOracle.hpp"
#include "opentxs/blockchain/node/Manager.hpp"
#include "opentxs/core/Flag.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"
#include "opentxs/core/crypto/PaymentCode.hpp"
#include "opentxs/network/zeromq/Frame.hpp"
#include "opentxs/network/zeromq/FrameSection.hpp"
#include "opentxs/network/zeromq/Pipeline.hpp"
#include "opentxs/protobuf/HDPath.pb.h"
#include "ui/base/List.hpp"

#define OT_METHOD "opentxs::ui::implementation::BlockchainAccountStatus::"

namespace opentxs::factory
{
auto BlockchainAccountStatusModel(
    const api::client::Manager& api,
    const ui::implementation::BlockchainAccountStatusPrimaryID& id,
    const blockchain::Type chain,
    const SimpleCallback& cb) noexcept
    -> std::unique_ptr<ui::internal::BlockchainAccountStatus>
{
    using ReturnType = ui::implementation::BlockchainAccountStatus;

    return std::make_unique<ReturnType>(api, id, chain, cb);
}
}  // namespace opentxs::factory

namespace opentxs::ui::implementation
{
BlockchainAccountStatus::BlockchainAccountStatus(
    const api::client::Manager& api,
    const BlockchainAccountStatusPrimaryID& id,
    const blockchain::Type chain,
    const SimpleCallback& cb) noexcept
    : BlockchainAccountStatusType(api, id, cb, false)
    , Worker(api, std::chrono::milliseconds{100})
    , chain_(chain)
{
    init_executor({
        api.Endpoints().BlockchainAccountCreated(),
        api.Endpoints().BlockchainReorg(),
        api.Endpoints().BlockchainScanProgress(),
    });
    pipeline_->Push(MakeWork(Work::init));
}

auto BlockchainAccountStatus::add_children(ChildMap&& map) noexcept -> void
{
    add_items([&] {
        auto rows = ChildDefinitions{};

        for (auto& [sourceType, sourceMap] : map) {
            for (auto& it : sourceMap) {
                auto& [sourceID, data] = it;
                auto& [sourceName, custom] = data;
                rows.emplace_back(
                    std::move(sourceID),
                    std::make_pair(sourceType, std::move(sourceName)),
                    CustomData{},
                    std::move(custom));
            }
        }

        return rows;
    }());
}

auto BlockchainAccountStatus::construct_row(
    const BlockchainAccountStatusRowID& id,
    const BlockchainAccountStatusSortKey& index,
    CustomData& custom) const noexcept -> RowPointer
{
    return factory::BlockchainSubaccountSourceWidget(
        *this, Widget::api_, id, index, custom);
}

auto BlockchainAccountStatus::load() noexcept -> void
{
    try {
        auto map = [&] {
            auto out = ChildMap{};
            const auto& api = Widget::api_;
            const auto& account = api.Blockchain().Account(primary_id_, chain_);
            // TODO imported accounts

            for (const auto& subaccountID : account.GetHD().all()) {
                populate(
                    account,
                    subaccountID,
                    blockchain::crypto::SubaccountType::HD,
                    blockchain::crypto::Subchain::Error,  // NOTE: all subchains
                    out);
            }

            for (const auto& subaccountID : account.GetPaymentCode().all()) {
                populate(
                    account,
                    subaccountID,
                    blockchain::crypto::SubaccountType::PaymentCode,
                    blockchain::crypto::Subchain::Error,  // NOTE: all subchains
                    out);
            }

            return out;
        }();
        add_children(std::move(map));
    } catch (const std::exception& e) {
        LogOutput(OT_METHOD)(__func__)(": ")(e.what()).Flush();
    }
}

auto BlockchainAccountStatus::pipeline(const Message& in) noexcept -> void
{
    if (false == running_.get()) { return; }

    const auto body = in.Body();

    if (1 > body.size()) {
        LogOutput(OT_METHOD)(__func__)(": Invalid message").Flush();

        OT_FAIL;
    }

    const auto work = [&] {
        try {

            return body.at(0).as<Work>();
        } catch (...) {

            OT_FAIL;
        }
    }();

    if ((false == startup_complete()) && (Work::init != work)) {
        pipeline_->Push(in);

        return;
    }

    switch (work) {
        case Work::shutdown: {
            running_->Off();
            shutdown(shutdown_promise_);
        } break;
        case Work::newaccount: {
            process_account(in);
        } break;
        case Work::header:
        case Work::reorg: {
            process_reorg(in);
        } break;
        case Work::progress: {
            process_progress(in);
        } break;
        case Work::init: {
            startup();
        } break;
        case Work::statemachine: {
            do_work();
        } break;
        default: {
            LogOutput(OT_METHOD)(__func__)(": Unhandled type").Flush();

            OT_FAIL;
        }
    }
}

auto BlockchainAccountStatus::populate(
    const blockchain::crypto::Account& account,
    const Identifier& subaccountID,
    const blockchain::crypto::SubaccountType type,
    const blockchain::crypto::Subchain subchain,
    ChildMap& out) const noexcept -> void
{
    const auto& api = Widget::api_;

    switch (type) {
        case blockchain::crypto::SubaccountType::HD: {
            const auto& hd = account.GetHD();
            const auto& subaccount = hd.at(subaccountID);
            const auto path = subaccount.Path();
            populate(
                subaccount,
                api.Factory().Identifier(path.root()),
                api.Seeds().SeedDescription(path.root()),
                subaccount.Name(),
                subchain,
                out[type]);
        } break;
        case blockchain::crypto::SubaccountType::PaymentCode: {
            const auto& pc = account.GetPaymentCode();
            const auto& subaccount = pc.at(subaccountID);
            populate(
                subaccount,
                subaccount.Local().ID(),
                subaccount.Local().asBase58() + " (local)",
                subaccount.Remote().asBase58() + " (remote)",
                subchain,
                out[type]);
        } break;
        case blockchain::crypto::SubaccountType::Imported: {
            // TODO
        } break;
        case blockchain::crypto::SubaccountType::Notification: {
            // TODO
        } break;
        default: {

            OT_FAIL;
        }
    }
}

auto BlockchainAccountStatus::populate(
    const blockchain::crypto::Subaccount& node,
    const Identifier& sourceID,
    const std::string& sourceDescription,
    const std::string& subaccountName,
    const blockchain::crypto::Subchain subchain,
    SubaccountMap& out) const noexcept -> void
{
    auto& data = out[sourceID];
    auto& [sourceText, cantCapture] = data;

    if (sourceText.empty()) { sourceText = sourceDescription; }

    auto& subaccounts = [&]() -> auto&
    {
        using Subaccounts = std::vector<BlockchainSubaccountSourceRowData>;
        auto* ptr = [&] {
            auto& custom = data.second;

            if (0u == custom.size()) {

                return custom.emplace_back(
                    std::make_unique<Subaccounts>().release());
            } else {
                OT_ASSERT(1u == custom.size());

                return custom.front();
            }
        }();

        OT_ASSERT(nullptr != ptr);

        return *reinterpret_cast<Subaccounts*>(ptr);
    }
    ();
    using Subchains = std::vector<BlockchainSubaccountRowData>;
    auto& subaccount = subaccounts.emplace_back(
        node.ID(), subaccountName, CustomData{}, CustomData{});
    auto& subchainData = [&]() -> auto&
    {
        auto& children = subaccount.children_;

        OT_ASSERT(0u == children.size());

        auto& ptr =
            children.emplace_back(std::make_unique<Subchains>().release());

        OT_ASSERT(1u == children.size());
        OT_ASSERT(nullptr != ptr);

        return *reinterpret_cast<Subchains*>(ptr);
    }
    ();
    const auto subchainList = [&]() -> std::set<blockchain::crypto::Subchain> {
        if (blockchain::crypto::Subchain::Error == subchain) {

            return node.AllowedSubchains();
        }

        return {subchain};
    }();

    for (const auto subtype : subchainList) {
        auto [name, progress] = subchain_display_name(node, subtype);
        subchainData.emplace_back(
            subtype, std::move(name), std::move(progress), CustomData{});
    }
}

auto BlockchainAccountStatus::process_account(const Message& in) noexcept
    -> void
{
    const auto& api = Widget::api_;
    auto body = in.Body();

    OT_ASSERT(4 < body.size());

    const auto chain = body.at(1).as<blockchain::Type>();

    if (chain != chain_) { return; }

    const auto owner = [&] {
        auto out = api.Factory().Identifier();
        out->Assign(body.at(2).Bytes());

        return out;
    }();

    if (owner != primary_id_) { return; }

    const auto type = body.at(3).as<blockchain::crypto::SubaccountType>();
    const auto subaccountID = [&] {
        auto out = api.Factory().Identifier();
        out->Assign(body.at(4).Bytes());

        return out;
    }();
    const auto& account = api.Blockchain().Account(primary_id_, chain_);
    auto out = ChildMap{};

    try {
        auto map = [&] {
            auto out = ChildMap{};
            populate(
                account,
                subaccountID,
                type,
                blockchain::crypto::Subchain::Error,  // NOTE: all subchains
                out);

            return out;
        }();
        add_children(std::move(map));
    } catch (const std::exception& e) {
        LogOutput(OT_METHOD)(__func__)(": ")(e.what()).Flush();
    }
}

auto BlockchainAccountStatus::process_progress(const Message& in) noexcept
    -> void
{
    const auto& api = Widget::api_;
    auto body = in.Body();

    OT_ASSERT(5 < body.size());

    const auto chain = body.at(1).as<blockchain::Type>();

    if (chain != chain_) { return; }

    const auto owner = [&] {
        auto out = api.Factory().Identifier();
        out->Assign(body.at(2).Bytes());

        return out;
    }();

    if (owner != primary_id_) { return; }

    const auto type = body.at(3).as<blockchain::crypto::SubaccountType>();
    const auto subaccountID = [&] {
        auto out = api.Factory().Identifier();
        out->Assign(body.at(4).Bytes());

        return out;
    }();
    const auto subchain = body.at(5).as<blockchain::crypto::Subchain>();
    const auto& account = api.Blockchain().Account(primary_id_, chain_);
    auto out = ChildMap{};

    try {
        auto map = [&] {
            auto out = ChildMap{};
            populate(account, subaccountID, type, subchain, out);

            return out;
        }();
        add_children(std::move(map));
    } catch (const std::exception& e) {
        LogOutput(OT_METHOD)(__func__)(": ")(e.what()).Flush();
    }
}

auto BlockchainAccountStatus::process_reorg(const Message& in) noexcept -> void
{
    const auto body = in.Body();

    OT_ASSERT(1 < body.size());

    const auto chain = body.at(1).as<blockchain::Type>();

    if (chain != chain_) { return; }

    load();
}

auto BlockchainAccountStatus::startup() noexcept -> void
{
    load();
    finish_startup();
    trigger();
}

auto BlockchainAccountStatus::subchain_display_name(
    const blockchain::crypto::Subaccount& node,
    BlockchainSubaccountRowID subchain) const noexcept
    -> std::pair<BlockchainSubaccountSortKey, CustomData>
{
    auto out = std::pair<BlockchainSubaccountSortKey, CustomData>{};
    auto& nameOut = out.first;
    auto& progressOut =
        *static_cast<std::string*>(out.second.emplace_back(new std::string{}));
    auto name = std::stringstream{};
    auto progress = std::stringstream{};
    using Height = blockchain::block::Height;
    const auto target = [&]() -> std::optional<Height> {
        try {
            const auto& api = Widget::api_;
            const auto& chain =
                api.Network().Blockchain().GetChain(node.Parent().Chain());

            return chain.HeaderOracle().BestChain().first;
        } catch (...) {

            return std::nullopt;
        }
    }();
    const auto scanned = [&]() -> std::optional<Height> {
        try {

            return node.ScanProgress(subchain).first;
        } catch (...) {

            return std::nullopt;
        }
    }();
    const auto effective = std::max<Height>(0, scanned.value_or(0));
    const auto eTarget = std::max<Height>(1, target.value_or(1));
    const auto eProgress = std::min(effective, eTarget);
    const auto percent = [&] {
        auto out = std::stringstream{};

        if (target.has_value()) {
            out << std::to_string((100 * eProgress) / eTarget);
        } else {
            out << "?";
        }

        out << " %";

        return out.str();
    }();
    name << opentxs::print(subchain) << " subchain";
    progress << std::to_string(eProgress);
    progress << " of ";

    if (target.has_value()) {
        progress << std::to_string(eTarget);
    } else {
        progress << "?";
    }

    progress << " (";
    progress << percent;
    progress << ')';
    name << ": " << progress.str();
    nameOut = name.str();
    progressOut = progress.str();

    return out;
}

BlockchainAccountStatus::~BlockchainAccountStatus() = default;
}  // namespace opentxs::ui::implementation
