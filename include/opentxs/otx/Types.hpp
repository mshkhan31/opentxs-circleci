// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_OTX_TYPES_HPP
#define OPENTXS_OTX_TYPES_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <cstddef>
#include <cstdint>

namespace opentxs
{
namespace otx
{
enum class ConsensusType : std::uint8_t;
enum class LastReplyStatus : std::uint8_t;
enum class OTXPushType : std::uint8_t;
enum class OperationType : std::uint16_t;
enum class ServerReplyType : std::uint8_t;
enum class ServerRequestType : std::uint8_t;
}  // namespace otx

constexpr auto value(const otx::ConsensusType in) noexcept
{
    return static_cast<std::uint8_t>(in);
}
constexpr auto value(const otx::LastReplyStatus in) noexcept
{
    return static_cast<std::uint8_t>(in);
}
constexpr auto value(const otx::OTXPushType in) noexcept
{
    return static_cast<std::uint8_t>(in);
}
constexpr auto value(const otx::OperationType in) noexcept
{
    return static_cast<std::uint16_t>(in);
}
constexpr auto value(const otx::ServerReplyType in) noexcept
{
    return static_cast<std::uint8_t>(in);
}
constexpr auto value(const otx::ServerRequestType in) noexcept
{
    return static_cast<std::uint8_t>(in);
}
}  // namespace opentxs
#endif
