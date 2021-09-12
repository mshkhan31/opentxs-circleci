// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_PROTOBUF_VERIFYOTX_HPP
#define OPENTXS_PROTOBUF_VERIFYOTX_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include "opentxs/protobuf/Basic.hpp"

namespace opentxs
{
namespace proto
{
auto ServerReplyAllowedOTXPush() noexcept -> const VersionMap&;
auto ServerReplyAllowedSignature() noexcept -> const VersionMap&;
auto ServerRequestAllowedNym() noexcept -> const VersionMap&;
auto ServerRequestAllowedSignature() noexcept -> const VersionMap&;
}  // namespace proto
}  // namespace opentxs
#endif  // OPENTXS_PROTOBUF_VERIFYOTX_HPP
