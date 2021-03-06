// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_PROTOBUF_VERIFICATION_HPP
#define OPENTXS_PROTOBUF_VERIFICATION_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include "opentxs/protobuf/verify/VerifyContacts.hpp"

namespace opentxs
{
namespace proto
{
class Verification;
}  // namespace proto
}  // namespace opentxs

namespace opentxs
{
namespace proto
{
auto CheckProto_1(
    const Verification& verification,
    const bool silent,
    const VerificationType indexed) -> bool;
auto CheckProto_2(const Verification&, const bool, const VerificationType)
    -> bool;
auto CheckProto_3(const Verification&, const bool, const VerificationType)
    -> bool;
auto CheckProto_4(const Verification&, const bool, const VerificationType)
    -> bool;
auto CheckProto_5(const Verification&, const bool, const VerificationType)
    -> bool;
auto CheckProto_6(const Verification&, const bool, const VerificationType)
    -> bool;
auto CheckProto_7(const Verification&, const bool, const VerificationType)
    -> bool;
auto CheckProto_8(const Verification&, const bool, const VerificationType)
    -> bool;
auto CheckProto_9(const Verification&, const bool, const VerificationType)
    -> bool;
auto CheckProto_10(const Verification&, const bool, const VerificationType)
    -> bool;
auto CheckProto_11(const Verification&, const bool, const VerificationType)
    -> bool;
auto CheckProto_12(const Verification&, const bool, const VerificationType)
    -> bool;
auto CheckProto_13(const Verification&, const bool, const VerificationType)
    -> bool;
auto CheckProto_14(const Verification&, const bool, const VerificationType)
    -> bool;
auto CheckProto_15(const Verification&, const bool, const VerificationType)
    -> bool;
auto CheckProto_16(const Verification&, const bool, const VerificationType)
    -> bool;
auto CheckProto_17(const Verification&, const bool, const VerificationType)
    -> bool;
auto CheckProto_18(const Verification&, const bool, const VerificationType)
    -> bool;
auto CheckProto_19(const Verification&, const bool, const VerificationType)
    -> bool;
auto CheckProto_20(const Verification&, const bool, const VerificationType)
    -> bool;
}  // namespace proto
}  // namespace opentxs

#endif  // OPENTXS_PROTOBUF_VERIFICATION_HPP
