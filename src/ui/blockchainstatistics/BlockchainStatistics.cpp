// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"    // IWYU pragma: associated
#include "1_Internal.hpp"  // IWYU pragma: associated
#include "ui/blockchainstatistics/BlockchainStatistics.hpp"  // IWYU pragma: associated

#include <cstddef>
#include <future>
#include <map>
#include <memory>
#include <set>
#include <vector>

#include "opentxs/Types.hpp"
#include "opentxs/api/Endpoints.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/api/client/Manager.hpp"
#include "opentxs/api/network/Blockchain.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/node/BlockOracle.hpp"
#include "opentxs/blockchain/node/FilterOracle.hpp"
#include "opentxs/blockchain/node/HeaderOracle.hpp"
#include "opentxs/blockchain/node/Manager.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Flag.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"
#include "opentxs/network/zeromq/Frame.hpp"
#include "opentxs/network/zeromq/FrameSection.hpp"
#include "opentxs/network/zeromq/Pipeline.hpp"
#include "ui/base/List.hpp"

#define OT_METHOD "opentxs::ui::implementation::BlockchainStatistics::"

namespace zmq = opentxs::network::zeromq;

namespace opentxs::factory
{
auto BlockchainStatisticsModel(
    const api::client::Manager& api,
    const SimpleCallback& cb) noexcept
    -> std::unique_ptr<ui::internal::BlockchainStatistics>
{
    using ReturnType = ui::implementation::BlockchainStatistics;

    return std::make_unique<ReturnType>(api, cb);
}
}  // namespace opentxs::factory

namespace opentxs::ui::implementation
{
BlockchainStatistics::BlockchainStatistics(
    const api::client::Manager& api,
    const SimpleCallback& cb) noexcept
    : BlockchainStatisticsList(api, api.Factory().Identifier(), cb, false)
    , Worker(api, {})
    , blockchain_(api.Network().Blockchain())
{
    init_executor({
        api.Endpoints().BlockchainBlockDownloadQueue(),
        api.Endpoints().BlockchainNewFilter(),
        api.Endpoints().BlockchainPeer(),
        api.Endpoints().BlockchainPeerConnection(),
        api.Endpoints().BlockchainReorg(),
        api.Endpoints().BlockchainStateChange(),
        api.Endpoints().BlockchainWalletUpdated(),
    });
    pipeline_->Push(MakeWork(Work::init));
}

auto BlockchainStatistics::construct_row(
    const BlockchainStatisticsRowID& id,
    const BlockchainStatisticsSortKey& index,
    CustomData& custom) const noexcept -> RowPointer
{
    return factory::BlockchainStatisticsItem(
        *this, Widget::api_, id, index, custom);
}

auto BlockchainStatistics::custom(
    const BlockchainStatisticsRowID& chain) const noexcept -> CustomData
{
    // NOTE:
    //  0: header oracle height
    //  1: filter oracle height
    //  2: connected peer count
    //  3: active peer count
    //  4: block download queue
    //  5: balance
    auto out = CustomData{};

    try {
        const auto& network = blockchain_.GetChain(chain);
        const auto& header = network.HeaderOracle();
        const auto& filter = network.FilterOracle();
        const auto& block = network.BlockOracle();
        out.emplace_back(
            new blockchain::block::Height{header.BestChain().first});
        out.emplace_back(new blockchain::block::Height{
            filter.FilterTip(filter.DefaultType()).first});
        out.emplace_back(new std::size_t{network.GetPeerCount()});
        out.emplace_back(new std::size_t{network.GetVerifiedPeerCount()});
        out.emplace_back(new std::size_t{block.DownloadQueue()});
        out.emplace_back(new blockchain::Amount{network.GetBalance().second});
    } catch (...) {
        out.emplace_back(new blockchain::block::Height{-1});
        out.emplace_back(new blockchain::block::Height{-1});
        out.emplace_back(new std::size_t{0});
        out.emplace_back(new std::size_t{0});
        out.emplace_back(new std::size_t{0});
        out.emplace_back(new blockchain::Amount{0});
    }

    return out;
}

auto BlockchainStatistics::pipeline(const Message& in) noexcept -> void
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

    switch (work) {
        case Work::shutdown: {
            running_->Off();
            shutdown(shutdown_promise_);
        } break;
        case Work::balance:
        case Work::blockheader:
        case Work::activepeer:
        case Work::reorg:
        case Work::filter:
        case Work::block:
        case Work::connectedpeer: {
            process_work(in);
        } break;
        case Work::statechange: {
            process_state(in);
        } break;
        case Work::init: {
            startup();
        } break;
        case Work::statemachine: {
            do_work();
        } break;
        default: {
            LogOutput(OT_METHOD)(__func__)(": Unhandled type: ")(
                static_cast<OTZMQWorkType>(work))
                .Flush();

            OT_FAIL;
        }
    }
}

auto BlockchainStatistics::process_chain(
    BlockchainStatisticsRowID chain) noexcept -> void
{
    auto data = custom(chain);
    add_item(chain, blockchain::DisplayString(chain), data);
    delete_inactive(blockchain_.EnabledChains());
}

auto BlockchainStatistics::process_state(const Message& in) noexcept -> void
{
    const auto body = in.Body();

    OT_ASSERT(2 < body.size());

    process_chain(body.at(1).as<blockchain::Type>());
}

auto BlockchainStatistics::process_work(const Message& in) noexcept -> void
{
    const auto body = in.Body();

    OT_ASSERT(1 < body.size());

    process_chain(body.at(1).as<blockchain::Type>());
}

auto BlockchainStatistics::startup() noexcept -> void
{
    for (const auto& chain : blockchain_.EnabledChains()) {
        process_chain(chain);
    }

    finish_startup();
}

BlockchainStatistics::~BlockchainStatistics()
{
    wait_for_startup();
    stop_worker().get();
}
}  // namespace opentxs::ui::implementation
