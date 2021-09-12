// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/protobuf/verify/VerificationSet.hpp"  // IWYU pragma: associated

#include <stdexcept>
#include <string>
#include <utility>

#include "opentxs/protobuf/Basic.hpp"
#include "opentxs/protobuf/Check.hpp"
#include "opentxs/protobuf/VerificationGroup.pb.h"
#include "opentxs/protobuf/VerificationSet.pb.h"
#include "opentxs/protobuf/verify/VerificationGroup.hpp"
#include "opentxs/protobuf/verify/VerifyContacts.hpp"
#include "protobuf/Check.hpp"

#define PROTO_NAME "verification set"

namespace opentxs
{
namespace proto
{

auto CheckProto_1(
    const VerificationSet& input,
    const bool silent,
    const VerificationType indexed) -> bool
{
    if (input.has_internal()) {
        try {
            const bool validInternal = Check(
                input.internal(),
                VerificationSetAllowedGroup().at(input.version()).first,
                VerificationSetAllowedGroup().at(input.version()).second,
                silent,
                indexed);

            if (!validInternal) { FAIL_1("invalid internal group") }
        } catch (const std::out_of_range&) {
            FAIL_2(
                "allowed verification group version not defined for version",
                input.version())
        }
    }

    if (input.has_external()) {
        try {
            const bool validExternal = Check(
                input.external(),
                VerificationSetAllowedGroup().at(input.version()).first,
                VerificationSetAllowedGroup().at(input.version()).second,
                silent,
                indexed);

            if (!validExternal) { FAIL_1("invalid external group") }
        } catch (const std::out_of_range&) {
            FAIL_2(
                "allowed verification group version not defined for version",
                input.version())
        }
    }

    for (auto& it : input.repudiated()) {
        if (MIN_PLAUSIBLE_IDENTIFIER < it.size()) {
            FAIL_1("invalid repudiation")
        }
    }

    return true;
}

auto CheckProto_2(
    const VerificationSet& input,
    const bool silent,
    const VerificationType) -> bool
{
    UNDEFINED_VERSION(2)
}

auto CheckProto_3(
    const VerificationSet& input,
    const bool silent,
    const VerificationType) -> bool
{
    UNDEFINED_VERSION(3)
}

auto CheckProto_4(
    const VerificationSet& input,
    const bool silent,
    const VerificationType) -> bool
{
    UNDEFINED_VERSION(4)
}

auto CheckProto_5(
    const VerificationSet& input,
    const bool silent,
    const VerificationType) -> bool
{
    UNDEFINED_VERSION(5)
}

auto CheckProto_6(
    const VerificationSet& input,
    const bool silent,
    const VerificationType) -> bool
{
    UNDEFINED_VERSION(6)
}

auto CheckProto_7(
    const VerificationSet& input,
    const bool silent,
    const VerificationType) -> bool
{
    UNDEFINED_VERSION(7)
}

auto CheckProto_8(
    const VerificationSet& input,
    const bool silent,
    const VerificationType) -> bool
{
    UNDEFINED_VERSION(8)
}

auto CheckProto_9(
    const VerificationSet& input,
    const bool silent,
    const VerificationType) -> bool
{
    UNDEFINED_VERSION(9)
}

auto CheckProto_10(
    const VerificationSet& input,
    const bool silent,
    const VerificationType) -> bool
{
    UNDEFINED_VERSION(10)
}

auto CheckProto_11(
    const VerificationSet& input,
    const bool silent,
    const VerificationType) -> bool
{
    UNDEFINED_VERSION(11)
}

auto CheckProto_12(
    const VerificationSet& input,
    const bool silent,
    const VerificationType) -> bool
{
    UNDEFINED_VERSION(12)
}

auto CheckProto_13(
    const VerificationSet& input,
    const bool silent,
    const VerificationType) -> bool
{
    UNDEFINED_VERSION(13)
}

auto CheckProto_14(
    const VerificationSet& input,
    const bool silent,
    const VerificationType) -> bool
{
    UNDEFINED_VERSION(14)
}

auto CheckProto_15(
    const VerificationSet& input,
    const bool silent,
    const VerificationType) -> bool
{
    UNDEFINED_VERSION(15)
}

auto CheckProto_16(
    const VerificationSet& input,
    const bool silent,
    const VerificationType) -> bool
{
    UNDEFINED_VERSION(16)
}

auto CheckProto_17(
    const VerificationSet& input,
    const bool silent,
    const VerificationType) -> bool
{
    UNDEFINED_VERSION(17)
}

auto CheckProto_18(
    const VerificationSet& input,
    const bool silent,
    const VerificationType) -> bool
{
    UNDEFINED_VERSION(18)
}

auto CheckProto_19(
    const VerificationSet& input,
    const bool silent,
    const VerificationType) -> bool
{
    UNDEFINED_VERSION(19)
}

auto CheckProto_20(
    const VerificationSet& input,
    const bool silent,
    const VerificationType) -> bool
{
    UNDEFINED_VERSION(20)
}
}  // namespace proto
}  // namespace opentxs
