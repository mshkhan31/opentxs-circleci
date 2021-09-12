// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/protobuf/verify/StorageContactNymIndex.hpp"  // IWYU pragma: associated

#include <string>

#include "opentxs/protobuf/StorageContactNymIndex.pb.h"
#include "protobuf/Check.hpp"

#define PROTO_NAME "storage contact nym index"

namespace opentxs::proto
{
auto CheckProto_1(const StorageContactNymIndex& input, const bool silent)
    -> bool
{
    if (false == input.has_contact()) { FAIL_1("missing contact") }

    if (MIN_PLAUSIBLE_IDENTIFIER > input.contact().size()) {
        FAIL_2("invalid contact", input.contact())
    }

    if (MAX_PLAUSIBLE_IDENTIFIER < input.contact().size()) {
        FAIL_2("invalid contact", input.contact())
    }

    for (const auto& it : input.nym()) {
        if (MIN_PLAUSIBLE_IDENTIFIER > it.size()) { FAIL_2("invalid nym", it) }

        if (MAX_PLAUSIBLE_IDENTIFIER < it.size()) { FAIL_2("invalid nym", it) }
    }

    return true;
}

auto CheckProto_2(const StorageContactNymIndex& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(2)
}

auto CheckProto_3(const StorageContactNymIndex& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(3)
}

auto CheckProto_4(const StorageContactNymIndex& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(4)
}

auto CheckProto_5(const StorageContactNymIndex& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(5)
}

auto CheckProto_6(const StorageContactNymIndex& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(6)
}

auto CheckProto_7(const StorageContactNymIndex& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(7)
}

auto CheckProto_8(const StorageContactNymIndex& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(8)
}

auto CheckProto_9(const StorageContactNymIndex& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(9)
}

auto CheckProto_10(const StorageContactNymIndex& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(10)
}

auto CheckProto_11(const StorageContactNymIndex& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(11)
}

auto CheckProto_12(const StorageContactNymIndex& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(12)
}

auto CheckProto_13(const StorageContactNymIndex& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(13)
}

auto CheckProto_14(const StorageContactNymIndex& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(14)
}

auto CheckProto_15(const StorageContactNymIndex& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(15)
}

auto CheckProto_16(const StorageContactNymIndex& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(16)
}

auto CheckProto_17(const StorageContactNymIndex& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(17)
}

auto CheckProto_18(const StorageContactNymIndex& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(18)
}

auto CheckProto_19(const StorageContactNymIndex& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(19)
}

auto CheckProto_20(const StorageContactNymIndex& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(20)
}
}  // namespace opentxs::proto
