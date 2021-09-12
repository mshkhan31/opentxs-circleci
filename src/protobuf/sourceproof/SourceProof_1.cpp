// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/protobuf/verify/SourceProof.hpp"  // IWYU pragma: associated

#include "opentxs/protobuf/Enums.pb.h"
#include "opentxs/protobuf/SourceProof.pb.h"
#include "protobuf/Check.hpp"

#define PROTO_NAME "source proof"

namespace opentxs
{
namespace proto
{

auto CheckProto_1(
    const SourceProof& input,
    const bool silent,
    bool& ExpectSourceSignature) -> bool
{
    if (!input.has_type()) { FAIL_1("missing type") }

    switch (input.type()) {
        case SOURCEPROOFTYPE_SELF_SIGNATURE: {
            ExpectSourceSignature = false;
        } break;
        case SOURCEPROOFTYPE_SIGNATURE: {
            ExpectSourceSignature = true;
        } break;
        case SOURCEPROOFTYPE_ERROR:
        default:
            FAIL_2("incorrect or unknown type", input.type())
    }

    return true;
}

auto CheckProto_2(const SourceProof& input, const bool silent, bool&) -> bool
{
    UNDEFINED_VERSION(2)
}

auto CheckProto_3(const SourceProof& input, const bool silent, bool&) -> bool
{
    UNDEFINED_VERSION(3)
}

auto CheckProto_4(const SourceProof& input, const bool silent, bool&) -> bool
{
    UNDEFINED_VERSION(4)
}

auto CheckProto_5(const SourceProof& input, const bool silent, bool&) -> bool
{
    UNDEFINED_VERSION(5)
}

auto CheckProto_6(const SourceProof& input, const bool silent, bool&) -> bool
{
    UNDEFINED_VERSION(6)
}

auto CheckProto_7(const SourceProof& input, const bool silent, bool&) -> bool
{
    UNDEFINED_VERSION(7)
}

auto CheckProto_8(const SourceProof& input, const bool silent, bool&) -> bool
{
    UNDEFINED_VERSION(8)
}

auto CheckProto_9(const SourceProof& input, const bool silent, bool&) -> bool
{
    UNDEFINED_VERSION(9)
}

auto CheckProto_10(const SourceProof& input, const bool silent, bool&) -> bool
{
    UNDEFINED_VERSION(10)
}

auto CheckProto_11(const SourceProof& input, const bool silent, bool&) -> bool
{
    UNDEFINED_VERSION(11)
}

auto CheckProto_12(const SourceProof& input, const bool silent, bool&) -> bool
{
    UNDEFINED_VERSION(12)
}

auto CheckProto_13(const SourceProof& input, const bool silent, bool&) -> bool
{
    UNDEFINED_VERSION(13)
}

auto CheckProto_14(const SourceProof& input, const bool silent, bool&) -> bool
{
    UNDEFINED_VERSION(14)
}

auto CheckProto_15(const SourceProof& input, const bool silent, bool&) -> bool
{
    UNDEFINED_VERSION(15)
}

auto CheckProto_16(const SourceProof& input, const bool silent, bool&) -> bool
{
    UNDEFINED_VERSION(16)
}

auto CheckProto_17(const SourceProof& input, const bool silent, bool&) -> bool
{
    UNDEFINED_VERSION(17)
}

auto CheckProto_18(const SourceProof& input, const bool silent, bool&) -> bool
{
    UNDEFINED_VERSION(18)
}

auto CheckProto_19(const SourceProof& input, const bool silent, bool&) -> bool
{
    UNDEFINED_VERSION(19)
}

auto CheckProto_20(const SourceProof& input, const bool silent, bool&) -> bool
{
    UNDEFINED_VERSION(20)
}
}  // namespace proto
}  // namespace opentxs
