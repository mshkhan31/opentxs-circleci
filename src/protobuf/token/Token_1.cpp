// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/protobuf/verify/Token.hpp"  // IWYU pragma: associated

#include <cstdint>

#include "opentxs/protobuf/Basic.hpp"
#include "opentxs/protobuf/CashEnums.pb.h"
#include "opentxs/protobuf/Token.pb.h"
#include "opentxs/protobuf/verify/LucreTokenData.hpp"  // IWYU pragma: keep
#include "opentxs/protobuf/verify/VerifyCash.hpp"
#include "protobuf/Check.hpp"

#define PROTO_NAME "token"

#include <algorithm>
#include <set>
#include <string>

namespace opentxs
{
namespace proto
{
auto CheckProto_1(
    const Token& input,
    const bool silent,
    const CashType type,
    const std::set<TokenState>& state,
    std::int64_t& totalValue,
    std::int64_t& validFrom,
    std::int64_t& validTo) -> bool
{
    if (type != input.type()) {
        FAIL_4(
            "Incorrect type",
            std::to_string(input.state()),
            " expected ",
            std::to_string(type));
    }

    const bool haveExpectedState = false == state.empty();

    if (haveExpectedState && (0 == state.count(input.state()))) {
        FAIL_2("Incorrect state", std::to_string(input.state()));
    }

    switch (input.state()) {
        case TOKENSTATE_BLINDED:
        case TOKENSTATE_SIGNED:
        case TOKENSTATE_READY: {
            totalValue += input.denomination();
            validFrom = std::max(input.validfrom(), validFrom);
            validTo = std::min(input.validto(), validTo);
        } break;
        case TOKENSTATE_SPENT:
        case TOKENSTATE_EXPIRED: {
        } break;
        case TOKENSTATE_ERROR:
        default: {
            FAIL_2("Invalid state", std::to_string(input.state()));
        }
    }

    CHECK_IDENTIFIER(notary);
    CHECK_IDENTIFIER(mint);

    switch (input.type()) {
        case CASHTYPE_LUCRE: {
            CHECK_SUBOBJECT_VA(
                lucre, TokenAllowedLucreTokenData(), input.state());
        } break;
        case CASHTYPE_ERROR:
        default: {
            FAIL_2("Invalid type", std::to_string(input.type()));
        }
    }

    return true;
}
auto CheckProto_2(
    const Token& input,
    const bool silent,
    const CashType,
    const std::set<TokenState>&,
    std::int64_t&,
    std::int64_t&,
    std::int64_t&) -> bool
{
    UNDEFINED_VERSION(2)
}

auto CheckProto_3(
    const Token& input,
    const bool silent,
    const CashType,
    const std::set<TokenState>&,
    std::int64_t&,
    std::int64_t&,
    std::int64_t&) -> bool
{
    UNDEFINED_VERSION(3)
}

auto CheckProto_4(
    const Token& input,
    const bool silent,
    const CashType,
    const std::set<TokenState>&,
    std::int64_t&,
    std::int64_t&,
    std::int64_t&) -> bool
{
    UNDEFINED_VERSION(4)
}

auto CheckProto_5(
    const Token& input,
    const bool silent,
    const CashType,
    const std::set<TokenState>&,
    std::int64_t&,
    std::int64_t&,
    std::int64_t&) -> bool
{
    UNDEFINED_VERSION(5)
}

auto CheckProto_6(
    const Token& input,
    const bool silent,
    const CashType,
    const std::set<TokenState>&,
    std::int64_t&,
    std::int64_t&,
    std::int64_t&) -> bool
{
    UNDEFINED_VERSION(6)
}

auto CheckProto_7(
    const Token& input,
    const bool silent,
    const CashType,
    const std::set<TokenState>&,
    std::int64_t&,
    std::int64_t&,
    std::int64_t&) -> bool
{
    UNDEFINED_VERSION(7)
}

auto CheckProto_8(
    const Token& input,
    const bool silent,
    const CashType,
    const std::set<TokenState>&,
    std::int64_t&,
    std::int64_t&,
    std::int64_t&) -> bool
{
    UNDEFINED_VERSION(8)
}

auto CheckProto_9(
    const Token& input,
    const bool silent,
    const CashType,
    const std::set<TokenState>&,
    std::int64_t&,
    std::int64_t&,
    std::int64_t&) -> bool
{
    UNDEFINED_VERSION(9)
}

auto CheckProto_10(
    const Token& input,
    const bool silent,
    const CashType,
    const std::set<TokenState>&,
    std::int64_t&,
    std::int64_t&,
    std::int64_t&) -> bool
{
    UNDEFINED_VERSION(10)
}

auto CheckProto_11(
    const Token& input,
    const bool silent,
    const CashType,
    const std::set<TokenState>&,
    std::int64_t&,
    std::int64_t&,
    std::int64_t&) -> bool
{
    UNDEFINED_VERSION(11)
}

auto CheckProto_12(
    const Token& input,
    const bool silent,
    const CashType,
    const std::set<TokenState>&,
    std::int64_t&,
    std::int64_t&,
    std::int64_t&) -> bool
{
    UNDEFINED_VERSION(12)
}

auto CheckProto_13(
    const Token& input,
    const bool silent,
    const CashType,
    const std::set<TokenState>&,
    std::int64_t&,
    std::int64_t&,
    std::int64_t&) -> bool
{
    UNDEFINED_VERSION(13)
}

auto CheckProto_14(
    const Token& input,
    const bool silent,
    const CashType,
    const std::set<TokenState>&,
    std::int64_t&,
    std::int64_t&,
    std::int64_t&) -> bool
{
    UNDEFINED_VERSION(14)
}

auto CheckProto_15(
    const Token& input,
    const bool silent,
    const CashType,
    const std::set<TokenState>&,
    std::int64_t&,
    std::int64_t&,
    std::int64_t&) -> bool
{
    UNDEFINED_VERSION(15)
}

auto CheckProto_16(
    const Token& input,
    const bool silent,
    const CashType,
    const std::set<TokenState>&,
    std::int64_t&,
    std::int64_t&,
    std::int64_t&) -> bool
{
    UNDEFINED_VERSION(16)
}

auto CheckProto_17(
    const Token& input,
    const bool silent,
    const CashType,
    const std::set<TokenState>&,
    std::int64_t&,
    std::int64_t&,
    std::int64_t&) -> bool
{
    UNDEFINED_VERSION(17)
}

auto CheckProto_18(
    const Token& input,
    const bool silent,
    const CashType,
    const std::set<TokenState>&,
    std::int64_t&,
    std::int64_t&,
    std::int64_t&) -> bool
{
    UNDEFINED_VERSION(18)
}

auto CheckProto_19(
    const Token& input,
    const bool silent,
    const CashType,
    const std::set<TokenState>&,
    std::int64_t&,
    std::int64_t&,
    std::int64_t&) -> bool
{
    UNDEFINED_VERSION(19)
}

auto CheckProto_20(
    const Token& input,
    const bool silent,
    const CashType,
    const std::set<TokenState>&,
    std::int64_t&,
    std::int64_t&,
    std::int64_t&) -> bool
{
    UNDEFINED_VERSION(20)
}
}  // namespace proto
}  // namespace opentxs
