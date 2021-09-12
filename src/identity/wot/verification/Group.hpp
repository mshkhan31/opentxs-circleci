// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>
#include <iosfwd>
#include <map>
#include <memory>
#include <vector>

#include "internal/identity/wot/verification/Verification.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/identity/wot/verification/Group.hpp"
#include "opentxs/identity/wot/verification/Item.hpp"

namespace opentxs
{
namespace api
{
class Core;
}  // namespace api

namespace identity
{
class Nym;
}  // namespace identity

class Factory;
class PasswordPrompt;
}  // namespace opentxs

namespace opentxs::identity::wot::verification::implementation
{
class Group final : public internal::Group
{
public:
    operator SerializedType() const noexcept final;

    auto API() const noexcept -> const api::Core& final
    {
        return parent_.API();
    }
    /// Throws std::out_of_range for invalid position
    auto at(const std::size_t position) const noexcept(false)
        -> const value_type& final
    {
        return *nyms_.at(position);
    }
    auto begin() const noexcept -> const_iterator final { return cbegin(); }
    auto cbegin() const noexcept -> const_iterator final
    {
        return const_iterator(this, 0);
    }
    auto cend() const noexcept -> const_iterator final
    {
        return const_iterator(this, nyms_.size());
    }
    auto end() const noexcept -> const_iterator final { return cend(); }
    auto External() const noexcept -> bool final { return external_; }
    auto NymID() const noexcept -> const identifier::Nym& final
    {
        return parent_.NymID();
    }
    auto size() const noexcept -> std::size_t final { return nyms_.size(); }
    auto UpgradeNymVersion(const VersionNumber version) noexcept -> bool final;
    auto Version() const noexcept -> VersionNumber final { return version_; }

    auto AddItem(
        const identifier::Nym& claimOwner,
        const Identifier& claim,
        const identity::Nym& signer,
        const PasswordPrompt& reason,
        const Item::Type value,
        const Time start,
        const Time end,
        const VersionNumber version) noexcept -> bool final;
    auto AddItem(
        const identifier::Nym& verifier,
        const Item::SerializedType verification) noexcept -> bool final;
    /// Throws std::out_of_range for invalid position
    auto at(const std::size_t position) noexcept(false) -> value_type& final
    {
        return *nyms_.at(position);
    }
    auto begin() noexcept -> iterator final { return iterator(this, 0); }
    auto DeleteItem(const Identifier& item) noexcept -> bool final;
    auto end() noexcept -> iterator final
    {
        return iterator(this, nyms_.size());
    }
    void Register(const Identifier& id, const identifier::Nym& nym) noexcept
        final;
    void Unregister(const Identifier& id) noexcept final;

    ~Group() final = default;

private:
    friend opentxs::Factory;

    using Vector = std::vector<std::unique_ptr<internal::Nym>>;

    internal::Set& parent_;
    const VersionNumber version_;
    const bool external_;
    Vector nyms_;
    std::map<OTIdentifier, OTNymID> map_;

    static auto instantiate(
        internal::Group& parent,
        const SerializedType& serialized) noexcept -> Vector;

    auto get_nym(const identifier::Nym& nym) noexcept -> internal::Nym&;

    Group(
        internal::Set& parent,
        bool external,
        const VersionNumber version = DefaultVersion) noexcept;
    Group(
        internal::Set& parent,
        const SerializedType& serialized,
        bool external) noexcept;
    Group() = delete;
    Group(const Group&) = delete;
    Group(Group&&) = delete;
    auto operator=(const Group&) -> Group& = delete;
    auto operator=(Group&&) -> Group& = delete;
};
}  // namespace opentxs::identity::wot::verification::implementation
