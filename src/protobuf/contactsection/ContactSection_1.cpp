// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/protobuf/verify/ContactSection.hpp"  // IWYU pragma: associated

#include <cstdint>
#include <stdexcept>
#include <utility>

#include "Proto.hpp"
#include "opentxs/protobuf/Basic.hpp"
#include "opentxs/protobuf/Check.hpp"
#include "opentxs/protobuf/Contact.hpp"
#include "opentxs/protobuf/ContactItem.pb.h"
#include "opentxs/protobuf/ContactSection.pb.h"
#include "opentxs/protobuf/verify/ContactItem.hpp"
#include "opentxs/protobuf/verify/VerifyContacts.hpp"
#include "protobuf/Check.hpp"

#define PROTO_NAME "contact section"

namespace opentxs
{
namespace proto
{

auto CheckProto_1(
    const ContactSection& input,
    const bool silent,
    const ClaimType indexed,
    const uint32_t parentVersion) -> bool
{
    if (!input.has_name()) { FAIL_1("missing name") }

    if (!ValidContactSectionName(parentVersion, input.name())) {
        FAIL_2("invalid name", input.name())
    }

    for (auto& it : input.item()) {
        try {
            bool validItem = Check(
                it,
                ContactSectionAllowedItem().at(input.version()).first,
                ContactSectionAllowedItem().at(input.version()).second,
                silent,
                indexed,
                ContactSectionVersion{input.version(), input.name()});

            if (!validItem) { FAIL_1("invalid item") }
        } catch (const std::out_of_range&) {
            FAIL_2(
                "allowed contact item version not defined for version",
                input.version())
        }
    }

    return true;
}

auto CheckProto_2(
    const ContactSection& input,
    const bool silent,
    const ClaimType indexed,
    const uint32_t parentVersion) -> bool
{
    return CheckProto_1(input, silent, indexed, parentVersion);
}

auto CheckProto_3(
    const ContactSection& input,
    const bool silent,
    const ClaimType indexed,
    const uint32_t parentVersion) -> bool
{
    return CheckProto_1(input, silent, indexed, parentVersion);
}

auto CheckProto_4(
    const ContactSection& input,
    const bool silent,
    const ClaimType indexed,
    const uint32_t parentVersion) -> bool
{
    return CheckProto_1(input, silent, indexed, parentVersion);
}

auto CheckProto_5(
    const ContactSection& input,
    const bool silent,
    const ClaimType indexed,
    const uint32_t parentVersion) -> bool
{
    return CheckProto_1(input, silent, indexed, parentVersion);
}

auto CheckProto_6(
    const ContactSection& input,
    const bool silent,
    const ClaimType indexed,
    const uint32_t parentVersion) -> bool
{
    if (!input.has_name()) { FAIL_1("missing name") }

    if (!ValidContactSectionName(parentVersion, input.name())) {
        FAIL_2("invalid name", input.name())
    }

    if (0 == input.item_size()) { FAIL_1("empty section") }

    for (auto& it : input.item()) {
        try {
            bool validItem = Check(
                it,
                ContactSectionAllowedItem().at(input.version()).first,
                ContactSectionAllowedItem().at(input.version()).second,
                silent,
                indexed,
                ContactSectionVersion{input.version(), input.name()});

            if (!validItem) { FAIL_1("invalid item") }
        } catch (const std::out_of_range&) {
            FAIL_2(
                "allowed contact item version not defined for version",
                input.version())
        }
    }

    return true;
}

auto CheckProto_7(
    const ContactSection& input,
    const bool silent,
    const ClaimType indexed,
    const uint32_t parentVersion) -> bool
{
    UNDEFINED_VERSION(7)
}

auto CheckProto_8(
    const ContactSection& input,
    const bool silent,
    const ClaimType indexed,
    const uint32_t parentVersion) -> bool
{
    UNDEFINED_VERSION(8)
}

auto CheckProto_9(
    const ContactSection& input,
    const bool silent,
    const ClaimType indexed,
    const uint32_t parentVersion) -> bool
{
    UNDEFINED_VERSION(9)
}

auto CheckProto_10(
    const ContactSection& input,
    const bool silent,
    const ClaimType indexed,
    const uint32_t parentVersion) -> bool
{
    UNDEFINED_VERSION(10)
}

auto CheckProto_11(
    const ContactSection& input,
    const bool silent,
    const ClaimType indexed,
    const uint32_t parentVersion) -> bool
{
    UNDEFINED_VERSION(11)
}

auto CheckProto_12(
    const ContactSection& input,
    const bool silent,
    const ClaimType indexed,
    const uint32_t parentVersion) -> bool
{
    UNDEFINED_VERSION(12)
}

auto CheckProto_13(
    const ContactSection& input,
    const bool silent,
    const ClaimType indexed,
    const uint32_t parentVersion) -> bool
{
    UNDEFINED_VERSION(13)
}

auto CheckProto_14(
    const ContactSection& input,
    const bool silent,
    const ClaimType indexed,
    const uint32_t parentVersion) -> bool
{
    UNDEFINED_VERSION(14)
}

auto CheckProto_15(
    const ContactSection& input,
    const bool silent,
    const ClaimType indexed,
    const uint32_t parentVersion) -> bool
{
    UNDEFINED_VERSION(15)
}

auto CheckProto_16(
    const ContactSection& input,
    const bool silent,
    const ClaimType indexed,
    const uint32_t parentVersion) -> bool
{
    UNDEFINED_VERSION(16)
}

auto CheckProto_17(
    const ContactSection& input,
    const bool silent,
    const ClaimType indexed,
    const uint32_t parentVersion) -> bool
{
    UNDEFINED_VERSION(17)
}

auto CheckProto_18(
    const ContactSection& input,
    const bool silent,
    const ClaimType indexed,
    const uint32_t parentVersion) -> bool
{
    UNDEFINED_VERSION(18)
}

auto CheckProto_19(
    const ContactSection& input,
    const bool silent,
    const ClaimType indexed,
    const uint32_t parentVersion) -> bool
{
    UNDEFINED_VERSION(19)
}

auto CheckProto_20(
    const ContactSection& input,
    const bool silent,
    const ClaimType indexed,
    const uint32_t parentVersion) -> bool
{
    UNDEFINED_VERSION(20)
}
}  // namespace proto
}  // namespace opentxs
