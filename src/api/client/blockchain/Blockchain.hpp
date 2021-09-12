// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "api/client/blockchain/database/Database.hpp"
// IWYU pragma: no_include "internal/blockchain/node/Node.hpp"
// IWYU pragma: no_include "opentxs/api/client/Contacts.hpp"
// IWYU pragma: no_include "opentxs/blockchain/BlockchainType.hpp"
// IWYU pragma: no_include "opentxs/blockchain/crypto/Account.hpp"
// IWYU pragma: no_include "opentxs/blockchain/crypto/AddressStyle.hpp"
// IWYU pragma: no_include "opentxs/blockchain/crypto/HD.hpp"
// IWYU pragma: no_include "opentxs/blockchain/crypto/HDProtocol.hpp"
// IWYU pragma: no_include "opentxs/blockchain/crypto/PaymentCode.hpp"
// IWYU pragma: no_include "opentxs/blockchain/crypto/SubaccountType.hpp"
// IWYU pragma: no_include "opentxs/blockchain/crypto/Subchain.hpp"
// IWYU pragma: no_include "opentxs/blockchain/node/Manager.hpp"
// IWYU pragma: no_include "opentxs/core/identifier/Nym.hpp"
// IWYU pragma: no_include "opentxs/network/zeromq/socket/Publish.hpp"

#pragma once

#include <atomic>
#include <cstdint>
#include <functional>
#include <future>
#include <iosfwd>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <set>
#include <string>
#include <thread>
#include <tuple>
#include <utility>
#include <vector>

#include "internal/api/client/Client.hpp"
#include "internal/blockchain/crypto/Crypto.hpp"
#include "opentxs/Bytes.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/client/Blockchain.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/crypto/Element.hpp"
#include "opentxs/blockchain/crypto/Subaccount.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/blockchain/crypto/Wallet.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/network/zeromq/Message.hpp"

namespace opentxs
{
namespace api
{
namespace client
{
class Activity;
class Contacts;
}  // namespace client

class Core;
class Legacy;
}  // namespace api

namespace blockchain
{
namespace node
{
class Manager;
}  // namespace node
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
namespace socket
{
class Publish;
}  // namespace socket
}  // namespace zeromq
}  // namespace network

namespace proto
{
class HDPath;
}  // namespace proto

class Contact;
class Options;
class PasswordPrompt;
class PaymentCode;
}  // namespace opentxs

namespace opentxs::api::client::implementation
{
class Blockchain final : virtual public internal::Blockchain
{
public:
    struct Imp;

    auto Account(const identifier::Nym& nymID, const Chain chain) const
        noexcept(false) -> const opentxs::blockchain::crypto::Account& final;
    auto AccountList(const identifier::Nym& nymID) const noexcept
        -> std::set<OTIdentifier> final;
    auto AccountList(const Chain chain) const noexcept
        -> std::set<OTIdentifier> final;
    auto AccountList() const noexcept -> std::set<OTIdentifier> final;
    auto ActivityDescription(
        const identifier::Nym& nym,
        const Identifier& thread,
        const std::string& threadItemID) const noexcept -> std::string final;
    auto ActivityDescription(
        const identifier::Nym& nym,
        const Chain chain,
        const Tx& transaction) const noexcept -> std::string final;
    auto AssignContact(
        const identifier::Nym& nymID,
        const Identifier& accountID,
        const Subchain subchain,
        const Bip32Index index,
        const Identifier& contactID) const noexcept -> bool final;
    auto AssignLabel(
        const identifier::Nym& nymID,
        const Identifier& accountID,
        const Subchain subchain,
        const Bip32Index index,
        const std::string& label) const noexcept -> bool final;
    auto AssignTransactionMemo(const std::string& id, const std::string& label)
        const noexcept -> bool final;
    auto CalculateAddress(
        const Chain chain,
        const opentxs::blockchain::crypto::AddressStyle format,
        const Data& pubkey) const noexcept -> std::string final;
    auto Confirm(const Key key, const opentxs::blockchain::block::Txid& tx)
        const noexcept -> bool final;
    auto Contacts() const noexcept -> const api::client::Contacts& final;
    auto DecodeAddress(const std::string& encoded) const noexcept
        -> DecodedAddress final;
    auto EncodeAddress(const Style style, const Chain chain, const Data& data)
        const noexcept -> std::string final;
    auto GetKey(const Key& id) const noexcept(false)
        -> const opentxs::blockchain::crypto::Element& final;
    auto HDSubaccount(const identifier::Nym& nymID, const Identifier& accountID)
        const noexcept(false) -> const opentxs::blockchain::crypto::HD& final;
    auto IndexItem(const ReadView bytes) const noexcept -> PatternID final;
    auto Internal() const noexcept -> const internal::Blockchain& final
    {
        return *this;
    }
    auto KeyEndpoint() const noexcept -> const std::string& final;
    auto KeyGenerated(const Chain chain) const noexcept -> void final;
    auto LoadTransactionBitcoin(const TxidHex& id) const noexcept
        -> std::unique_ptr<const Tx> final;
    auto LoadTransactionBitcoin(const Txid& id) const noexcept
        -> std::unique_ptr<const Tx> final;
    auto LookupAccount(const Identifier& id) const noexcept
        -> AccountData final;
    auto LookupContacts(const std::string& address) const noexcept
        -> ContactList final;
    auto LookupContacts(const Data& pubkeyHash) const noexcept
        -> ContactList final;
    auto NewHDSubaccount(
        const identifier::Nym& nymID,
        const opentxs::blockchain::crypto::HDProtocol standard,
        const Chain chain,
        const PasswordPrompt& reason) const noexcept -> OTIdentifier final;
    auto NewHDSubaccount(
        const identifier::Nym& nymID,
        const opentxs::blockchain::crypto::HDProtocol standard,
        const Chain derivationChain,
        const Chain targetChain,
        const PasswordPrompt& reason) const noexcept -> OTIdentifier final;
    auto NewNym(const identifier::Nym& id) const noexcept -> void final;
    auto NewPaymentCodeSubaccount(
        const identifier::Nym& nymID,
        const opentxs::PaymentCode& local,
        const opentxs::PaymentCode& remote,
        const proto::HDPath& path,
        const Chain chain,
        const PasswordPrompt& reason) const noexcept -> OTIdentifier final;
    auto NewPaymentCodeSubaccount(
        const identifier::Nym& nymID,
        const opentxs::PaymentCode& local,
        const opentxs::PaymentCode& remote,
        const ReadView& view,
        const Chain chain,
        const PasswordPrompt& reason) const noexcept -> OTIdentifier final;
    auto Owner(const Identifier& accountID) const noexcept
        -> const identifier::Nym& final;
    auto Owner(const Key& key) const noexcept -> const identifier::Nym& final;
    auto PaymentCodeSubaccount(
        const identifier::Nym& nymID,
        const Identifier& accountID) const noexcept(false)
        -> const opentxs::blockchain::crypto::PaymentCode& final;
    auto PaymentCodeSubaccount(
        const identifier::Nym& nymID,
        const opentxs::PaymentCode& local,
        const opentxs::PaymentCode& remote,
        const proto::HDPath& path,
        const Chain chain,
        const PasswordPrompt& reason) const noexcept(false)
        -> const opentxs::blockchain::crypto::PaymentCode& final;
    auto PubkeyHash(const Chain chain, const Data& pubkey) const noexcept(false)
        -> OTData final;
    auto ProcessContact(const Contact& contact) const noexcept -> bool final;
    auto ProcessMergedContact(const Contact& parent, const Contact& child)
        const noexcept -> bool final;
    auto ProcessTransaction(
        const Chain chain,
        const Tx& transaction,
        const PasswordPrompt& reason) const noexcept -> bool final;
    auto RecipientContact(const Key& key) const noexcept -> OTIdentifier final;
    auto Release(const Key key) const noexcept -> bool final;
    auto ReportScan(
        const Chain chain,
        const identifier::Nym& owner,
        const opentxs::blockchain::crypto::SubaccountType type,
        const Identifier& account,
        const Subchain subchain,
        const opentxs::blockchain::block::Position& progress) const noexcept
        -> void final;
    auto SenderContact(const Key& key) const noexcept -> OTIdentifier final;
    auto SubaccountList(const identifier::Nym& nymID, const Chain chain)
        const noexcept -> std::set<OTIdentifier> final;
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
    auto Unconfirm(
        const Key key,
        const opentxs::blockchain::block::Txid& tx,
        const Time time) const noexcept -> bool final;
    auto Wallet(const Chain chain) const noexcept(false)
        -> const opentxs::blockchain::crypto::Wallet& final;

    auto Init() noexcept -> void final;

    Blockchain(
        const api::Core& api,
        const api::client::Activity& activity,
        const api::client::Contacts& contacts,
        const api::Legacy& legacy,
        const std::string& dataFolder,
        const Options& args) noexcept;

    ~Blockchain() final;

private:
    std::unique_ptr<Imp> imp_;

    Blockchain() = delete;
    Blockchain(const Blockchain&) = delete;
    Blockchain(Blockchain&&) = delete;
    auto operator=(const Blockchain&) -> Blockchain& = delete;
    auto operator=(Blockchain&&) -> Blockchain& = delete;
};
}  // namespace opentxs::api::client::implementation
