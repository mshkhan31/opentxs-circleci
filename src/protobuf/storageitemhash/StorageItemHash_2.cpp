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
auto CheckProto_2(const StorageItemHash& input, const bool silent) -> bool
{
    if (!input.has_itemid()) { FAIL_1("missing id") }

    if (MIN_PLAUSIBLE_IDENTIFIER > input.itemid().size()) {
        FAIL_1("invalid id")
    }

    if (!input.has_hash()) { FAIL_1("missing hash") }

    if (MIN_PLAUSIBLE_IDENTIFIER > input.hash().size()) {
        FAIL_1("invalid hash")
    }

    if (!input.has_type()) { FAIL_1("missing type") }

    switch (input.type()) {
        case STORAGEHASH_PROTO:
        case STORAGEHASH_RAW: {
        } break;
        case STORAGEHASH_ERROR:
        default: {
            FAIL_1("invalid type")
        }
    }

    return true;
}

auto CheckProto_3(const StorageItemHash& input, const bool silent) -> bool
{
    return CheckProto_2(input, silent);
}

auto CheckProto_4(const StorageItemHash& input, const bool silent) -> bool
{
    return CheckProto_2(input, silent);
}

auto CheckProto_5(const StorageItemHash& input, const bool silent) -> bool
{
    return CheckProto_2(input, silent);
}

auto CheckProto_6(const StorageItemHash& input, const bool silent) -> bool
{
    return CheckProto_2(input, silent);
}

auto CheckProto_7(const StorageItemHash& input, const bool silent) -> bool
{
    return CheckProto_2(input, silent);
}

auto CheckProto_8(const StorageItemHash& input, const bool silent) -> bool
{
    return CheckProto_2(input, silent);
}

auto CheckProto_9(const StorageItemHash& input, const bool silent) -> bool
{
    return CheckProto_2(input, silent);
}

auto CheckProto_10(const StorageItemHash& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(10)
}

auto CheckProto_11(const StorageItemHash& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(11)
}

auto CheckProto_12(const StorageItemHash& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(12)
}

auto CheckProto_13(const StorageItemHash& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(13)
}

auto CheckProto_14(const StorageItemHash& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(14)
}

auto CheckProto_15(const StorageItemHash& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(15)
}

auto CheckProto_16(const StorageItemHash& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(16)
}

auto CheckProto_17(const StorageItemHash& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(17)
}

auto CheckProto_18(const StorageItemHash& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(18)
}

auto CheckProto_19(const StorageItemHash& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(19)
}

auto CheckProto_20(const StorageItemHash& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(20)
}
}  // namespace proto
}  // namespace opentxs
