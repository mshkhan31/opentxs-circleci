// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                          // IWYU pragma: associated
#include "1_Internal.hpp"                        // IWYU pragma: associated
#include "api/client/blockchain/Blockchain.hpp"  // IWYU pragma: associated

#include <set>
#include <type_traits>
#include <utility>

#include "Proto.hpp"
#include "Proto.tpp"
#include "api/client/blockchain/Imp.hpp"
#include "internal/api/client/Factory.hpp"
#include "internal/blockchain/Params.hpp"
#include "opentxs/api/Context.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/block/bitcoin/Transaction.hpp"  // IWYU pragma: keep
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/crypto/Bip32Child.hpp"
#include "opentxs/crypto/Bip43Purpose.hpp"
#include "opentxs/crypto/Bip44Type.hpp"
#include "opentxs/protobuf/HDPath.pb.h"

// #define OT_METHOD "opentxs::api::client::implementation::Blockchain::"

namespace zmq = opentxs::network::zeromq;

using ReturnType = opentxs::api::client::implementation::Blockchain;

namespace opentxs::factory
{
auto BlockchainAPI(
    const api::Core& api,
    const api::client::Activity& activity,
    const api::client::Contacts& contacts,
    const api::Legacy& legacy,
    const std::string& dataFolder,
    const Options& args) noexcept
    -> std::shared_ptr<api::client::internal::Blockchain>
{
    return std::make_shared<ReturnType>(
        api, activity, contacts, legacy, dataFolder, args);
}
}  // namespace opentxs::factory

namespace opentxs::api::client
{
auto Blockchain::Bip44(Chain chain) noexcept(false) -> Bip44Type
{
    return opentxs::blockchain::params::Data::Chains().at(chain).bip44_;
}

auto Blockchain::Bip44Path(
    Chain chain,
    const std::string& seed,
    AllocateOutput destination) noexcept(false) -> bool
{
    constexpr auto hard = static_cast<Bip32Index>(Bip32Child::HARDENED);
    const auto coin = Bip44(chain);
    auto output = proto::HDPath{};
    output.set_version(1);
    output.set_root(seed);
    output.add_child(static_cast<Bip32Index>(Bip43Purpose::HDWALLET) | hard);
    output.add_child(static_cast<Bip32Index>(coin) | hard);
    output.add_child(Bip32Index{0} | hard);
    return write(output, destination);
}
}  // namespace opentxs::api::client

namespace opentxs::api::client::implementation
{
auto Blockchain::Account(const identifier::Nym& nymID, const Chain chain) const
    noexcept(false) -> const opentxs::blockchain::crypto::Account&
{
    return imp_->Account(nymID, chain);
}

auto Blockchain::SubaccountList(const identifier::Nym& nymID, const Chain chain)
    const noexcept -> std::set<OTIdentifier>
{
    return imp_->SubaccountList(nymID, chain);
}

auto Blockchain::AccountList(const identifier::Nym& nymID) const noexcept
    -> std::set<OTIdentifier>
{
    return imp_->AccountList(nymID);
}

auto Blockchain::AccountList(const Chain chain) const noexcept
    -> std::set<OTIdentifier>
{
    return imp_->AccountList(chain);
}

auto Blockchain::AccountList() const noexcept -> std::set<OTIdentifier>
{
    return imp_->AccountList();
}

auto Blockchain::ActivityDescription(
    const identifier::Nym& nym,
    const Identifier& thread,
    const std::string& itemID) const noexcept -> std::string
{
    return imp_->ActivityDescription(nym, thread, itemID);
}

auto Blockchain::ActivityDescription(
    const identifier::Nym& nym,
    const Chain chain,
    const Tx& transaction) const noexcept -> std::string
{
    return imp_->ActivityDescription(nym, chain, transaction);
}

auto Blockchain::AssignContact(
    const identifier::Nym& nymID,
    const Identifier& accountID,
    const Subchain subchain,
    const Bip32Index index,
    const Identifier& contactID) const noexcept -> bool
{
    return imp_->AssignContact(nymID, accountID, subchain, index, contactID);
}

auto Blockchain::AssignLabel(
    const identifier::Nym& nymID,
    const Identifier& accountID,
    const Subchain subchain,
    const Bip32Index index,
    const std::string& label) const noexcept -> bool
{
    return imp_->AssignLabel(nymID, accountID, subchain, index, label);
}

auto Blockchain::AssignTransactionMemo(
    const TxidHex& id,
    const std::string& label) const noexcept -> bool
{
    return imp_->AssignTransactionMemo(id, label);
}

auto Blockchain::CalculateAddress(
    const Chain chain,
    const Style format,
    const Data& pubkey) const noexcept -> std::string
{
    return imp_->CalculateAddress(chain, format, pubkey);
}

auto Blockchain::Confirm(
    const Key key,
    const opentxs::blockchain::block::Txid& tx) const noexcept -> bool
{
    return imp_->Confirm(key, tx);
}

auto Blockchain::Contacts() const noexcept -> const api::client::Contacts&
{
    return imp_->Contacts();
}

auto Blockchain::DecodeAddress(const std::string& encoded) const noexcept
    -> DecodedAddress
{
    return imp_->DecodeAddress(encoded);
}

auto Blockchain::EncodeAddress(
    const Style style,
    const Chain chain,
    const Data& data) const noexcept -> std::string
{
    return imp_->EncodeAddress(style, chain, data);
}

auto Blockchain::GetKey(const Key& id) const noexcept(false)
    -> const opentxs::blockchain::crypto::Element&
{
    return imp_->GetKey(id);
}

auto Blockchain::HDSubaccount(
    const identifier::Nym& nymID,
    const Identifier& accountID) const noexcept(false)
    -> const opentxs::blockchain::crypto::HD&
{
    return imp_->HDSubaccount(nymID, accountID);
}

auto Blockchain::IndexItem(const ReadView bytes) const noexcept -> PatternID
{
    return imp_->IndexItem(bytes);
}

auto Blockchain::Init() noexcept -> void { imp_->Init(); }

auto Blockchain::KeyEndpoint() const noexcept -> const std::string&
{
    return imp_->KeyEndpoint();
}

auto Blockchain::KeyGenerated(const Chain chain) const noexcept -> void
{
    imp_->KeyGenerated(chain);
}

auto Blockchain::LoadTransactionBitcoin(const TxidHex& txid) const noexcept
    -> std::unique_ptr<const Tx>
{
    return imp_->LoadTransactionBitcoin(txid);
}

auto Blockchain::LoadTransactionBitcoin(const Txid& txid) const noexcept
    -> std::unique_ptr<const Tx>
{
    return imp_->LoadTransactionBitcoin(txid);
}

auto Blockchain::LookupAccount(const Identifier& id) const noexcept
    -> AccountData
{
    return imp_->LookupAccount(id);
}

auto Blockchain::LookupContacts(const std::string& address) const noexcept
    -> ContactList
{
    const auto [pubkeyHash, style, chain, supported] =
        imp_->DecodeAddress(address);

    return LookupContacts(pubkeyHash);
}

auto Blockchain::LookupContacts(const Data& pubkeyHash) const noexcept
    -> ContactList
{
    return imp_->LookupContacts(pubkeyHash);
}

auto Blockchain::NewHDSubaccount(
    const identifier::Nym& nymID,
    const opentxs::blockchain::crypto::HDProtocol standard,
    const Chain chain,
    const PasswordPrompt& reason) const noexcept -> OTIdentifier
{
    return imp_->NewHDSubaccount(nymID, standard, chain, chain, reason);
}

auto Blockchain::NewHDSubaccount(
    const identifier::Nym& nymID,
    const opentxs::blockchain::crypto::HDProtocol standard,
    const Chain derivationChain,
    const Chain targetChain,
    const PasswordPrompt& reason) const noexcept -> OTIdentifier
{
    return imp_->NewHDSubaccount(
        nymID, standard, derivationChain, targetChain, reason);
}

auto Blockchain::NewNym(const identifier::Nym& id) const noexcept -> void
{
    return imp_->NewNym(id);
}

auto Blockchain::NewPaymentCodeSubaccount(
    const identifier::Nym& nymID,
    const opentxs::PaymentCode& local,
    const opentxs::PaymentCode& remote,
    const proto::HDPath& path,
    const Chain chain,
    const PasswordPrompt& reason) const noexcept -> OTIdentifier
{
    return imp_->NewPaymentCodeSubaccount(
        nymID, local, remote, path, chain, reason);
}

auto Blockchain::NewPaymentCodeSubaccount(
    const identifier::Nym& nymID,
    const opentxs::PaymentCode& local,
    const opentxs::PaymentCode& remote,
    const ReadView& view,
    const Chain chain,
    const PasswordPrompt& reason) const noexcept -> OTIdentifier
{
    auto path = proto::Factory<proto::HDPath>(view);
    return imp_->NewPaymentCodeSubaccount(
        nymID, local, remote, path, chain, reason);
}

auto Blockchain::Owner(const Identifier& accountID) const noexcept
    -> const identifier::Nym&
{
    return imp_->Owner(accountID);
}

auto Blockchain::Owner(const Key& key) const noexcept -> const identifier::Nym&
{
    return imp_->Owner(key);
}

auto Blockchain::PaymentCodeSubaccount(
    const identifier::Nym& nymID,
    const Identifier& accountID) const noexcept(false)
    -> const opentxs::blockchain::crypto::PaymentCode&
{
    return imp_->PaymentCodeSubaccount(nymID, accountID);
}

auto Blockchain::PaymentCodeSubaccount(
    const identifier::Nym& nymID,
    const opentxs::PaymentCode& local,
    const opentxs::PaymentCode& remote,
    const proto::HDPath& path,
    const Chain chain,
    const PasswordPrompt& reason) const noexcept(false)
    -> const opentxs::blockchain::crypto::PaymentCode&
{
    return imp_->PaymentCodeSubaccount(
        nymID, local, remote, path, chain, reason);
}

auto Blockchain::ProcessContact(const Contact& contact) const noexcept -> bool
{
    return imp_->ProcessContact(contact);
}

auto Blockchain::ProcessMergedContact(
    const Contact& parent,
    const Contact& child) const noexcept -> bool
{
    return imp_->ProcessMergedContact(parent, child);
}

auto Blockchain::ProcessTransaction(
    const Chain chain,
    const Tx& in,
    const PasswordPrompt& reason) const noexcept -> bool
{
    return imp_->ProcessTransaction(chain, in, reason);
}

auto Blockchain::PubkeyHash(
    [[maybe_unused]] const Chain chain,
    const Data& pubkey) const noexcept(false) -> OTData
{
    return imp_->PubkeyHash(chain, pubkey);
}

auto Blockchain::RecipientContact(const Key& key) const noexcept -> OTIdentifier
{
    return imp_->RecipientContact(key);
}

auto Blockchain::Release(const Key key) const noexcept -> bool
{
    return imp_->Release(key);
}

auto Blockchain::ReportScan(
    const Chain chain,
    const identifier::Nym& owner,
    const opentxs::blockchain::crypto::SubaccountType type,
    const Identifier& account,
    const Subchain subchain,
    const opentxs::blockchain::block::Position& progress) const noexcept -> void
{
    imp_->ReportScan(chain, owner, type, account, subchain, progress);
}

auto Blockchain::SenderContact(const Key& key) const noexcept -> OTIdentifier
{
    return imp_->SenderContact(key);
}

auto Blockchain::Unconfirm(
    const Key key,
    const opentxs::blockchain::block::Txid& tx,
    const Time time) const noexcept -> bool
{
    return imp_->Unconfirm(key, tx, time);
}

auto Blockchain::UpdateElement(std::vector<ReadView>& hashes) const noexcept
    -> void
{
    imp_->UpdateElement(hashes);
}

auto Blockchain::UpdateBalance(
    const opentxs::blockchain::Type chain,
    const opentxs::blockchain::Balance balance) const noexcept -> void
{
    imp_->UpdateBalance(chain, balance);
}

auto Blockchain::UpdateBalance(
    const identifier::Nym& owner,
    const opentxs::blockchain::Type chain,
    const opentxs::blockchain::Balance balance) const noexcept -> void
{
    imp_->UpdateBalance(owner, chain, balance);
}

auto Blockchain::Wallet(const Chain chain) const noexcept(false)
    -> const opentxs::blockchain::crypto::Wallet&
{
    return imp_->Wallet(chain);
}

Blockchain::~Blockchain() = default;
}  // namespace opentxs::api::client::implementation
