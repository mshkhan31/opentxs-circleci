// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "internal/blockchain/block/bitcoin/Bitcoin.hpp"
#include "opentxs/Bytes.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/FilterType.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/bitcoin/Script.hpp"
#include "opentxs/core/Data.hpp"

namespace opentxs
{
namespace api
{
namespace client
{
class Blockchain;
}  // namespace client

class Core;
}  // namespace api

class PaymentCode;
}  // namespace opentxs

namespace opentxs::blockchain::block::bitcoin::implementation
{
class Script final : public internal::Script
{
public:
    static auto decode(const std::byte in) noexcept(false) -> OP;
    static auto is_direct_push(const OP opcode) noexcept(false)
        -> std::optional<std::size_t>;
    static auto is_push(const OP opcode) noexcept(false)
        -> std::optional<std::size_t>;
    static auto validate(const ScriptElements& elements) noexcept -> bool;

    auto at(const std::size_t position) const noexcept(false)
        -> const value_type& final
    {
        return elements_.at(position);
    }
    auto begin() const noexcept -> const_iterator final { return cbegin(); }
    auto CalculateHash160(const api::Core& api, const AllocateOutput output)
        const noexcept -> bool final;
    auto CalculateSize() const noexcept -> std::size_t final;
    auto cbegin() const noexcept -> const_iterator final
    {
        return const_iterator(this, 0);
    }
    auto cend() const noexcept -> const_iterator final
    {
        return const_iterator(this, elements_.size());
    }
    auto end() const noexcept -> const_iterator final { return cend(); }
    auto ExtractElements(const filter::Type style) const noexcept
        -> std::vector<Space> final;
    auto ExtractPatterns(
        const api::Core& api,
        const api::client::Blockchain& blockchain) const noexcept
        -> std::vector<PatternID> final;
    auto IsNotification(
        const std::uint8_t version,
        const PaymentCode& recipient) const noexcept -> bool final;
    auto LikelyPubkeyHashes(const api::Core& api) const noexcept
        -> std::vector<OTData> final;
    auto M() const noexcept -> std::optional<std::uint8_t> final;
    auto MultisigPubkey(const std::size_t position) const noexcept
        -> std::optional<ReadView> final;
    auto N() const noexcept -> std::optional<std::uint8_t> final;
    auto Print() const noexcept -> std::string final;
    auto Pubkey() const noexcept -> std::optional<ReadView> final;
    auto PubkeyHash() const noexcept -> std::optional<ReadView> final;
    auto RedeemScript() const noexcept
        -> std::unique_ptr<bitcoin::Script> final;
    auto Role() const noexcept -> Position final { return role_; }
    auto clone() const noexcept -> std::unique_ptr<internal::Script> final
    {
        return std::make_unique<Script>(*this);
    }
    auto ScriptHash() const noexcept -> std::optional<ReadView> final;
    auto Serialize(const AllocateOutput destination) const noexcept
        -> bool final;
    auto SigningSubscript(const blockchain::Type chain) const noexcept
        -> std::unique_ptr<internal::Script> final;
    auto size() const noexcept -> std::size_t final { return elements_.size(); }
    auto Type() const noexcept -> Pattern final { return type_; }
    auto Value(const std::size_t position) const noexcept
        -> std::optional<ReadView> final;

    Script(
        const blockchain::Type chain,
        const Position role,
        ScriptElements&& elements,
        std::optional<std::size_t> size = {}) noexcept;
    Script(const Script&) noexcept;

    ~Script() final = default;

private:
    const blockchain::Type chain_;
    const Position role_;
    const ScriptElements elements_;
    const Pattern type_;
    mutable std::optional<std::size_t> size_;

    static auto bytes(const value_type& element) noexcept -> std::size_t;
    static auto bytes(const ScriptElements& script) noexcept -> std::size_t;
    static auto is_data_push(const value_type& element) noexcept -> bool;
    static auto is_hash160(const value_type& element) noexcept -> bool;
    static auto is_public_key(const value_type& element) noexcept -> bool;
    static auto evaluate_data(const ScriptElements& script) noexcept -> Pattern;
    static auto evaluate_multisig(const ScriptElements& script) noexcept
        -> Pattern;
    static auto evaluate_pubkey(const ScriptElements& script) noexcept
        -> Pattern;
    static auto evaluate_pubkey_hash(const ScriptElements& script) noexcept
        -> Pattern;
    static auto evaluate_script_hash(const ScriptElements& script) noexcept
        -> Pattern;
    static auto evaluate_segwit(const ScriptElements& script) noexcept
        -> Pattern;
    static auto first_opcode(const ScriptElements& script) noexcept -> OP;
    static auto get_type(
        const Position role,
        const ScriptElements& script) noexcept -> Pattern;
    static auto last_opcode(const ScriptElements& script) noexcept -> OP;
    static auto potential_data(const ScriptElements& script) noexcept -> bool;
    static auto potential_multisig(const ScriptElements& script) noexcept
        -> bool;
    static auto potential_pubkey(const ScriptElements& script) noexcept -> bool;
    static auto potential_pubkey_hash(const ScriptElements& script) noexcept
        -> bool;
    static auto potential_script_hash(const ScriptElements& script) noexcept
        -> bool;
    static auto potential_segwit(const ScriptElements& script) noexcept -> bool;
    static auto to_number(const OP opcode) noexcept -> std::uint8_t;
    static auto validate(
        const ScriptElement& element,
        const bool checkForData = false) noexcept -> bool;

    auto get_data(const std::size_t position) const noexcept(false) -> ReadView;
    auto get_opcode(const std::size_t position) const noexcept(false) -> OP;

    Script() = delete;
    Script(Script&&) = delete;
    auto operator=(const Script&) -> Script& = delete;
    auto operator=(Script&&) -> Script& = delete;
};
}  // namespace opentxs::blockchain::block::bitcoin::implementation
