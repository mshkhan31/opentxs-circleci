// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/protobuf/verify/VerifyStorage.hpp"  // IWYU pragma: associated

#include <stdexcept>
#include <string>
#include <utility>

#include "opentxs/protobuf/Basic.hpp"
#include "opentxs/protobuf/Check.hpp"
#include "opentxs/protobuf/StorageThread.pb.h"
#include "opentxs/protobuf/StorageThreadItem.pb.h"
#include "opentxs/protobuf/verify/StorageThread.hpp"
#include "opentxs/protobuf/verify/StorageThreadItem.hpp"
#include "protobuf/Check.hpp"

#define PROTO_NAME "storage thread"

namespace opentxs
{
namespace proto
{

auto CheckProto_1(const StorageThread& input, const bool silent) -> bool
{
    if (!input.has_id()) { FAIL_1("missing id") }

    if (MIN_PLAUSIBLE_IDENTIFIER > input.id().size()) { FAIL_1("invalid id") }

    for (auto& nym : input.participant()) {
        if (MIN_PLAUSIBLE_IDENTIFIER > nym.size()) {
            FAIL_1("invalid participant")
        }
    }

    if (0 == input.participant_size()) { FAIL_1("no patricipants") }

    for (auto& item : input.item()) {
        try {
            const bool valid = Check(
                item,
                StorageThreadAllowedItem().at(input.version()).first,
                StorageThreadAllowedItem().at(input.version()).second,
                silent);

            if (false == valid) { FAIL_1("invalid item") }
        } catch (const std::out_of_range&) {
            FAIL_2(
                "allowed storage item hash version not defined for version",
                input.version())
        }
    }

    return true;
}

auto CheckProto_2(const StorageThread& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(2)
}

auto CheckProto_3(const StorageThread& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(3)
}

auto CheckProto_4(const StorageThread& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(4)
}

auto CheckProto_5(const StorageThread& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(5)
}

auto CheckProto_6(const StorageThread& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(6)
}

auto CheckProto_7(const StorageThread& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(7)
}

auto CheckProto_8(const StorageThread& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(8)
}

auto CheckProto_9(const StorageThread& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(9)
}

auto CheckProto_10(const StorageThread& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(10)
}

auto CheckProto_11(const StorageThread& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(11)
}

auto CheckProto_12(const StorageThread& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(12)
}

auto CheckProto_13(const StorageThread& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(13)
}

auto CheckProto_14(const StorageThread& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(14)
}

auto CheckProto_15(const StorageThread& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(15)
}

auto CheckProto_16(const StorageThread& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(16)
}

auto CheckProto_17(const StorageThread& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(17)
}

auto CheckProto_18(const StorageThread& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(18)
}

auto CheckProto_19(const StorageThread& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(19)
}

auto CheckProto_20(const StorageThread& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(20)
}
}  // namespace proto
}  // namespace opentxs
