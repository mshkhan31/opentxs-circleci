// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/blockchain/crypto/SubaccountType.hpp"

#pragma once

#include <atomic>
#include <cstddef>
#include <functional>
#include <iosfwd>
#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <string>
#include <thread>
#include <vector>

#include "api/client/blockchain/BalanceOracle.hpp"
#include "api/client/blockchain/Blockchain.hpp"
#include "api/client/blockchain/Imp.hpp"
#include "blockchain/database/common/Database.hpp"
#include "internal/api/client/Client.hpp"
#include "internal/blockchain/database/common/Common.hpp"
#include "internal/blockchain/node/Node.hpp"
#include "opentxs/Bytes.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/Context.hpp"
#include "opentxs/api/client/Blockchain.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/crypto/Subchain.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/blockchain/node/Manager.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/network/zeromq/ListenCallback.hpp"
#include "opentxs/network/zeromq/Message.hpp"
#include "opentxs/network/zeromq/socket/Publish.hpp"
#include "opentxs/network/zeromq/socket/Router.hpp"

namespace opentxs
{
namespace api
{
namespace client
{
namespace blockchain
{
struct SyncClient;
struct SyncServer;
}  // namespace blockchain

namespace internal
{
struct Blockchain;
}  // namespace internal

class Activity;
class Contacts;
}  // namespace client

class Core;
class Legacy;
}  // namespace api

namespace blockchain
{
namespace block
{
namespace bitcoin
{
namespace internal
{
struct Transaction;
}  // namespace internal
}  // namespace bitcoin
}  // namespace block
}  // namespace blockchain

namespace identifier
{
class Nym;
}  // namespace identifier

namespace network
{
namespace blockchain
{
namespace sync
{
class Data;
}  // namespace sync
}  // namespace blockchain

namespace zeromq
{
class Context;
class Message;
}  // namespace zeromq
}  // namespace network

class Contact;
class Data;
class Identifier;
class Options;
class PasswordPrompt;
}  // namespace opentxs

namespace zmq = opentxs::network::zeromq;

namespace opentxs::api::client::implementation
{
struct BlockchainImp final : public Blockchain::Imp {
    using Txid = opentxs::blockchain::block::Txid;
    using pTxid = opentxs::blockchain::block::pTxid;
    using Tx = Blockchain::Tx;
    using TxidHex = Blockchain::TxidHex;
    using PatternID = Blockchain::PatternID;
    using ContactList = Blockchain::ContactList;

    auto ActivityDescription(
        const identifier::Nym& nym,
        const Identifier& thread,
        const std::string& threadItemID) const noexcept -> std::string final;
    auto ActivityDescription(
        const identifier::Nym& nym,
        const opentxs::blockchain::Type chain,
        const Tx& transaction) const noexcept -> std::string final;
    auto AssignTransactionMemo(const TxidHex& id, const std::string& label)
        const noexcept -> bool final;
    auto IndexItem(const ReadView bytes) const noexcept -> PatternID final;
    auto KeyEndpoint() const noexcept -> const std::string& final;
    auto KeyGenerated(const opentxs::blockchain::Type chain) const noexcept
        -> void final;
    auto LoadTransactionBitcoin(const TxidHex& txid) const noexcept
        -> std::unique_ptr<const Tx> final;
    auto LoadTransactionBitcoin(const Txid& txid) const noexcept
        -> std::unique_ptr<const Tx> final;
    auto LookupContacts(const Data& pubkeyHash) const noexcept
        -> ContactList final;
    auto ProcessContact(const Contact& contact) const noexcept -> bool final;
    auto ProcessMergedContact(const Contact& parent, const Contact& child)
        const noexcept -> bool final;
    auto ProcessTransaction(
        const opentxs::blockchain::Type chain,
        const Tx& in,
        const PasswordPrompt& reason) const noexcept -> bool final;
    auto ReportScan(
        const opentxs::blockchain::Type chain,
        const identifier::Nym& owner,
        const opentxs::blockchain::crypto::SubaccountType type,
        const Identifier& account,
        const Blockchain::Subchain subchain,
        const opentxs::blockchain::block::Position& progress) const noexcept
        -> void final;
    auto UpdateBalance(
        const opentxs::blockchain::Type chain,
        const opentxs::blockchain::Balance balance) const noexcept
        -> void final;
    auto UpdateBalance(
        const identifier::Nym& owner,
        const opentxs::blockchain::Type chain,
        const opentxs::blockchain::Balance balance) const noexcept
        -> void final;
    auto UpdateElement(std::vector<ReadView>& pubkeyHashes) const noexcept
        -> void final;

    BlockchainImp(
        const api::Core& api,
        const api::client::Activity& activity,
        const api::client::Contacts& contacts,
        const api::Legacy& legacy,
        const std::string& dataFolder,
        const Options& args,
        api::client::internal::Blockchain& parent) noexcept;

    ~BlockchainImp() final = default;

private:
    api::client::internal::Blockchain& parent_;
    const api::client::Activity& activity_;
    const std::string key_generated_endpoint_;
    OTZMQPublishSocket transaction_updates_;
    OTZMQPublishSocket key_updates_;
    OTZMQPublishSocket scan_updates_;
    OTZMQPublishSocket new_blockchain_accounts_;
    blockchain::BalanceOracle balances_;

    auto broadcast_update_signal(const Txid& txid) const noexcept -> void
    {
        broadcast_update_signal(std::vector<pTxid>{txid});
    }
    auto broadcast_update_signal(
        const std::vector<pTxid>& transactions) const noexcept -> void;
    auto broadcast_update_signal(
        const opentxs::blockchain::block::bitcoin::internal::Transaction& tx)
        const noexcept -> void;
    auto load_transaction(const Lock& lock, const Txid& id) const noexcept
        -> std::unique_ptr<
            opentxs::blockchain::block::bitcoin::internal::Transaction>;
    auto load_transaction(const Lock& lock, const TxidHex& id) const noexcept
        -> std::unique_ptr<
            opentxs::blockchain::block::bitcoin::internal::Transaction>;
    auto notify_new_account(
        const Identifier& id,
        const identifier::Nym& owner,
        opentxs::blockchain::Type chain,
        opentxs::blockchain::crypto::SubaccountType type) const noexcept
        -> void final;
    auto reconcile_activity_threads(const Lock& lock, const Txid& txid)
        const noexcept -> bool;
    auto reconcile_activity_threads(
        const Lock& lock,
        const opentxs::blockchain::block::bitcoin::internal::Transaction& tx)
        const noexcept -> bool;
};
}  // namespace opentxs::api::client::implementation
