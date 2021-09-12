// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                   // IWYU pragma: associated
#include "1_Internal.hpp"                 // IWYU pragma: associated
#include "api/client/blockchain/Imp.hpp"  // IWYU pragma: associated

#include <bech32.h>
#include <boost/container/flat_map.hpp>
#include <segwit_addr.h>
#include <algorithm>
#include <cstddef>
#include <iterator>
#include <stdexcept>
#include <tuple>
#include <type_traits>
#include <utility>

#include "internal/api/client/Client.hpp"
#include "internal/blockchain/Params.hpp"
#include "internal/blockchain/crypto/Crypto.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/api/Core.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/api/Wallet.hpp"
#include "opentxs/api/client/Contacts.hpp"
#include "opentxs/api/crypto/Crypto.hpp"
#include "opentxs/api/crypto/Encode.hpp"
#include "opentxs/api/crypto/Hash.hpp"
#include "opentxs/api/storage/Storage.hpp"
#include "opentxs/blockchain/block/bitcoin/Transaction.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/crypto/Account.hpp"
#include "opentxs/blockchain/crypto/AddressStyle.hpp"
#include "opentxs/blockchain/crypto/Element.hpp"
#include "opentxs/blockchain/crypto/HD.hpp"
#include "opentxs/blockchain/crypto/HDProtocol.hpp"
#include "opentxs/blockchain/crypto/PaymentCode.hpp"
#include "opentxs/blockchain/crypto/Subaccount.hpp"
#include "opentxs/blockchain/crypto/SubaccountType.hpp"
#include "opentxs/blockchain/crypto/Subchain.hpp"
#include "opentxs/blockchain/crypto/Wallet.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"
#include "opentxs/core/crypto/PaymentCode.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/crypto/Bip32.hpp"
#include "opentxs/crypto/Bip32Child.hpp"
#include "opentxs/crypto/Bip43Purpose.hpp"
#include "opentxs/crypto/Bip44Type.hpp"
#include "opentxs/crypto/HashType.hpp"
#include "opentxs/identity/Nym.hpp"
#include "opentxs/protobuf/HDPath.pb.h"
#include "util/Container.hpp"
#include "util/HDIndex.hpp"

#define PATH_VERSION 1
#define COMPRESSED_PUBKEY_SIZE 33

#define OT_METHOD "opentxs::api::client::implementation::Blockchain::Imp::"

namespace opentxs::api::client
{
enum class Prefix : std::uint8_t {
    Unknown = 0,
    BitcoinP2PKH,
    BitcoinP2SH,
    BitcoinTestnetP2PKH,
    BitcoinTestnetP2SH,
    LitecoinP2PKH,
    LitecoinP2SH,
    LitecoinTestnetP2SH,
    PKTP2PKH,
    PKTP2SH,
};

using Style = Blockchain::Style;
using AddressMap = std::map<Prefix, std::string>;
using AddressReverseMap = std::map<std::string, Prefix>;
using StylePair = std::pair<Style, opentxs::blockchain::Type>;
// Style, preferred prefix, additional prefixes
using StyleMap = std::map<StylePair, std::pair<Prefix, std::set<Prefix>>>;
using StyleReverseMap = std::map<Prefix, std::set<StylePair>>;
using HrpMap = std::map<opentxs::blockchain::Type, std::string>;
using HrpReverseMap = std::map<std::string, opentxs::blockchain::Type>;

auto reverse(const StyleMap& in) noexcept -> StyleReverseMap;
auto reverse(const StyleMap& in) noexcept -> StyleReverseMap
{
    auto output = StyleReverseMap{};
    std::for_each(std::begin(in), std::end(in), [&](const auto& data) {
        const auto& [metadata, prefixData] = data;
        const auto& [preferred, additional] = prefixData;
        output[preferred].emplace(metadata);

        for (const auto& prefix : additional) {
            output[prefix].emplace(metadata);
        }
    });

    return output;
}

const AddressReverseMap address_prefix_reverse_map_{
    {"00", Prefix::BitcoinP2PKH},
    {"05", Prefix::BitcoinP2SH},
    {"30", Prefix::LitecoinP2PKH},
    {"32", Prefix::LitecoinP2SH},
    {"3a", Prefix::LitecoinTestnetP2SH},
    {"38", Prefix::PKTP2SH},
    {"6f", Prefix::BitcoinTestnetP2PKH},
    {"c4", Prefix::BitcoinTestnetP2SH},
    {"75", Prefix::PKTP2PKH},
};
const AddressMap address_prefix_map_{reverse_map(address_prefix_reverse_map_)};
const StyleMap address_style_map_{
    {{Style::P2PKH, opentxs::blockchain::Type::UnitTest},
     {Prefix::BitcoinTestnetP2PKH, {}}},
    {{Style::P2PKH, opentxs::blockchain::Type::BitcoinCash_testnet3},
     {Prefix::BitcoinTestnetP2PKH, {}}},
    {{Style::P2PKH, opentxs::blockchain::Type::BitcoinCash},
     {Prefix::BitcoinP2PKH, {}}},
    {{Style::P2PKH, opentxs::blockchain::Type::Bitcoin_testnet3},
     {Prefix::BitcoinTestnetP2PKH, {}}},
    {{Style::P2PKH, opentxs::blockchain::Type::Bitcoin},
     {Prefix::BitcoinP2PKH, {}}},
    {{Style::P2PKH, opentxs::blockchain::Type::Litecoin_testnet4},
     {Prefix::BitcoinTestnetP2PKH, {}}},
    {{Style::P2PKH, opentxs::blockchain::Type::Litecoin},
     {Prefix::LitecoinP2PKH, {}}},
    {{Style::P2PKH, opentxs::blockchain::Type::PKT_testnet},
     {Prefix::BitcoinTestnetP2PKH, {}}},
    {{Style::P2PKH, opentxs::blockchain::Type::PKT}, {Prefix::PKTP2PKH, {}}},
    {{Style::P2SH, opentxs::blockchain::Type::UnitTest},
     {Prefix::BitcoinTestnetP2SH, {}}},
    {{Style::P2SH, opentxs::blockchain::Type::BitcoinCash_testnet3},
     {Prefix::BitcoinTestnetP2SH, {}}},
    {{Style::P2SH, opentxs::blockchain::Type::BitcoinCash},
     {Prefix::BitcoinP2SH, {}}},
    {{Style::P2SH, opentxs::blockchain::Type::Bitcoin_testnet3},
     {Prefix::BitcoinTestnetP2SH, {}}},
    {{Style::P2SH, opentxs::blockchain::Type::Bitcoin},
     {Prefix::BitcoinP2SH, {}}},
    {{Style::P2SH, opentxs::blockchain::Type::Litecoin_testnet4},
     {Prefix::LitecoinTestnetP2SH, {Prefix::BitcoinTestnetP2SH}}},
    {{Style::P2SH, opentxs::blockchain::Type::Litecoin},
     {Prefix::LitecoinP2SH, {Prefix::BitcoinP2SH}}},
    {{Style::P2SH, opentxs::blockchain::Type::PKT_testnet},
     {Prefix::BitcoinTestnetP2SH, {}}},
    {{Style::P2SH, opentxs::blockchain::Type::PKT}, {Prefix::PKTP2SH, {}}},
};
const StyleReverseMap address_style_reverse_map_{reverse(address_style_map_)};
const HrpMap hrp_map_{
    {opentxs::blockchain::Type::Bitcoin, "bc"},
    {opentxs::blockchain::Type::Bitcoin_testnet3, "tb"},
    {opentxs::blockchain::Type::Litecoin, "ltc"},
    {opentxs::blockchain::Type::Litecoin_testnet4, "tltc"},
    {opentxs::blockchain::Type::PKT, "pkt"},
    {opentxs::blockchain::Type::PKT_testnet, "tpk"},
    {opentxs::blockchain::Type::UnitTest, "bcrt"},
};
const HrpReverseMap hrp_reverse_map_{reverse_map(hrp_map_)};
}  // namespace opentxs::api::client

namespace opentxs::api::client::implementation
{
Blockchain::Imp::Imp(
    const api::Core& api,
    const api::client::Contacts& contacts,
    api::client::internal::Blockchain& parent) noexcept
    : api_(api)
    , contacts_(contacts)
    , blank_(api_.Factory().Data(), Style::Unknown, {}, false)
    , lock_()
    , nym_lock_()
    , accounts_(api_)
    , wallets_(api_, parent)
{
}

auto Blockchain::Imp::Account(
    const identifier::Nym& nymID,
    const opentxs::blockchain::Type chain) const noexcept(false)
    -> const opentxs::blockchain::crypto::Account&
{
    if (false == validate_nym(nymID)) {
        throw std::runtime_error("Invalid nym");
    }

    return Wallet(chain).Account(nymID);
}

auto Blockchain::Imp::AccountList(const identifier::Nym& nym) const noexcept
    -> std::set<OTIdentifier>
{
    return wallets_.AccountList(nym);
}

auto Blockchain::Imp::AccountList(const opentxs::blockchain::Type chain)
    const noexcept -> std::set<OTIdentifier>
{
    return wallets_.AccountList(chain);
}

auto Blockchain::Imp::AccountList() const noexcept -> std::set<OTIdentifier>
{
    return wallets_.AccountList();
}

auto Blockchain::Imp::ActivityDescription(
    const identifier::Nym&,
    const Identifier&,
    const std::string&) const noexcept -> std::string
{
    return {};
}

auto Blockchain::Imp::ActivityDescription(
    const identifier::Nym&,
    const opentxs::blockchain::Type,
    const Tx&) const noexcept -> std::string
{
    return {};
}

auto Blockchain::Imp::address_prefix(
    const Style style,
    const opentxs::blockchain::Type chain) const noexcept(false) -> OTData
{
    return api_.Factory().Data(
        address_prefix_map_.at(address_style_map_.at({style, chain}).first),
        StringStyle::Hex);
}

auto Blockchain::Imp::AssignContact(
    const identifier::Nym& nymID,
    const Identifier& accountID,
    const Subchain subchain,
    const Bip32Index index,
    const Identifier& contactID) const noexcept -> bool
{
    if (false == validate_nym(nymID)) { return false; }

    auto lock = Lock{nym_mutex(nymID)};

    const auto chain = Translate(
        api_.Storage().BlockchainAccountType(nymID.str(), accountID.str()));

    OT_ASSERT(opentxs::blockchain::Type::Unknown != chain);

    try {
        auto& node = wallets_.Get(chain).Account(nymID).Subaccount(accountID);

        try {
            const auto& element = node.BalanceElement(subchain, index);
            const auto existing = element.Contact();

            if (contactID == existing) { return true; }

            return node.Internal().SetContact(subchain, index, contactID);
        } catch (...) {
            LogOutput(OT_METHOD)(__func__)(": Failed to load balance element")
                .Flush();

            return false;
        }
    } catch (...) {
        LogOutput(OT_METHOD)(__func__)(": Failed to load account").Flush();

        return false;
    }
}

auto Blockchain::Imp::AssignLabel(
    const identifier::Nym& nymID,
    const Identifier& accountID,
    const Subchain subchain,
    const Bip32Index index,
    const std::string& label) const noexcept -> bool
{
    if (false == validate_nym(nymID)) { return false; }

    auto lock = Lock{nym_mutex(nymID)};

    const auto chain = Translate(
        api_.Storage().BlockchainAccountType(nymID.str(), accountID.str()));

    OT_ASSERT(opentxs::blockchain::Type::Unknown != chain);

    try {
        auto& node = wallets_.Get(chain).Account(nymID).Subaccount(accountID);

        try {
            const auto& element = node.BalanceElement(subchain, index);

            if (label == element.Label()) { return true; }

            return node.Internal().SetLabel(subchain, index, label);
        } catch (...) {
            LogOutput(OT_METHOD)(__func__)(": Failed to load balance element")
                .Flush();

            return false;
        }
    } catch (...) {
        LogOutput(OT_METHOD)(__func__)(": Failed to load account").Flush();

        return false;
    }
}

auto Blockchain::Imp::bip44_type(
    const contact::ContactItemType type) const noexcept -> Bip44Type
{
    switch (type) {
        case contact::ContactItemType::BTC: {

            return Bip44Type::BITCOIN;
        }
        case contact::ContactItemType::LTC: {

            return Bip44Type::LITECOIN;
        }
        case contact::ContactItemType::DOGE: {

            return Bip44Type::DOGECOIN;
        }
        case contact::ContactItemType::DASH: {

            return Bip44Type::DASH;
        }
        case contact::ContactItemType::BCH: {

            return Bip44Type::BITCOINCASH;
        }
        case contact::ContactItemType::PKT: {

            return Bip44Type::PKT;
        }
        case contact::ContactItemType::TNBCH:
        case contact::ContactItemType::TNBTC:
        case contact::ContactItemType::TNXRP:
        case contact::ContactItemType::TNLTX:
        case contact::ContactItemType::TNXEM:
        case contact::ContactItemType::TNDASH:
        case contact::ContactItemType::TNMAID:
        case contact::ContactItemType::TNLSK:
        case contact::ContactItemType::TNDOGE:
        case contact::ContactItemType::TNXMR:
        case contact::ContactItemType::TNWAVES:
        case contact::ContactItemType::TNNXT:
        case contact::ContactItemType::TNSC:
        case contact::ContactItemType::TNSTEEM:
        case contact::ContactItemType::TNPKT:
        case contact::ContactItemType::Regtest: {
            return Bip44Type::TESTNET;
        }
        default: {
            OT_FAIL;
        }
    }
}

auto Blockchain::Imp::CalculateAddress(
    const opentxs::blockchain::Type chain,
    const Style format,
    const Data& pubkey) const noexcept -> std::string
{
    auto data = api_.Factory().Data();

    switch (format) {
        case Style::P2WPKH:
        case Style::P2PKH: {
            try {
                data = PubkeyHash(chain, pubkey);
            } catch (...) {
                LogOutput(OT_METHOD)(__func__)(": Invalid public key.").Flush();

                return {};
            }
        } break;
        default: {
            LogOutput(OT_METHOD)(__func__)(": Unsupported address style (")(
                static_cast<std::uint16_t>(format))(")")
                .Flush();

            return {};
        }
    }

    return EncodeAddress(format, chain, data);
}

auto Blockchain::Imp::Confirm(
    const Key key,
    const opentxs::blockchain::block::Txid& tx) const noexcept -> bool
{
    try {
        const auto [id, subchain, index] = key;
        const auto accountID = api_.Factory().Identifier(id);

        return get_node(accountID).Internal().Confirm(subchain, index, tx);
    } catch (...) {

        return false;
    }
}

auto Blockchain::Imp::DecodeAddress(const std::string& encoded) const noexcept
    -> DecodedAddress
{
    static constexpr auto check =
        [](DecodedAddress& output) -> DecodedAddress& {
        auto& [data, style, chains, supported] = output;
        supported = false;

        if (0 == data->size()) { return output; }
        if (Style::Unknown == style) { return output; }
        if (0 == chains.size()) { return output; }

        const auto& params = opentxs::blockchain::params::Data::Chains();

        for (const auto& chain : chains) {
            try {
                if (false == params.at(chain).scripts_.at(style)) {

                    return output;
                }
            } catch (...) {

                return output;
            }
        }

        supported = true;

        return output;
    };
    auto output = decode_bech23(encoded);

    if (output.has_value()) { return check(output.value()); }

    output = decode_legacy(encoded);

    if (output.has_value()) { return check(output.value()); }

    return blank_;
}

auto Blockchain::Imp::decode_bech23(const std::string& encoded) const noexcept
    -> std::optional<DecodedAddress>
{
    auto output{blank_};
    auto& [data, style, chains, supported] = output;

    try {
        const auto result = bech32::decode(encoded);
        using Encoding = bech32::Encoding;

        switch (result.encoding) {
            case Encoding::BECH32:
            case Encoding::BECH32M: {
            } break;
            case Encoding::INVALID:
            default: {
                throw std::runtime_error("not bech32");
            }
        }

        const auto [version, bytes] = segwit_addr::decode(result.hrp, encoded);

        try {
            switch (version) {
                case 0: {
                    switch (bytes.size()) {
                        case 20: {
                            style = Style::P2WPKH;
                        } break;
                        case 32: {
                            style = Style::P2WSH;
                        } break;
                        default: {
                            throw std::runtime_error{
                                "unknown version 0 program"};
                        }
                    }
                } break;
                case 1: {
                    switch (bytes.size()) {
                        case 32: {
                            style = Style::P2TR;
                        } break;
                        default: {
                            throw std::runtime_error{
                                "unknown version 1 program"};
                        }
                    }
                } break;
                case -1:
                default: {
                    throw std::runtime_error{"Unsupported version"};
                }
            }

            copy(reader(bytes), data->WriteInto());
            chains.emplace(hrp_reverse_map_.at(result.hrp));

            return std::move(output);
        } catch (const std::exception& e) {
            LogTrace(OT_METHOD)(__func__)(": ")(e.what()).Flush();

            return blank_;
        }
    } catch (const std::exception& e) {
        LogTrace(OT_METHOD)(__func__)(": ")(e.what()).Flush();

        return std::nullopt;
    }
}

auto Blockchain::Imp::decode_legacy(const std::string& encoded) const noexcept
    -> std::optional<DecodedAddress>
{
    auto output{blank_};
    auto& [data, style, chains, supported] = output;

    try {
        const auto bytes = api_.Factory().Data(
            api_.Crypto().Encode().IdentifierDecode(encoded), StringStyle::Raw);
        auto type = api_.Factory().Data();

        if (0 == bytes->size()) { throw std::runtime_error("not base58"); }

        try {
            switch (bytes->size()) {
                case 21: {
                    bytes->Extract(1, type, 0);
                    auto prefix{Prefix::Unknown};

                    try {
                        prefix = address_prefix_reverse_map_.at(type->asHex());
                    } catch (...) {
                        throw std::runtime_error(
                            "unable to decode version byte");
                    }

                    const auto& map = address_style_reverse_map_.at(prefix);

                    for (const auto& [decodeStyle, decodeChain] : map) {
                        style = decodeStyle;
                        chains.emplace(decodeChain);
                    }

                    bytes->Extract(20, data, 1);
                } break;
                default: {
                    throw std::runtime_error("unknown address format");
                }
            }

            return std::move(output);
        } catch (const std::exception& e) {
            LogTrace(OT_METHOD)(__func__)(": ")(e.what()).Flush();

            return blank_;
        }
    } catch (const std::exception& e) {
        LogTrace(OT_METHOD)(__func__)(": ")(e.what()).Flush();

        return std::nullopt;
    }
}

auto Blockchain::Imp::EncodeAddress(
    const Style style,
    const opentxs::blockchain::Type chain,
    const Data& data) const noexcept -> std::string
{
    switch (style) {
        case Style::P2WPKH: {

            return p2wpkh(chain, data);
        }
        case Style::P2PKH: {

            return p2pkh(chain, data);
        }
        case Style::P2SH: {

            return p2sh(chain, data);
        }
        default: {
            LogOutput(OT_METHOD)(__func__)(": Unsupported address style (")(
                static_cast<std::uint16_t>(style))(")")
                .Flush();

            return {};
        }
    }
}

auto Blockchain::Imp::GetKey(const Key& id) const noexcept(false)
    -> const opentxs::blockchain::crypto::Element&
{
    const auto [str, subchain, index] = id;
    const auto account = api_.Factory().Identifier(str);
    using Type = opentxs::blockchain::crypto::SubaccountType;

    switch (accounts_.Type(account)) {
        case Type::HD: {
            const auto& hd = HDSubaccount(accounts_.Owner(account), account);

            return hd.BalanceElement(subchain, index);
        }
        case Type::PaymentCode: {
            const auto& pc =
                PaymentCodeSubaccount(accounts_.Owner(account), account);

            return pc.BalanceElement(subchain, index);
        }
        case Type::Imported:
        case Type::Error:
        default: {
        }
    }

    throw std::out_of_range("key not found");
}

auto Blockchain::Imp::get_node(const Identifier& accountID) const
    noexcept(false) -> opentxs::blockchain::crypto::Subaccount&
{
    const auto& nymID = accounts_.Owner(accountID);
    const auto nym = nymID.str();
    const auto id = accountID.str();
    const auto& wallet = [&]() -> auto&
    {
        const auto type = api_.Storage().BlockchainAccountType(nym, id);

        if (contact::ContactItemType::Error == type) {
            const auto error = std::string{"account "} + id + " for nym " +
                               nym + " does not exist";

            throw std::out_of_range(error);
        }

        return wallets_.Get(Translate(type));
    }
    ();
    const auto& account = wallet.Account(nymID);
    const auto& subaccount =
        [&]() -> const opentxs::blockchain::crypto::Subaccount& {
        switch (accounts_.Type(accountID)) {
            case opentxs::blockchain::crypto::SubaccountType::HD: {

                return account.GetHD().at(accountID);
            }
            case opentxs::blockchain::crypto::SubaccountType::PaymentCode: {

                return account.GetPaymentCode().at(accountID);
            }
            case opentxs::blockchain::crypto::SubaccountType::Imported:
            case opentxs::blockchain::crypto::SubaccountType::Error:
            default: {
                throw std::out_of_range("subaccount type not supported");
            }
        }
    }();

    return subaccount.Internal();
}

auto Blockchain::Imp::HDSubaccount(
    const identifier::Nym& nymID,
    const Identifier& accountID) const noexcept(false)
    -> const opentxs::blockchain::crypto::HD&
{
    const auto id = accountID.str();
    const auto nym = nymID.str();
    const auto type = api_.Storage().BlockchainAccountType(nym, id);

    if (contact::ContactItemType::Error == type) {
        const auto error =
            std::string{"HD account "} + id + " for " + nym + " does not exist";

        throw std::out_of_range(error);
    }

    auto& wallet = wallets_.Get(Translate(type));
    auto& account = wallet.Account(nymID);

    return account.GetHD().at(accountID);
}

auto Blockchain::Imp::IndexItem(const ReadView bytes) const noexcept
    -> PatternID
{
    return {};
}

auto Blockchain::Imp::Init() noexcept -> void
{
    accounts_.Populate();

    for (const auto& chain : opentxs::blockchain::SupportedChains()) {
        Wallet(chain);
    }
}

auto Blockchain::Imp::init_path(
    const std::string& root,
    const contact::ContactItemType chain,
    const Bip32Index account,
    const opentxs::blockchain::crypto::HDProtocol standard,
    proto::HDPath& path) const noexcept -> void
{
    using Standard = opentxs::blockchain::crypto::HDProtocol;
    path.set_version(PATH_VERSION);
    path.set_root(root);

    switch (standard) {
        case Standard::BIP_32: {
            path.add_child(HDIndex{account, Bip32Child::HARDENED});
        } break;
        case Standard::BIP_44: {
            path.add_child(
                HDIndex{Bip43Purpose::HDWALLET, Bip32Child::HARDENED});
            path.add_child(HDIndex{bip44_type(chain), Bip32Child::HARDENED});
            path.add_child(account);
        } break;
        case Standard::BIP_49: {
            path.add_child(
                HDIndex{Bip43Purpose::P2SH_P2WPKH, Bip32Child::HARDENED});
            path.add_child(HDIndex{bip44_type(chain), Bip32Child::HARDENED});
            path.add_child(account);
        } break;
        case Standard::BIP_84: {
            path.add_child(HDIndex{Bip43Purpose::P2WPKH, Bip32Child::HARDENED});
            path.add_child(HDIndex{bip44_type(chain), Bip32Child::HARDENED});
            path.add_child(account);
        } break;
        default: {
            OT_FAIL;
        }
    }
}

auto Blockchain::Imp::KeyEndpoint() const noexcept -> const std::string&
{
    static const auto blank = std::string{};

    return blank;
}

auto Blockchain::Imp::KeyGenerated(
    const opentxs::blockchain::Type) const noexcept -> void
{
}

auto Blockchain::Imp::LoadTransactionBitcoin(const TxidHex&) const noexcept
    -> std::unique_ptr<const Tx>
{
    return {};
}

auto Blockchain::Imp::LoadTransactionBitcoin(const Txid&) const noexcept
    -> std::unique_ptr<const Tx>
{
    return {};
}

auto Blockchain::Imp::LookupAccount(const Identifier& id) const noexcept
    -> AccountData
{
    return wallets_.LookupAccount(id);
}

auto Blockchain::Imp::LookupContacts(const Data&) const noexcept -> ContactList
{
    return {};
}

auto Blockchain::Imp::NewHDSubaccount(
    const identifier::Nym& nymID,
    const opentxs::blockchain::crypto::HDProtocol standard,
    const opentxs::blockchain::Type derivationChain,
    const opentxs::blockchain::Type targetChain,
    const PasswordPrompt& reason) const noexcept -> OTIdentifier
{
    static const auto blank = api_.Factory().Identifier();

    if (false == validate_nym(nymID)) { return blank; }

    if (opentxs::blockchain::Type::Unknown == derivationChain) {
        LogOutput(OT_METHOD)(__func__)(": Invalid derivationChain").Flush();

        return blank;
    }

    if (opentxs::blockchain::Type::Unknown == targetChain) {
        LogOutput(OT_METHOD)(__func__)(": Invalid targetChain").Flush();

        return blank;
    }

    auto nym = api_.Wallet().Nym(nymID);

    if (false == bool(nym)) {
        LogOutput(OT_METHOD)(__func__)(": Nym does not exist.").Flush();

        return blank;
    }

    auto nymPath = proto::HDPath{};

    if (false == nym->Path(nymPath)) {
        LogOutput(OT_METHOD)(__func__)(": No nym path.").Flush();

        return blank;
    }

    if (0 == nymPath.root().size()) {
        LogOutput(OT_METHOD)(__func__)(": Missing root.").Flush();

        return blank;
    }

    if (2 > nymPath.child().size()) {
        LogOutput(OT_METHOD)(__func__)(": Invalid path.").Flush();

        return blank;
    }

    auto accountPath = proto::HDPath{};
    init_path(
        nymPath.root(),
        Translate(derivationChain),
        HDIndex{nymPath.child(1), Bip32Child::HARDENED},
        standard,
        accountPath);

    try {
        auto accountID{blank};
        auto& tree = wallets_.Get(targetChain).Account(nymID);
        tree.Internal().AddHDNode(accountPath, standard, reason, accountID);

        OT_ASSERT(false == accountID->empty());

        LogVerbose(OT_METHOD)(__func__)(": Created new HD subaccount ")(
            accountID)(" for ")(DisplayString(targetChain))(" account ")(
            tree.AccountID())(" owned by ")(nymID.str())(" using path ")(
            opentxs::crypto::Print(accountPath))
            .Flush();
        accounts_.New(
            opentxs::blockchain::crypto::SubaccountType::HD,
            targetChain,
            accountID,
            nymID);
        notify_new_account(
            accountID,
            nymID,
            targetChain,
            opentxs::blockchain::crypto::SubaccountType::HD);

        return accountID;
    } catch (...) {
        LogVerbose(OT_METHOD)(__func__)(": Failed to create account").Flush();

        return blank;
    }
}

auto Blockchain::Imp::NewNym(const identifier::Nym& id) const noexcept -> void
{
    for (const auto& chain : opentxs::blockchain::SupportedChains()) {
        Wallet(chain).Account(id);
    }
}

auto Blockchain::Imp::NewPaymentCodeSubaccount(
    const identifier::Nym& nymID,
    const opentxs::PaymentCode& local,
    const opentxs::PaymentCode& remote,
    const proto::HDPath path,
    const opentxs::blockchain::Type chain,
    const PasswordPrompt& reason) const noexcept -> OTIdentifier
{
    auto lock = Lock{nym_mutex(nymID)};

    return new_payment_code(lock, nymID, local, remote, path, chain, reason);
}

auto Blockchain::Imp::new_payment_code(
    const Lock&,
    const identifier::Nym& nymID,
    const opentxs::PaymentCode& local,
    const opentxs::PaymentCode& remote,
    const proto::HDPath path,
    const opentxs::blockchain::Type chain,
    const PasswordPrompt& reason) const noexcept -> OTIdentifier
{
    static const auto blank = api_.Factory().Identifier();

    if (false == validate_nym(nymID)) { return blank; }

    if (opentxs::blockchain::Type::Unknown == chain) {
        LogOutput(OT_METHOD)(__func__)(": Invalid chain").Flush();

        return blank;
    }

    auto nym = api_.Wallet().Nym(nymID);

    if (false == bool(nym)) {
        LogOutput(OT_METHOD)(__func__)(": Nym does not exist.").Flush();

        return blank;
    }

    if (0 == path.root().size()) {
        LogOutput(OT_METHOD)(__func__)(": Missing root.").Flush();

        return blank;
    }

    if (3 > path.child().size()) {
        LogOutput(OT_METHOD)(__func__)(": Invalid path: ")(
            opentxs::crypto::Print(path))
            .Flush();

        return blank;
    }

    try {
        auto accountID{blank};
        auto& tree = wallets_.Get(chain).Account(nymID);
        tree.Internal().AddUpdatePaymentCode(
            local, remote, path, reason, accountID);

        OT_ASSERT(false == accountID->empty());

        LogVerbose(OT_METHOD)(__func__)(
            ": Created new payment code subaccount ")(accountID)(" for  ")(
            DisplayString(chain))(" account ")(tree.AccountID())(" owned by ")(
            nymID.str())("in reference to remote payment code ")(
            remote.asBase58())
            .Flush();
        accounts_.New(
            opentxs::blockchain::crypto::SubaccountType::PaymentCode,
            chain,
            accountID,
            nymID);
        notify_new_account(
            accountID,
            nymID,
            chain,
            opentxs::blockchain::crypto::SubaccountType::PaymentCode);

        return accountID;
    } catch (...) {
        LogVerbose(OT_METHOD)(__func__)(": Failed to create account").Flush();

        return blank;
    }
}

auto Blockchain::Imp::nym_mutex(const identifier::Nym& nym) const noexcept
    -> std::mutex&
{
    auto lock = Lock{lock_};

    return nym_lock_[nym];
}

auto Blockchain::Imp::Owner(const Key& key) const noexcept
    -> const identifier::Nym&
{
    const auto& [account, subchain, index] = key;
    static const auto blank = api_.Factory().NymID();

    if (Subchain::Outgoing == subchain) { return blank; }

    return Owner(api_.Factory().Identifier(account));
}

auto Blockchain::Imp::p2pkh(
    const opentxs::blockchain::Type chain,
    const Data& pubkeyHash) const noexcept -> std::string
{
    try {
        auto preimage = address_prefix(Style::P2PKH, chain);

        OT_ASSERT(1 == preimage->size());

        preimage += pubkeyHash;

        OT_ASSERT(21 == preimage->size());

        return api_.Crypto().Encode().IdentifierEncode(preimage);
    } catch (...) {
        LogOutput(OT_METHOD)(__func__)(": Unsupported chain (")(
            opentxs::print(chain))(")")
            .Flush();

        return "";
    }
}

auto Blockchain::Imp::p2sh(
    const opentxs::blockchain::Type chain,
    const Data& pubkeyHash) const noexcept -> std::string
{
    try {
        auto preimage = address_prefix(Style::P2SH, chain);

        OT_ASSERT(1 == preimage->size());

        preimage += pubkeyHash;

        OT_ASSERT(21 == preimage->size());

        return api_.Crypto().Encode().IdentifierEncode(preimage);
    } catch (...) {
        LogOutput(OT_METHOD)(__func__)(": Unsupported chain (")(
            opentxs::print(chain))(")")
            .Flush();

        return "";
    }
}

auto Blockchain::Imp::p2wpkh(
    const opentxs::blockchain::Type chain,
    const Data& hash) const noexcept -> std::string
{
    try {
        const auto& hrp = hrp_map_.at(chain);
        const auto prog = [&] {
            auto out = std::vector<std::uint8_t>{};
            std::transform(
                hash.begin(),
                hash.end(),
                std::back_inserter(out),
                [](const auto& byte) {
                    return std::to_integer<std::uint8_t>(byte);
                });

            return out;
        }();

        return segwit_addr::encode(hrp, 0, prog);
    } catch (...) {
        LogOutput(OT_METHOD)(__func__)(": Unsupported chain (")(
            opentxs::print(chain))(")")
            .Flush();

        return "";
    }
}

auto Blockchain::Imp::PaymentCodeSubaccount(
    const identifier::Nym& nymID,
    const Identifier& accountID) const noexcept(false)
    -> const opentxs::blockchain::crypto::PaymentCode&
{
    const auto type = api_.Storage().Bip47Chain(nymID, accountID);

    if (contact::ContactItemType::Error == type) {
        const auto error = std::string{"Payment code account "} +
                           accountID.str() + " for " + nymID.str() +
                           " does not exist";

        throw std::out_of_range(error);
    }

    auto& wallet = wallets_.Get(Translate(type));
    auto& account = wallet.Account(nymID);

    return account.GetPaymentCode().at(accountID);
}

auto Blockchain::Imp::PaymentCodeSubaccount(
    const identifier::Nym& nymID,
    const opentxs::PaymentCode& local,
    const opentxs::PaymentCode& remote,
    const proto::HDPath path,
    const opentxs::blockchain::Type chain,
    const PasswordPrompt& reason) const noexcept(false)
    -> const opentxs::blockchain::crypto::PaymentCode&
{
    auto lock = Lock{nym_mutex(nymID)};
    const auto accountID =
        opentxs::blockchain::crypto::internal::PaymentCode::GetID(
            api_, chain, local, remote);
    const auto type = api_.Storage().Bip47Chain(nymID, accountID);

    if (contact::ContactItemType::Error == type) {
        const auto id =
            new_payment_code(lock, nymID, local, remote, path, chain, reason);

        if (accountID != id) {
            throw std::out_of_range("Failed to create account");
        }
    }

    auto& balanceList = wallets_.Get(chain);
    auto& tree = balanceList.Account(nymID);

    return tree.GetPaymentCode().at(accountID);
}

auto Blockchain::Imp::ProcessContact(const Contact&) const noexcept -> bool
{
    return false;
}

auto Blockchain::Imp::ProcessMergedContact(const Contact&, const Contact&)
    const noexcept -> bool
{
    return false;
}

auto Blockchain::Imp::ProcessTransaction(
    const opentxs::blockchain::Type,
    const Tx&,
    const PasswordPrompt&) const noexcept -> bool
{
    return false;
}

auto Blockchain::Imp::PubkeyHash(
    [[maybe_unused]] const opentxs::blockchain::Type chain,
    const Data& pubkey) const noexcept(false) -> OTData
{
    if (pubkey.empty()) { throw std::runtime_error("Empty pubkey"); }

    if (COMPRESSED_PUBKEY_SIZE != pubkey.size()) {
        throw std::runtime_error("Incorrect pubkey size");
    }

    auto output = Data::Factory();

    if (false == api_.Crypto().Hash().Digest(
                     opentxs::crypto::HashType::Bitcoin,
                     pubkey.Bytes(),
                     output->WriteInto())) {
        throw std::runtime_error("Unable to calculate hash.");
    }

    return output;
}

auto Blockchain::Imp::RecipientContact(const Key& key) const noexcept
    -> OTIdentifier
{
    static const auto blank = api_.Factory().Identifier();
    const auto& [account, subchain, index] = key;
    using Subchain = opentxs::blockchain::crypto::Subchain;

    if (Subchain::Notification == subchain) { return blank; }

    const auto accountID = api_.Factory().Identifier(account);
    const auto& owner = Owner(accountID);

    try {
        if (owner.empty()) {
            throw std::runtime_error{"Failed to load account owner"};
        }

        const auto& element = GetKey(key);

        switch (subchain) {
            case Subchain::Internal:
            case Subchain::External:
            case Subchain::Incoming: {

                return contacts_.NymToContact(owner);
            }
            case Subchain::Outgoing: {

                return element.Contact();
            }
            default: {

                return blank;
            }
        }
    } catch (const std::exception& e) {
        LogOutput(OT_METHOD)(__func__)(": ")(e.what()).Flush();

        return blank;
    }
}

auto Blockchain::Imp::Release(const Key key) const noexcept -> bool
{
    try {
        const auto [id, subchain, index] = key;
        const auto accountID = api_.Factory().Identifier(id);

        return get_node(accountID).Internal().Unreserve(subchain, index);
    } catch (...) {

        return false;
    }
}

auto Blockchain::Imp::ReportScan(
    const opentxs::blockchain::Type,
    const identifier::Nym&,
    const opentxs::blockchain::crypto::SubaccountType,
    const Identifier&,
    const Subchain,
    const opentxs::blockchain::block::Position&) const noexcept -> void
{
}

auto Blockchain::Imp::SenderContact(const Key& key) const noexcept
    -> OTIdentifier
{
    static const auto blank = api_.Factory().Identifier();
    const auto& [account, subchain, index] = key;
    using Subchain = opentxs::blockchain::crypto::Subchain;

    if (Subchain::Notification == subchain) { return blank; }

    const auto accountID = api_.Factory().Identifier(account);
    const auto& owner = Owner(accountID);

    try {
        if (owner.empty()) {
            throw std::runtime_error{"Failed to load account owner"};
        }

        const auto& element = GetKey(key);

        switch (subchain) {
            case Subchain::Internal:
            case Subchain::Outgoing: {

                return contacts_.NymToContact(owner);
            }
            case Subchain::External:
            case Subchain::Incoming: {

                return element.Contact();
            }
            default: {

                return blank;
            }
        }
    } catch (const std::exception& e) {
        LogOutput(OT_METHOD)(__func__)(": ")(e.what()).Flush();

        return blank;
    }
}

auto Blockchain::Imp::Unconfirm(
    const Key key,
    const opentxs::blockchain::block::Txid& tx,
    const Time time) const noexcept -> bool
{
    try {
        const auto [id, subchain, index] = key;
        const auto accountID = api_.Factory().Identifier(id);

        return get_node(accountID).Internal().Unconfirm(
            subchain, index, tx, time);
    } catch (...) {

        return false;
    }
}

auto Blockchain::Imp::UpdateBalance(
    const opentxs::blockchain::Type,
    const opentxs::blockchain::Balance) const noexcept -> void
{
}

auto Blockchain::Imp::UpdateBalance(
    const identifier::Nym&,
    const opentxs::blockchain::Type,
    const opentxs::blockchain::Balance) const noexcept -> void
{
}

auto Blockchain::Imp::UpdateElement(std::vector<ReadView>&) const noexcept
    -> void
{
}

auto Blockchain::Imp::validate_nym(const identifier::Nym& nymID) const noexcept
    -> bool
{
    if (false == nymID.empty()) {
        if (0 < api_.Wallet().LocalNyms().count(nymID)) { return true; }
    }

    return false;
}

auto Blockchain::Imp::Wallet(const opentxs::blockchain::Type chain) const
    noexcept(false) -> const opentxs::blockchain::crypto::Wallet&
{
    if (0 == opentxs::blockchain::DefinedChains().count(chain)) {
        throw std::runtime_error("Invalid chain");
    }

    return wallets_.Get(chain);
}
}  // namespace opentxs::api::client::implementation
