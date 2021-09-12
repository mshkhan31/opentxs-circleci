// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                          // IWYU pragma: associated
#include "1_Internal.hpp"                        // IWYU pragma: associated
#include "blockchain/node/wallet/Proposals.hpp"  // IWYU pragma: associated

#include <chrono>
#include <deque>
#include <functional>
#include <map>
#include <mutex>
#include <optional>
#include <set>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include "blockchain/node/wallet/BitcoinTransactionBuilder.hpp"
#include "internal/blockchain/block/bitcoin/Bitcoin.hpp"
#include "internal/blockchain/crypto/Crypto.hpp"
#include "internal/blockchain/node/Node.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/Core.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/api/client/Blockchain.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/SendResult.hpp"
#include "opentxs/blockchain/crypto/PaymentCode.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"
#include "opentxs/core/crypto/PaymentCode.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/protobuf/BlockchainTransaction.pb.h"
#include "opentxs/protobuf/BlockchainTransactionProposal.pb.h"
#include "opentxs/protobuf/BlockchainTransactionProposedNotification.pb.h"
#include "util/ScopeGuard.hpp"

#define OT_METHOD "opentxs::blockchain::node::wallet::Proposals::"

namespace opentxs::blockchain::node::wallet
{
struct Proposals::Imp {
public:
    auto Add(const Proposal& tx, std::promise<SendOutcome>&& promise)
        const noexcept -> void
    {
        auto lock = Lock{lock_};
        auto id = api_.Factory().Identifier(tx.id());

        if (false == db_.AddProposal(id, tx)) {
            LogOutput(OT_METHOD)(__func__)(": Database error").Flush();
            static const auto blank = api_.Factory().Data();
            promise.set_value({SendResult::DatabaseError, blank});
        }

        pending_.Add(std::move(id), std::move(promise));
    }
    auto Run() noexcept -> bool
    {
        auto lock = Lock{lock_};
        send(lock);
        rebroadcast(lock);
        cleanup(lock);

        return pending_.HasData();
    }

    Imp(const api::Core& api,
        const api::client::Blockchain& crypto,
        const node::internal::Network& node,
        const node::internal::WalletDatabase& db,
        const Type chain) noexcept
        : api_(api)
        , crypto_(crypto)
        , node_(node)
        , db_(db)
        , chain_(chain)
        , lock_()
        , pending_(api_)
        , confirming_()
    {
        for (const auto& serialized : db_.LoadProposals()) {
            auto id = api_.Factory().Identifier(serialized.id());

            if (serialized.has_finished()) {
                confirming_.emplace(std::move(id), Time{});
            } else {
                pending_.Add(std::move(id), {});
            }
        }
    }

private:
    enum class BuildResult : std::int8_t {
        PermanentFailure = -1,
        Success = 0,
        TemporaryFailure = 1,
    };

    using Builder = std::function<BuildResult(
        const Identifier& id,
        Proposal&,
        std::promise<SendOutcome>&)>;
    using Promise = std::promise<SendOutcome>;
    using Data = std::pair<OTIdentifier, Promise>;

    struct Pending {
        auto Exists(const Identifier& id) const noexcept -> bool
        {
            auto lock = Lock{lock_};

            return 0 < ids_.count(id);
        }
        auto HasData() const noexcept -> bool
        {
            auto lock = Lock{lock_};

            return 0 < data_.size();
        }

        auto Add(
            OTIdentifier&& id,
            std::promise<SendOutcome>&& promise) noexcept -> void
        {
            auto lock = Lock{lock_};

            if (0 < ids_.count(id)) {
                LogOutput(OT_METHOD)(__func__)(": Proposal already exists")
                    .Flush();
                static const auto blank = api_.Factory().Data();
                promise.set_value({SendResult::DuplicateProposal, blank});
            }

            ids_.emplace(id);
            data_.emplace_back(std::move(id), std::move(promise));
        }
        auto Add(Data&& job) noexcept -> void
        {
            auto lock = Lock{lock_};
            const auto& [id, promise] = job;

            OT_ASSERT(0 == ids_.count(id));

            ids_.emplace(id);
            data_.emplace_back(std::move(job));
        }
        auto Delete(const Identifier& id) noexcept -> void
        {
            auto lock = Lock{lock_};
            auto copy = OTIdentifier{id};

            if (0 < ids_.count(copy)) {
                ids_.erase(copy);

                for (auto i{data_.begin()}; i != data_.end();) {
                    if (i->first == copy) {
                        i = data_.erase(i);
                    } else {
                        ++i;
                    }
                }
            }
        }
        auto Pop() noexcept -> Data
        {
            auto lock = Lock{lock_};
            auto post = ScopeGuard{[&] { data_.pop_front(); }};

            return std::move(data_.front());
        }

        Pending(const api::Core& api) noexcept
            : api_(api)
            , lock_()
            , data_()
            , ids_()
        {
        }

    private:
        const api::Core& api_;
        mutable std::mutex lock_;
        std::deque<Data> data_;
        std::set<OTIdentifier> ids_;
    };

    const api::Core& api_;
    const api::client::Blockchain& crypto_;
    const node::internal::Network& node_;
    const node::internal::WalletDatabase& db_;
    const Type chain_;
    mutable std::mutex lock_;
    mutable Pending pending_;
    mutable std::map<OTIdentifier, Time> confirming_;

    static auto is_expired(const Proposal& tx) noexcept -> bool
    {
        return Clock::now() > Clock::from_time_t(tx.expires());
    }

    auto build_transaction_bitcoin(
        const Identifier& id,
        Proposal& proposal,
        std::promise<SendOutcome>& promise) const noexcept -> BuildResult
    {
        static const auto blank = api_.Factory().Data();
        auto output = BuildResult::Success;
        auto rc = SendResult::UnspecifiedError;
        auto txid{blank};
        auto builder = BitcoinTransactionBuilder{
            api_, crypto_, db_, id, proposal, chain_, node_.FeeRate()};
        auto post = ScopeGuard{[&] {
            switch (output) {
                case BuildResult::TemporaryFailure: {
                } break;
                case BuildResult::PermanentFailure: {
                    builder.ReleaseKeys();
                    [[fallthrough]];
                }
                case BuildResult::Success:
                default: {

                    promise.set_value({rc, txid});
                }
            }
        }};

        if (false == builder.CreateOutputs(proposal)) {
            LogOutput(OT_METHOD)(__func__)(": Failed to create outputs")
                .Flush();
            output = BuildResult::PermanentFailure;
            rc = SendResult::OutputCreationError;

            return output;
        }

        if (false == builder.AddChange(proposal)) {
            LogOutput(OT_METHOD)(__func__)(": Failed to allocate change output")
                .Flush();
            output = BuildResult::PermanentFailure;
            rc = SendResult::ChangeError;

            return output;
        }

        while (false == builder.IsFunded()) {
            using Spend = node::internal::WalletDatabase::Spend;
            auto utxo =
                db_.ReserveUTXO(builder.Spender(), id, Spend::ConfirmedOnly);

            if (false == utxo.has_value()) {
                LogOutput(OT_METHOD)(__func__)(": Insufficient funds").Flush();
                output = BuildResult::PermanentFailure;
                rc = SendResult::InsufficientFunds;

                return output;
            }

            if (false == builder.AddInput(utxo.value())) {
                LogOutput(OT_METHOD)(__func__)(": Failed to add input").Flush();
                output = BuildResult::PermanentFailure;
                rc = SendResult::InputCreationError;

                return output;
            }
        }

        OT_ASSERT(builder.IsFunded());

        builder.FinalizeOutputs();

        if (false == builder.SignInputs()) {
            LogOutput(OT_METHOD)(__func__)(": Transaction signing failure")
                .Flush();
            output = BuildResult::PermanentFailure;
            rc = SendResult::SignatureError;

            return output;
        }

        auto pTransaction = builder.FinalizeTransaction();

        if (false == bool(pTransaction)) {
            LogOutput(OT_METHOD)(__func__)(
                ": Failed to instantiate transaction")
                .Flush();
            output = BuildResult::PermanentFailure;

            return output;
        }

        const auto& transaction = *pTransaction;

        {
            const auto proto = transaction.Serialize(crypto_);

            if (false == proto.has_value()) {
                LogOutput(OT_METHOD)(__func__)(
                    ": Failed to serialize transaction")
                    .Flush();
                output = BuildResult::PermanentFailure;

                return output;
            }

            *proposal.mutable_finished() = proto.value();
        }

        if (!db_.AddProposal(id, proposal)) {
            LogOutput(OT_METHOD)(__func__)(": Database error (proposal)")
                .Flush();
            output = BuildResult::PermanentFailure;
            rc = SendResult::DatabaseError;

            return output;
        }

        if (!db_.AddOutgoingTransaction(id, proposal, transaction)) {
            LogOutput(OT_METHOD)(__func__)(": Database error (transaction)")
                .Flush();
            output = BuildResult::PermanentFailure;
            rc = SendResult::DatabaseError;

            return output;
        }

        txid = transaction.ID();
        const auto sent = node_.BroadcastTransaction(transaction);

        try {
            if (sent) {
                auto bytes = api_.Factory().Data();
                transaction.Serialize(bytes->WriteInto());
                LogOutput("Broadcasting ")(DisplayString(chain_))(
                    " transaction ")(txid->asHex())
                    .Flush();
                LogOutput(bytes->asHex()).Flush();
            } else {
                throw std::runtime_error{"Failed to send tx"};
            }

            rc = SendResult::Sent;

            for (const auto& notif : proposal.notification()) {
                using PC = crypto::internal::PaymentCode;
                const auto accountID = PC::GetID(
                    api_,
                    chain_,
                    api_.Factory().PaymentCode(notif.sender()),
                    api_.Factory().PaymentCode(notif.recipient()));
                const auto nymID = [&] {
                    auto out = api_.Factory().NymID();
                    const auto& bytes = proposal.initiator();
                    out->Assign(bytes.data(), bytes.size());

                    OT_ASSERT(false == out->empty());

                    return out;
                }();
                const auto& account =
                    crypto_.PaymentCodeSubaccount(nymID, accountID);
                account.AddNotification(transaction.ID());
            }
        } catch (const std::exception& e) {
            LogOutput(OT_METHOD)(__func__)(": ")(e.what()).Flush();
        }

        return output;
    }
    auto cleanup(const Lock& lock) noexcept -> void
    {
        const auto finished = db_.CompletedProposals();

        for (const auto& id : finished) {
            pending_.Delete(id);
            confirming_.erase(id);
        }

        db_.ForgetProposals(finished);
    }
    auto get_builder() const noexcept -> Builder
    {
        switch (chain_) {
            case Type::Bitcoin:
            case Type::Bitcoin_testnet3:
            case Type::BitcoinCash:
            case Type::BitcoinCash_testnet3:
            case Type::Litecoin:
            case Type::Litecoin_testnet4:
            case Type::PKT:
            case Type::PKT_testnet:
            case Type::UnitTest: {

                return [this](const auto& id, auto& in, auto& out) -> auto
                {
                    return build_transaction_bitcoin(id, in, out);
                };
            }
            case Type::Unknown:
            case Type::Ethereum_frontier:
            case Type::Ethereum_ropsten:
            default: {
                LogOutput(OT_METHOD)(__func__)(": Unsupported chain").Flush();

                return {};
            }
        }
    }
    auto rebroadcast(const Lock& lock) noexcept -> void
    {
        // TODO monitor inv messages from peers and wait until a peer
        // we didn't transmit the transaction to tells us about it
        constexpr auto limit = std::chrono::minutes(1);

        for (auto& [id, time] : confirming_) {
            if ((Clock::now() - time) < limit) { continue; }

            auto proposal = db_.LoadProposal(id);

            if (false == proposal.has_value()) { continue; }

            auto pTx = factory::BitcoinTransaction(
                api_, crypto_, proposal.value().finished());

            if (false == bool(pTx)) { continue; }

            const auto& tx = *pTx;
            node_.BroadcastTransaction(tx);
        }
    }
    auto send(const Lock& lock) noexcept -> void
    {
        if (false == pending_.HasData()) { return; }

        auto job = pending_.Pop();
        auto& [id, promise] = job;
        auto wipe{false};
        auto erase{false};
        auto loop = ScopeGuard{[&]() {
            const auto& [id, promise] = job;

            if (wipe) {
                db_.CancelProposal(id);
                erase = true;
            }

            if (false == erase) { pending_.Add(std::move(job)); }
        }};
        auto serialized = db_.LoadProposal(id);

        if (false == serialized.has_value()) { return; }

        auto& data = *serialized;

        if (is_expired(data)) {
            wipe = true;

            return;
        }

        if (auto builder = get_builder(); builder) {
            switch (builder(id, data, promise)) {
                case BuildResult::PermanentFailure: {
                    wipe = true;
                    [[fallthrough]];
                }
                case BuildResult::TemporaryFailure: {

                    return;
                }
                case BuildResult::Success:
                default: {
                    confirming_.emplace(std::move(id), Clock::now());
                    erase = true;
                }
            }
        }
    }
};

Proposals::Proposals(
    const api::Core& api,
    const api::client::Blockchain& crypto,
    const node::internal::Network& node,
    const node::internal::WalletDatabase& db,
    const Type chain) noexcept
    : imp_(std::make_unique<Imp>(api, crypto, node, db, chain))
{
    OT_ASSERT(imp_);
}

auto Proposals::Add(const Proposal& tx, std::promise<SendOutcome>&& promise)
    const noexcept -> void
{
    imp_->Add(tx, std::move(promise));
}

auto Proposals::Run() noexcept -> bool { return imp_->Run(); }

Proposals::~Proposals() = default;
}  // namespace opentxs::blockchain::node::wallet
