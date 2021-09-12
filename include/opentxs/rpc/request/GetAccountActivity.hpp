// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_RPC_GET_ACCOUNT_ACTIVITY_HPP
#define OPENTXS_RPC_GET_ACCOUNT_ACTIVITY_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <set>

#include "opentxs/Types.hpp"
#include "opentxs/rpc/request/Base.hpp"

namespace opentxs
{
namespace proto
{
class RPCCommand;
}  // namespace proto
}  // namespace opentxs

namespace opentxs
{
namespace rpc
{
namespace request
{
class OPENTXS_EXPORT GetAccountActivity final : public Base
{
public:
    static auto DefaultVersion() noexcept -> VersionNumber;

    auto Accounts() const noexcept -> const Identifiers&;

    /// throws std::runtime_error for invalid constructor arguments
    GetAccountActivity(
        SessionIndex session,
        const Identifiers& accounts,
        const AssociateNyms& nyms = {}) noexcept(false);
    OPENTXS_NO_EXPORT GetAccountActivity(
        const proto::RPCCommand& serialized) noexcept(false);
    GetAccountActivity() noexcept;

    ~GetAccountActivity() final;

private:
    GetAccountActivity(const GetAccountActivity&) = delete;
    GetAccountActivity(GetAccountActivity&&) = delete;
    auto operator=(const GetAccountActivity&) -> GetAccountActivity& = delete;
    auto operator=(GetAccountActivity&&) -> GetAccountActivity& = delete;
};
}  // namespace request
}  // namespace rpc
}  // namespace opentxs
#endif
