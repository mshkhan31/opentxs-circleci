// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/protobuf/verify/VerificationIdentity.hpp"  // IWYU pragma: associated

#include <stdexcept>
#include <string>
#include <utility>

#include "opentxs/protobuf/Basic.hpp"
#include "opentxs/protobuf/Check.hpp"
#include "opentxs/protobuf/Verification.pb.h"
#include "opentxs/protobuf/VerificationIdentity.pb.h"
#include "opentxs/protobuf/verify/Verification.hpp"
#include "opentxs/protobuf/verify/VerifyContacts.hpp"
#include "protobuf/Check.hpp"

#define PROTO_NAME "verification identity"

namespace opentxs
{
namespace proto
{

auto CheckProto_1(
    const VerificationIdentity& input,
    const bool silent,
    VerificationNymMap& map,
    const VerificationType indexed) -> bool
{
    if (!input.has_nym()) { FAIL_1("missing nym") }

    if (MIN_PLAUSIBLE_IDENTIFIER > input.nym().size()) {
        FAIL_2("invalid nym", input.nym())
    }

    map[input.nym()] += 1;

    for (auto& it : input.verification()) {
        try {
            const bool verification = Check(
                it,
                VerificationIdentityAllowedVerification()
                    .at(input.version())
                    .first,
                VerificationIdentityAllowedVerification()
                    .at(input.version())
                    .second,
                silent,
                indexed);

            if (!verification) { FAIL_1("invalid verification") }
        } catch (const std::out_of_range&) {
            FAIL_2(
                "allowed verification version not defined for version",
                input.version())
        }
    }

    return true;
}

auto CheckProto_2(
    const VerificationIdentity& input,
    const bool silent,
    VerificationNymMap&,
    const VerificationType) -> bool
{
    UNDEFINED_VERSION(2)
}

auto CheckProto_3(
    const VerificationIdentity& input,
    const bool silent,
    VerificationNymMap&,
    const VerificationType) -> bool
{
    UNDEFINED_VERSION(3)
}

auto CheckProto_4(
    const VerificationIdentity& input,
    const bool silent,
    VerificationNymMap&,
    const VerificationType) -> bool
{
    UNDEFINED_VERSION(4)
}

auto CheckProto_5(
    const VerificationIdentity& input,
    const bool silent,
    VerificationNymMap&,
    const VerificationType) -> bool
{
    UNDEFINED_VERSION(5)
}

auto CheckProto_6(
    const VerificationIdentity& input,
    const bool silent,
    VerificationNymMap&,
    const VerificationType) -> bool
{
    UNDEFINED_VERSION(6)
}

auto CheckProto_7(
    const VerificationIdentity& input,
    const bool silent,
    VerificationNymMap&,
    const VerificationType) -> bool
{
    UNDEFINED_VERSION(7)
}

auto CheckProto_8(
    const VerificationIdentity& input,
    const bool silent,
    VerificationNymMap&,
    const VerificationType) -> bool
{
    UNDEFINED_VERSION(8)
}

auto CheckProto_9(
    const VerificationIdentity& input,
    const bool silent,
    VerificationNymMap&,
    const VerificationType) -> bool
{
    UNDEFINED_VERSION(9)
}

auto CheckProto_10(
    const VerificationIdentity& input,
    const bool silent,
    VerificationNymMap&,
    const VerificationType) -> bool
{
    UNDEFINED_VERSION(10)
}

auto CheckProto_11(
    const VerificationIdentity& input,
    const bool silent,
    VerificationNymMap&,
    const VerificationType) -> bool
{
    UNDEFINED_VERSION(11)
}

auto CheckProto_12(
    const VerificationIdentity& input,
    const bool silent,
    VerificationNymMap&,
    const VerificationType) -> bool
{
    UNDEFINED_VERSION(12)
}

auto CheckProto_13(
    const VerificationIdentity& input,
    const bool silent,
    VerificationNymMap&,
    const VerificationType) -> bool
{
    UNDEFINED_VERSION(13)
}

auto CheckProto_14(
    const VerificationIdentity& input,
    const bool silent,
    VerificationNymMap&,
    const VerificationType) -> bool
{
    UNDEFINED_VERSION(14)
}

auto CheckProto_15(
    const VerificationIdentity& input,
    const bool silent,
    VerificationNymMap&,
    const VerificationType) -> bool
{
    UNDEFINED_VERSION(15)
}

auto CheckProto_16(
    const VerificationIdentity& input,
    const bool silent,
    VerificationNymMap&,
    const VerificationType) -> bool
{
    UNDEFINED_VERSION(16)
}

auto CheckProto_17(
    const VerificationIdentity& input,
    const bool silent,
    VerificationNymMap&,
    const VerificationType) -> bool
{
    UNDEFINED_VERSION(17)
}

auto CheckProto_18(
    const VerificationIdentity& input,
    const bool silent,
    VerificationNymMap&,
    const VerificationType) -> bool
{
    UNDEFINED_VERSION(18)
}

auto CheckProto_19(
    const VerificationIdentity& input,
    const bool silent,
    VerificationNymMap&,
    const VerificationType) -> bool
{
    UNDEFINED_VERSION(19)
}

auto CheckProto_20(
    const VerificationIdentity& input,
    const bool silent,
    VerificationNymMap&,
    const VerificationType) -> bool
{
    UNDEFINED_VERSION(20)
}
}  // namespace proto
}  // namespace opentxs
