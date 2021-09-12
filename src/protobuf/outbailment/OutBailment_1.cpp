// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/protobuf/verify/OutBailment.hpp"  // IWYU pragma: associated

#include <string>

#include "opentxs/protobuf/OutBailment.pb.h"
#include "protobuf/Check.hpp"

#define PROTO_NAME "outbailment"

namespace opentxs
{
namespace proto
{

auto CheckProto_1(const OutBailment& input, const bool silent) -> bool
{
    if (!input.has_unitid()) { FAIL_1("missing unit id") }

    if (MIN_PLAUSIBLE_IDENTIFIER > input.unitid().size()) {
        FAIL_2("invalid unit id", input.unitid())
    }

    if (MAX_PLAUSIBLE_IDENTIFIER < input.unitid().size()) {
        FAIL_2("invalid unit id", input.unitid())
    }

    if (!input.has_serverid()) { FAIL_1("missing server id") }

    if (MIN_PLAUSIBLE_IDENTIFIER > input.serverid().size()) {
        FAIL_2("invalid server id", input.serverid())
    }

    if (MAX_PLAUSIBLE_IDENTIFIER < input.serverid().size()) {
        FAIL_2("invalid server id", input.serverid())
    }

    if (!input.has_amount()) { FAIL_1("missing amount") }

    if (!input.has_instructions()) { FAIL_1("missing instructions") }

    CHECK_NONE(payment)

    return true;
}

auto CheckProto_2(const OutBailment& input, const bool silent) -> bool
{

    return CheckProto_1(input, silent);
}

auto CheckProto_3(const OutBailment& input, const bool silent) -> bool
{

    return CheckProto_1(input, silent);
}

auto CheckProto_4(const OutBailment& input, const bool silent) -> bool
{

    return CheckProto_1(input, silent);
}

auto CheckProto_5(const OutBailment& input, const bool silent) -> bool
{
    if (!input.has_unitid()) { FAIL_1("missing unit id") }

    if (MIN_PLAUSIBLE_IDENTIFIER > input.unitid().size()) {
        FAIL_2("invalid unit id", input.unitid())
    }

    if (MAX_PLAUSIBLE_IDENTIFIER < input.unitid().size()) {
        FAIL_2("invalid unit id", input.unitid())
    }

    if (!input.has_serverid()) { FAIL_1("missing server id") }

    if (MIN_PLAUSIBLE_IDENTIFIER > input.serverid().size()) {
        FAIL_2("invalid server id", input.serverid())
    }

    if (MAX_PLAUSIBLE_IDENTIFIER < input.serverid().size()) {
        FAIL_2("invalid server id", input.serverid())
    }

    if (!input.has_amount()) { FAIL_1("missing amount") }

    if (!input.has_instructions()) { FAIL_1("missing instructions") }

    return true;
}

auto CheckProto_6(const OutBailment& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(6)
}

auto CheckProto_7(const OutBailment& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(7)
}

auto CheckProto_8(const OutBailment& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(8)
}

auto CheckProto_9(const OutBailment& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(9)
}

auto CheckProto_10(const OutBailment& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(10)
}

auto CheckProto_11(const OutBailment& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(11)
}

auto CheckProto_12(const OutBailment& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(12)
}

auto CheckProto_13(const OutBailment& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(13)
}

auto CheckProto_14(const OutBailment& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(14)
}

auto CheckProto_15(const OutBailment& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(15)
}

auto CheckProto_16(const OutBailment& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(16)
}

auto CheckProto_17(const OutBailment& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(17)
}

auto CheckProto_18(const OutBailment& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(18)
}

auto CheckProto_19(const OutBailment& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(19)
}

auto CheckProto_20(const OutBailment& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(20)
}
}  // namespace proto
}  // namespace opentxs
