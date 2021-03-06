// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                         // IWYU pragma: associated
#include "1_Internal.hpp"                       // IWYU pragma: associated
#include "blockchain/block/bitcoin/Output.hpp"  // IWYU pragma: associated

#include <boost/container/vector.hpp>
#include <algorithm>
#include <cstddef>
#include <cstring>
#include <iosfwd>
#include <iterator>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>

#include "internal/api/client/Client.hpp"
#include "internal/blockchain/bitcoin/Bitcoin.hpp"
#include "internal/blockchain/block/Block.hpp"
#include "internal/contact/Contact.hpp"
#include "opentxs/Bytes.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/api/Core.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/api/client/Blockchain.hpp"
#include "opentxs/blockchain/block/bitcoin/Output.hpp"
#include "opentxs/blockchain/block/bitcoin/Script.hpp"
#include "opentxs/blockchain/crypto/Element.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/crypto/Subchain.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/network/blockchain/bitcoin/CompactSize.hpp"
#include "opentxs/protobuf/BlockchainTransactionOutput.pb.h"
#include "opentxs/protobuf/BlockchainWalletKey.pb.h"

#define OT_METHOD                                                              \
    "opentxs::blockchain::block::bitcoin::implementation::Output::"

namespace opentxs::factory
{
using ReturnType = blockchain::block::bitcoin::implementation::Output;
using Position = opentxs::blockchain::block::bitcoin::Script::Position;

auto BitcoinTransactionOutput(
    const api::Core& api,
    const api::client::Blockchain& blockchain,
    const blockchain::Type chain,
    const std::uint32_t index,
    const std::uint64_t value,
    std::unique_ptr<const blockchain::block::bitcoin::internal::Script> script,
    const std::set<blockchain::crypto::Key>& keys) noexcept
    -> std::unique_ptr<blockchain::block::bitcoin::internal::Output>
{
    try {
        auto keySet = boost::container::flat_set<ReturnType::KeyID>{};
        std::for_each(std::begin(keys), std::end(keys), [&](const auto& key) {
            keySet.emplace(key);
        });

        return std::make_unique<ReturnType>(
            api,
            blockchain,
            chain,
            index,
            value,
            std::move(script),
            std::move(keySet));
    } catch (const std::exception& e) {
        LogOutput("opentxs::factory::")(__func__)(": ")(e.what()).Flush();

        return {};
    }
}

auto BitcoinTransactionOutput(
    const api::Core& api,
    const api::client::Blockchain& blockchain,
    const blockchain::Type chain,
    const std::uint32_t index,
    const std::uint64_t value,
    const blockchain::bitcoin::CompactSize& cs,
    const ReadView script) noexcept
    -> std::unique_ptr<blockchain::block::bitcoin::internal::Output>
{
    try {
        return std::make_unique<ReturnType>(
            api,
            blockchain,
            chain,
            index,
            value,
            sizeof(value) + cs.Total(),
            script);
    } catch (const std::exception& e) {
        LogOutput("opentxs::factory::")(__func__)(": ")(e.what()).Flush();

        return {};
    }
}

auto BitcoinTransactionOutput(
    const api::Core& api,
    const api::client::Blockchain& blockchain,
    const blockchain::Type chain,
    const proto::BlockchainTransactionOutput& in) noexcept
    -> std::unique_ptr<blockchain::block::bitcoin::internal::Output>
{
    try {
        auto value = static_cast<std::int64_t>(in.value());
        auto cs = blockchain::bitcoin::CompactSize(in.script().size());
        auto keys = boost::container::flat_set<ReturnType::KeyID>{};
        auto pkh = boost::container::flat_set<blockchain::PatternID>{};
        using Payer = OTIdentifier;
        using Payee = OTIdentifier;
        using Correction = std::pair<Payer, Payee>;
        auto corrections = std::vector<Correction>{};

        for (const auto& key : in.key()) {
            const auto subchain = static_cast<blockchain::crypto::Subchain>(
                static_cast<std::uint8_t>(key.subchain()));
            auto keyid =
                ReturnType::KeyID{key.subaccount(), subchain, key.index()};

            if (blockchain::crypto::Subchain::Outgoing == subchain) {
                LogOutput("opentxs::factory::")(__func__)(
                    ": invalid key detected in transaction")
                    .Flush();
                auto sender = blockchain.SenderContact(keyid);
                auto recipient = blockchain.RecipientContact(keyid);

                if (sender->empty() || recipient->empty()) { OT_FAIL; }

                corrections.emplace_back(
                    std::move(sender), std::move(recipient));
            } else {
                keys.emplace(std::move(keyid));
            }
        }

        for (const auto& pattern : in.pubkey_hash()) { pkh.emplace(pattern); }

        auto out = std::make_unique<ReturnType>(
            api,
            blockchain,
            chain,
            in.version(),
            in.index(),
            value,
            factory::BitcoinScript(chain, in.script(), Position::Output),
            sizeof(value) + cs.Total(),
            std::move(keys),
            std::move(pkh),
            (in.has_script_hash()
                 ? std::make_optional<blockchain::PatternID>(in.script_hash())
                 : std::nullopt),
            in.indexed());

        for (const auto& payer : in.payer()) {
            if (false == payer.empty()) {
                out->SetPayer([&] {
                    auto id = api.Factory().Identifier();
                    id->Assign(payer.data(), payer.size());

                    return id;
                }());
            }
        }

        for (const auto& payee : in.payee()) {
            if (false == payee.empty()) {
                out->SetPayee([&] {
                    auto id = api.Factory().Identifier();
                    id->Assign(payee.data(), payee.size());

                    return id;
                }());
            }
        }

        for (const auto& [payer, payee] : corrections) {
            out->SetPayer(payer);
            out->SetPayee(payee);
        }

        return std::move(out);
    } catch (const std::exception& e) {
        LogOutput("opentxs::factory::")(__func__)(": ")(e.what()).Flush();

        return {};
    }
}
}  // namespace opentxs::factory

namespace opentxs::blockchain::block::bitcoin::implementation
{
const VersionNumber Output::default_version_{1};
const VersionNumber Output::key_version_{1};

Output::Output(
    const api::Core& api,
    const api::client::Blockchain& blockchain,
    const blockchain::Type chain,
    const VersionNumber version,
    const std::uint32_t index,
    const std::uint64_t value,
    std::unique_ptr<const internal::Script> script,
    std::optional<std::size_t> size,
    boost::container::flat_set<KeyID>&& keys,
    boost::container::flat_set<PatternID>&& pubkeyHashes,
    std::optional<PatternID>&& scriptHash,
    const bool indexed) noexcept(false)
    : api_(api)
    , crypto_(blockchain)
    , chain_(chain)
    , serialize_version_(version)
    , index_(index)
    , value_(value)
    , script_(std::move(script))
    , pubkey_hashes_(std::move(pubkeyHashes))
    , script_hash_(std::move(scriptHash))
    , cache_(api, std::move(size), std::move(keys))
{
    if (false == bool(script_)) {
        throw std::runtime_error("Invalid output script");
    }

    if (false == indexed) { index_elements(); }
}

Output::Output(
    const api::Core& api,
    const api::client::Blockchain& blockchain,
    const blockchain::Type chain,
    const std::uint32_t index,
    const std::uint64_t value,
    const std::size_t size,
    const ReadView in,
    const VersionNumber version) noexcept(false)
    : Output(
          api,
          blockchain,
          chain,
          version,
          index,
          value,
          factory::BitcoinScript(chain, in, Script::Position::Output),
          size,
          {},
          {},
          {},
          false)
{
}

Output::Output(
    const api::Core& api,
    const api::client::Blockchain& blockchain,
    const blockchain::Type chain,
    const std::uint32_t index,
    const std::uint64_t value,
    std::unique_ptr<const internal::Script> script,
    boost::container::flat_set<KeyID>&& keys,
    const VersionNumber version) noexcept(false)
    : Output(
          api,
          blockchain,
          chain,
          version,
          index,
          value,
          std::move(script),
          {},
          std::move(keys),
          {},
          {},
          false)
{
}

Output::Output(const Output& rhs) noexcept
    : api_(rhs.api_)
    , crypto_(rhs.crypto_)
    , chain_(rhs.chain_)
    , serialize_version_(rhs.serialize_version_)
    , index_(rhs.index_)
    , value_(rhs.value_)
    , script_(rhs.script_->clone())
    , pubkey_hashes_(rhs.pubkey_hashes_)
    , script_hash_(rhs.script_hash_)
    , cache_(rhs.cache_)
{
}

auto Output::AssociatedLocalNyms(
    const api::client::Blockchain& blockchain,
    std::vector<OTNymID>& output) const noexcept -> void
{
    cache_.for_each_key([&](const auto& key) {
        const auto& owner = blockchain.Owner(key);

        if (false == owner.empty()) { output.emplace_back(owner); }
    });
}

auto Output::AssociatedRemoteContacts(
    const api::client::Blockchain& blockchain,
    std::vector<OTIdentifier>& output) const noexcept -> void
{
    const auto hashes = script_->LikelyPubkeyHashes(api_);
    std::for_each(std::begin(hashes), std::end(hashes), [&](const auto& hash) {
        auto contacts = blockchain.LookupContacts(hash);
        std::move(
            std::begin(contacts),
            std::end(contacts),
            std::back_inserter(output));
    });

    auto payer = cache_.payer();
    auto payee = cache_.payee();

    if (false == payee->empty()) { output.emplace_back(std::move(payee)); }
    if (false == payer->empty()) { output.emplace_back(std::move(payer)); }
}

auto Output::CalculateSize() const noexcept -> std::size_t
{
    return cache_.size([&] {
        const auto scriptCS =
            blockchain::bitcoin::CompactSize(script_->CalculateSize());

        return sizeof(value_) + scriptCS.Total();
    });
}

auto Output::ExtractElements(const filter::Type style) const noexcept
    -> std::vector<Space>
{
    return script_->ExtractElements(style);
}

auto Output::FindMatches(
    const ReadView txid,
    const FilterType type,
    const ParsedPatterns& patterns) const noexcept -> Matches
{
    const auto output =
        SetIntersection(api_, txid, patterns, ExtractElements(type));
    LogTrace(OT_METHOD)(__func__)(": Verified ")(output.second.size())(
        " pattern matches")
        .Flush();
    std::for_each(
        std::begin(output.second),
        std::end(output.second),
        [this](const auto& match) {
            const auto& [txid, element] = match;
            const auto& [index, subchainID] = element;
            const auto& [subchain, account] = subchainID;
            auto keyid = KeyID{account->str(), subchain, index};

            if (crypto::Subchain::Outgoing == subchain) {
                auto sender = crypto_.SenderContact(keyid);
                auto recipient = crypto_.RecipientContact(keyid);

                if (sender->empty()) { OT_FAIL; }
                if (recipient->empty()) { OT_FAIL; }

                cache_.set_payer(sender);
                cache_.set_payee(recipient);
            } else {
                cache_.add(std::move(keyid));
            }
        });

    return output;
}

auto Output::GetPatterns() const noexcept -> std::vector<PatternID>
{
    return {std::begin(pubkey_hashes_), std::end(pubkey_hashes_)};
}

auto Output::index_elements() noexcept -> void
{
    auto& hashes =
        const_cast<boost::container::flat_set<PatternID>&>(pubkey_hashes_);
    const auto patterns = script_->ExtractPatterns(api_, crypto_);
    LogTrace(OT_METHOD)(__func__)(": ")(patterns.size())(
        " pubkey hashes found:")
        .Flush();
    std::for_each(
        std::begin(patterns), std::end(patterns), [&](const auto& id) -> auto {
            hashes.emplace(id);
            LogTrace("    * ")(id).Flush();
        });
    const auto scriptHash = script_->ScriptHash();

    if (scriptHash.has_value()) {
        const_cast<std::optional<PatternID>&>(script_hash_) =
            crypto_.IndexItem(scriptHash.value());
    }
}

auto Output::MergeMetadata(const SerializeType& rhs) noexcept -> void
{
    std::for_each(
        std::begin(rhs.key()), std::end(rhs.key()), [this](const auto& key) {
            const auto subchain = static_cast<crypto::Subchain>(
                static_cast<std::uint8_t>(key.subchain()));

            if (crypto::Subchain::Outgoing == subchain) {
                LogOutput(OT_METHOD)(__func__)(": discarding invalid key")
                    .Flush();
            } else {
                cache_.add({key.subaccount(), subchain, key.index()});
            }
        });

    if (cache_.payer()->empty() && false == rhs.payer().empty()) {
        SetPayer([&] {
            auto id = api_.Factory().Identifier();
            auto& value = rhs.payer(0);
            id->Assign(value.data(), value.size());

            return id;
        }());
    }

    if (cache_.payee()->empty() && false == rhs.payee().empty()) {
        SetPayee([&] {
            auto id = api_.Factory().Identifier();
            auto& value = rhs.payee(0);
            id->Assign(value.data(), value.size());

            return id;
        }());
    }
}

auto Output::NetBalanceChange(
    const api::client::Blockchain& blockchain,
    const identifier::Nym& nym) const noexcept -> opentxs::Amount
{
    auto done{false};
    auto output = opentxs::Amount{0};
    cache_.for_each_key([&](const auto& key) {
        if (done) { return; }

        if (nym == blockchain.Owner(key)) {
            done = true;
            output = value_;
        }
    });

    return output;
}

auto Output::Note(const api::client::Blockchain& blockchain) const noexcept
    -> std::string
{
    auto done{false};
    auto output = std::string{};
    cache_.for_each_key([&](const auto& key) {
        if (done) { return; }

        try {
            const auto& element = blockchain.GetKey(key);
            const auto note = element.Label();

            if (false == note.empty()) {
                done = true;
                output = note;
            }
        } catch (...) {
        }
    });

    return output;
}

auto Output::Print() const noexcept -> std::string
{
    auto out = std::stringstream{};
    out << "    value: " << std::to_string(value_) << '\n';
    out << "    script: " << '\n';
    out << script_->Print();

    return out.str();
}

auto Output::Serialize(const AllocateOutput destination) const noexcept
    -> std::optional<std::size_t>
{
    if (!destination) {
        LogOutput(OT_METHOD)(__func__)(": Invalid output allocator").Flush();

        return std::nullopt;
    }

    const auto size = CalculateSize();
    auto output = destination(size);

    if (false == output.valid(size)) {
        LogOutput(OT_METHOD)(__func__)(": Failed to allocate output bytes")
            .Flush();

        return std::nullopt;
    }

    const auto scriptCS =
        blockchain::bitcoin::CompactSize(script_->CalculateSize());
    const auto csData = scriptCS.Encode();
    auto it = static_cast<std::byte*>(output.data());
    std::memcpy(static_cast<void*>(it), &value_, sizeof(value_));
    std::advance(it, sizeof(value_));
    std::memcpy(static_cast<void*>(it), csData.data(), csData.size());
    std::advance(it, csData.size());

    if (script_->Serialize(preallocated(scriptCS.Value(), it))) {

        return size;
    } else {
        LogOutput(OT_METHOD)(__func__)(": Failed to serialize script").Flush();

        return std::nullopt;
    }
}

auto Output::Serialize(
    const api::client::Blockchain& blockchain,
    SerializeType& out) const noexcept -> bool
{
    out.set_version(std::max(default_version_, serialize_version_));
    out.set_index(index_);
    out.set_value(value_);

    if (false == script_->Serialize(writer(*out.mutable_script()))) {
        return false;
    }

    cache_.for_each_key([&](const auto& key) {
        const auto& [accountID, subchain, index] = key;
        auto& serializedKey = *out.add_key();
        serializedKey.set_version(key_version_);
        serializedKey.set_chain(
            contact::internal::translate(Translate(chain_)));
        serializedKey.set_nym(blockchain.Owner(key).str());
        serializedKey.set_subaccount(accountID);
        serializedKey.set_subchain(static_cast<std::uint32_t>(subchain));
        serializedKey.set_index(index);
    });

    for (const auto& id : pubkey_hashes_) { out.add_pubkey_hash(id); }

    if (script_hash_.has_value()) { out.set_script_hash(script_hash_.value()); }

    out.set_indexed(true);

    if (const auto payer = cache_.payer(); false == payer->empty()) {
        out.add_payer(std::string{payer->Bytes()});
    }

    if (const auto payee = cache_.payee(); false == payee->empty()) {
        out.add_payee(std::string{payee->Bytes()});
    }

    return true;
}
}  // namespace opentxs::blockchain::block::bitcoin::implementation
