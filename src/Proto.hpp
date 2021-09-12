// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_PROTO_HPP
#define OPENTXS_PROTO_HPP

#include <google/protobuf/message_lite.h>

#include "opentxs/Bytes.hpp"

namespace opentxs
{
using ProtobufType = ::google::protobuf::MessageLite;

auto operator==(const ProtobufType& lhs, const ProtobufType& rhs) noexcept
    -> bool;

namespace proto
{
auto write(const ProtobufType& in, const AllocateOutput out) noexcept -> bool;
}  // namespace proto
}  // namespace opentxs
#endif
