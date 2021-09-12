// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <list>
#include <mutex>
#include <string>

#include "opentxs/Types.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/contract/Signable.hpp"

namespace opentxs
{
namespace api
{
class Core;
}  // namespace api

namespace proto
{
class Signature;
}  // namespace proto

class PasswordPrompt;
}  // namespace opentxs

namespace opentxs::contract::implementation
{
class Signable : virtual public opentxs::contract::Signable
{
public:
    auto Alias() const -> std::string override;
    auto ID() const -> OTIdentifier override;
    auto Nym() const -> Nym_p override;
    auto Terms() const -> const std::string& override;
    auto Validate() const -> bool override;
    auto Version() const -> VersionNumber override;

    auto SetAlias(const std::string& alias) -> void override;

    ~Signable() override = default;

protected:
    using Signatures = std::list<Signature>;

    const api::Core& api_;
    mutable std::mutex lock_;
    const Nym_p nym_;
    const VersionNumber version_;
    const std::string conditions_;
    const OTIdentifier id_;
    Signatures signatures_;
    std::string alias_;

    auto CheckID(const Lock& lock) const -> bool;
    virtual auto id(const Lock& lock) const -> OTIdentifier;
    virtual auto validate(const Lock& lock) const -> bool = 0;
    virtual auto verify_signature(
        const Lock& lock,
        const proto::Signature& signature) const -> bool;
    auto verify_write_lock(const Lock& lock) const -> bool;

    auto clear_signatures(const Lock& lock) noexcept -> void;
    virtual auto first_time_init(const Lock& lock) noexcept(false) -> void;
    virtual auto init_serialized(const Lock& lock) noexcept(false) -> void;
    virtual auto update_signature(
        const Lock& lock,
        const PasswordPrompt& reason) -> bool;
    auto update_version(const Lock& lock, const VersionNumber version) noexcept
        -> void;

    virtual auto GetID(const Lock& lock) const -> OTIdentifier = 0;

    Signable(
        const api::Core& api,
        const Nym_p& nym,
        const VersionNumber version,
        const std::string& conditions,
        const std::string& alias) noexcept;
    Signable(
        const api::Core& api,
        const Nym_p& nym,
        const VersionNumber version,
        const std::string& conditions,
        const std::string& alias,
        OTIdentifier&& id,
        Signatures&& signatures) noexcept;
    Signable(const Signable&) noexcept;
    Signable(Signable&&) = delete;
    auto operator=(const Signable&) -> Signable& = delete;
    auto operator=(Signable&&) -> Signable& = delete;
};
}  // namespace opentxs::contract::implementation
