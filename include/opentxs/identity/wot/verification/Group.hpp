// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_IDENTITY_WOT_VERIFICATION_GROUP_HPP
#define OPENTXS_IDENTITY_WOT_VERIFICATION_GROUP_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include "opentxs/Types.hpp"
#include "opentxs/identity/wot/verification/Item.hpp"
#include "opentxs/iterator/Bidirectional.hpp"

namespace opentxs
{
namespace identifier
{
class Nym;
}  // namespace identifier

namespace identity
{
namespace wot
{
namespace verification
{
class Nym;
}  // namespace verification
}  // namespace wot
}  // namespace identity

namespace proto
{
class VerificationGroup;
}  // namespace proto

class PasswordPrompt;
}  // namespace opentxs

namespace opentxs
{
namespace identity
{
namespace wot
{
namespace verification
{
class OPENTXS_EXPORT Group
{
public:
    using value_type = Nym;
    using iterator = opentxs::iterator::Bidirectional<Group, value_type>;
    using const_iterator =
        opentxs::iterator::Bidirectional<const Group, const value_type>;
    using SerializedType = proto::VerificationGroup;

    static const VersionNumber DefaultVersion;

    OPENTXS_NO_EXPORT virtual operator SerializedType() const noexcept = 0;

    /// Throws std::out_of_range for invalid position
    virtual auto at(const std::size_t position) const noexcept(false)
        -> const value_type& = 0;
    virtual auto begin() const noexcept -> const_iterator = 0;
    virtual auto cbegin() const noexcept -> const_iterator = 0;
    virtual auto cend() const noexcept -> const_iterator = 0;
    virtual auto end() const noexcept -> const_iterator = 0;
    virtual auto size() const noexcept -> std::size_t = 0;
    virtual auto Version() const noexcept -> VersionNumber = 0;

    virtual auto AddItem(
        const identifier::Nym& claimOwner,
        const Identifier& claim,
        const identity::Nym& signer,
        const PasswordPrompt& reason,
        const Item::Type value = Item::Type::Confirm,
        const Time start = {},
        const Time end = {},
        const VersionNumber version = Item::DefaultVersion) noexcept
        -> bool = 0;
    OPENTXS_NO_EXPORT virtual auto AddItem(
        const identifier::Nym& verifier,
        const Item::SerializedType verification) noexcept -> bool = 0;
    virtual auto DeleteItem(const Identifier& item) noexcept -> bool = 0;
    /// Throws std::out_of_range for invalid position
    virtual auto at(const std::size_t position) noexcept(false)
        -> value_type& = 0;
    virtual auto begin() noexcept -> iterator = 0;
    virtual auto end() noexcept -> iterator = 0;

    virtual ~Group() = default;

protected:
    Group() = default;

private:
    Group(const Group&) = delete;
    Group(Group&&) = delete;
    auto operator=(const Group&) -> Group& = delete;
    auto operator=(Group&&) -> Group& = delete;
};
}  // namespace verification
}  // namespace wot
}  // namespace identity
}  // namespace opentxs
#endif
