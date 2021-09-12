// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"    // IWYU pragma: associated
#include "1_Internal.hpp"  // IWYU pragma: associated
#include "rpc/RPC.tpp"     // IWYU pragma: associated

#include <chrono>
#include <future>
#include <string>
#include <type_traits>
#include <vector>

#include "opentxs/Pimpl.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/api/client/Blockchain.hpp"
#include "opentxs/api/client/Contacts.hpp"
#include "opentxs/api/client/Manager.hpp"
#include "opentxs/api/client/OTX.hpp"
#include "opentxs/api/network/Blockchain.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/api/storage/Storage.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/node/Manager.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/crypto/PaymentCode.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/rpc/PaymentType.hpp"
#include "opentxs/rpc/ResponseCode.hpp"
#include "opentxs/rpc/request/Base.hpp"
#include "opentxs/rpc/request/SendPayment.hpp"
#include "opentxs/rpc/response/Base.hpp"
#include "opentxs/rpc/response/SendPayment.hpp"
#include "rpc/RPC.hpp"

namespace opentxs::rpc::implementation
{
auto RPC::send_payment(const request::Base& base) const noexcept
    -> std::unique_ptr<response::Base>
{
    const auto& in = base.asSendPayment();
    const auto reply = [&](const auto code) {
        return std::make_unique<response::SendPayment>(
            in, response::Base::Responses{{0, code}}, response::Base::Tasks{});
    };

    try {
        const auto& api = client_session(base);

        switch (in.PaymentType()) {
            case PaymentType::blockchain: {

                return send_payment_blockchain(api, in);
            }
            case PaymentType::cheque:
            case PaymentType::transfer:
            case PaymentType::voucher:
            case PaymentType::invoice:
            case PaymentType::blinded: {

                return send_payment_custodial(api, in);
            }
            case PaymentType::error:
            default: {

                return reply(ResponseCode::invalid);
            }
        }
    } catch (...) {

        return reply(ResponseCode::bad_session);
    }
}

auto RPC::send_payment_blockchain(
    const api::client::Manager& api,
    const request::SendPayment& in) const noexcept
    -> std::unique_ptr<response::Base>
{
    auto tasks = response::Base::Tasks{};
    const auto reply = [&](const auto code) {
        return std::make_unique<response::SendPayment>(
            in, response::Base::Responses{{0, code}}, std::move(tasks));
    };

    const auto id = api.Factory().Identifier(in.SourceAccount());
    const auto& blockchain = api.Blockchain();
    const auto data = blockchain.LookupAccount(id);
    const auto& [chain, owner] = data;

    if (blockchain::Type::Unknown == chain) {
        return reply(ResponseCode::account_not_found);
    }

    api.Network().Blockchain().Start(chain);

    try {
        auto future = [&] {
            const auto& [chain, owner] = data;
            const auto amount = in.Amount();
            const auto& address = in.DestinationAccount();
            const auto& memo = in.Memo();
            const auto& network = api.Network().Blockchain().GetChain(chain);
            const auto recipient = api.Factory().PaymentCode(address);

            if (0 < recipient->Version()) {
                return network.SendToPaymentCode(
                    owner, recipient, amount, memo);
            } else {
                return network.SendToAddress(owner, address, amount, memo);
            }
        }();
        const auto [code, txid] = future.get();

        if (txid->empty()) { return reply(ResponseCode::transaction_failed); }

        tasks.emplace_back(0, txid->asHex());

        return reply(ResponseCode::txid);
    } catch (...) {

        return reply(ResponseCode::transaction_failed);
    }
}

auto RPC::send_payment_custodial(
    const api::client::Manager& api,
    const request::SendPayment& in) const noexcept
    -> std::unique_ptr<response::Base>
{
    const auto contact = api.Factory().Identifier(in.RecipientContact());
    const auto source = api.Factory().Identifier(in.SourceAccount());
    auto tasks = response::Base::Tasks{};
    const auto reply = [&](const auto code) {
        return std::make_unique<response::SendPayment>(
            in, response::Base::Responses{{0, code}}, std::move(tasks));
    };

    if (contact->empty()) { return reply(ResponseCode::invalid); }

    if (auto c = api.Contacts().Contact(contact); false == bool(c)) {

        return reply(ResponseCode::contact_not_found);
    }

    const auto sender = api.Storage().AccountOwner(source);

    if (sender->empty()) {

        return reply(ResponseCode::account_owner_not_found);
    }

    const auto& otx = api.OTX();

    switch (otx.CanMessage(sender, contact)) {
        case Messagability::MISSING_CONTACT:
        case Messagability::CONTACT_LACKS_NYM:
        case Messagability::NO_SERVER_CLAIM:
        case Messagability::INVALID_SENDER:
        case Messagability::MISSING_SENDER: {

            return reply(ResponseCode::no_path_to_recipient);
        }
        case Messagability::MISSING_RECIPIENT:
        case Messagability::UNREGISTERED: {

            return reply(ResponseCode::retry);
        }
        case Messagability::READY:
        default: {
        }
    }

    switch (in.PaymentType()) {
        case PaymentType::cheque: {
            auto [taskID, future] = otx.SendCheque(
                sender,
                source,
                contact,
                in.Amount(),
                in.Memo(),
                Clock::now(),
                Clock::now() + std::chrono::hours(OT_CHEQUE_HOURS));

            if (0 == taskID) { return reply(ResponseCode::error); }

            tasks.emplace_back(
                0,
                queue_task(
                    api,
                    sender,
                    std::to_string(taskID),
                    [&](const auto& in, auto& out) -> void {
                        evaluate_send_payment_cheque(in, out);
                    },
                    std::move(future)));

            return reply(ResponseCode::queued);
        }
        case PaymentType::transfer: {
            const auto destination =
                api.Factory().Identifier(in.DestinationAccount());
            const auto notary = api.Storage().AccountServer(source);
            auto [taskID, future] = otx.SendTransfer(
                sender, notary, source, destination, in.Amount(), in.Memo());

            if (0 == taskID) { return reply(ResponseCode::error); }

            tasks.emplace_back(
                0,
                queue_task(
                    api,
                    sender,
                    std::to_string(taskID),
                    [&](const auto& in, auto& out) -> void {
                        evaluate_send_payment_transfer(api, in, out);
                    },
                    std::move(future)));

            return reply(ResponseCode::queued);
        }
        case PaymentType::voucher:
        case PaymentType::invoice:
        case PaymentType::blinded:
        default: {
            return std::make_unique<response::SendPayment>(
                in,
                response::Base::Responses{{0, ResponseCode::unimplemented}},
                response::Base::Tasks{});
        }
    }
}
}  // namespace opentxs::rpc::implementation
