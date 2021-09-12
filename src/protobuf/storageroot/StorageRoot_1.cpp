// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/protobuf/verify/StorageRoot.hpp"  // IWYU pragma: associated

#include "opentxs/protobuf/StorageRoot.pb.h"
#include "protobuf/Check.hpp"

#define PROTO_NAME "storage root"

namespace opentxs
{
namespace proto
{
auto CheckProto_1(const StorageRoot& input, const bool silent) -> bool
{
    CHECK_IDENTIFIER(items)
    CHECK_EXCLUDED(sequence)

    return true;
}
}  // namespace proto
}  // namespace opentxs
