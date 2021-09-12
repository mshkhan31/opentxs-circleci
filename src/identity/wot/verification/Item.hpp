// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Proto.hpp"
#include "internal/identity/wot/verification/Verification.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/identity/wot/verification/Item.hpp"
#include "opentxs/protobuf/Signature.pb.h"

namespace opentxs
{
namespace api
{
class Core;
}  // namespace api

namespace identifier
{
class Nym;
}  // namespace identifier

namespace identity
{
class Nym;
}  // namespace identity

class Factory;
class PasswordPrompt;
}  // namespace opentxs

namespace opentxs::identity::wot::verification::implementation
{
class Item final : public internal::Item
{
public:
    operator SerializedType() const noexcept final;

    auto Begin() const noexcept -> Time final { return start_; }
    auto ClaimID() const noexcept -> const Identifier& final { return claim_; }
    auto End() const noexcept -> Time final { return end_; }
    auto ID() const noexcept -> const Identifier& final { return id_; }
    auto Signature() const noexcept -> const proto::Signature& final
    {
        return sig_;
    }
    auto Valid() const noexcept -> Validity final { return valid_; }
    auto Value() const noexcept -> Type final { return value_; }
    auto Version() const noexcept -> VersionNumber final { return version_; }

    ~Item() final = default;

private:
    friend opentxs::Factory;

    const VersionNumber version_;
    const OTIdentifier claim_;
    const Type value_;
    const Validity valid_;
    const Time start_;
    const Time end_;
    const OTIdentifier id_;
    const proto::Signature sig_;

    static auto calculate_id(
        const api::Core& api,
        const VersionNumber version,
        const Identifier& claim,
        const Type value,
        const Time start,
        const Time end,
        const Validity valid,
        const identifier::Nym& nym) noexcept(false) -> OTIdentifier;
    static auto get_sig(
        const identity::Nym& signer,
        const VersionNumber version,
        const Identifier& id,
        const Identifier& claim,
        const Type value,
        const Time start,
        const Time end,
        const Validity valid,
        const PasswordPrompt& reason) noexcept(false) -> proto::Signature;
    static auto id_form(
        const VersionNumber version,
        const Identifier& claim,
        const Type value,
        const Time start,
        const Time end,
        const Validity valid) noexcept -> SerializedType;
    static auto sig_form(
        const VersionNumber version,
        const Identifier& id,
        const Identifier& claim,
        const Type value,
        const Time start,
        const Time end,
        const Validity valid) noexcept -> SerializedType;

    Item(
        const internal::Nym& parent,
        const Identifier& claim,
        const identity::Nym& signer,
        const PasswordPrompt& reason,
        const Type value = Type::Confirm,
        const Time start = {},
        const Time end = {},
        const VersionNumber version = DefaultVersion) noexcept(false);
    Item(
        const internal::Nym& parent,
        const SerializedType& serialized) noexcept(false);
    Item() = delete;
    Item(const Item&) = delete;
    Item(Item&&) = delete;
    auto operator=(const Item&) -> Item& = delete;
    auto operator=(Item&&) -> Item& = delete;
};
}  // namespace opentxs::identity::wot::verification::implementation
