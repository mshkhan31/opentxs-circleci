// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/protobuf/verify/StorageItemHash.hpp"  // IWYU pragma: associated

#include <string>

#include "opentxs/protobuf/StorageEnums.pb.h"
#include "opentxs/protobuf/StorageItemHash.pb.h"
#include "protobuf/Check.hpp"

#define PROTO_NAME "storage item hash"

namespace opentxs
{
namespace proto
{
auto CheckProto_1(const StorageItemHash& input, const bool silent) -> bool
{
    if (!input.has_itemid()) { FAIL_1("missing id") }

    if (MIN_PLAUSIBLE_IDENTIFIER > input.itemid().size()) {
        FAIL_1("invalid id")
    }

    if (!input.has_hash()) { FAIL_1("missing hash") }

    if (MIN_PLAUSIBLE_IDENTIFIER > input.hash().size()) {
        FAIL_1("invalid has")
    }

    if (input.has_type() && (STORAGEHASH_ERROR != input.type())) {
        FAIL_1("unexpected type field present")
    }

    return true;
}
}  // namespace proto
}  // namespace opentxs
