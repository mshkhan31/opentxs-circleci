// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/protobuf/verify/UnitDefinition.hpp"  // IWYU pragma: associated

#include <stdexcept>
#include <string>
#include <utility>

#include "opentxs/protobuf/Basic.hpp"
#include "opentxs/protobuf/BasketParams.pb.h"
#include "opentxs/protobuf/Check.hpp"
#include "opentxs/protobuf/ContractEnums.pb.h"
#include "opentxs/protobuf/CurrencyParams.pb.h"
#include "opentxs/protobuf/Enums.pb.h"
#include "opentxs/protobuf/EquityParams.pb.h"
#include "opentxs/protobuf/Nym.pb.h"
#include "opentxs/protobuf/Signature.pb.h"
#include "opentxs/protobuf/UnitDefinition.pb.h"
#include "opentxs/protobuf/verify/BasketParams.hpp"
#include "opentxs/protobuf/verify/CurrencyParams.hpp"
#include "opentxs/protobuf/verify/EquityParams.hpp"
#include "opentxs/protobuf/verify/Nym.hpp"
#include "opentxs/protobuf/verify/Signature.hpp"
#include "opentxs/protobuf/verify/VerifyContracts.hpp"
#include "protobuf/Check.hpp"

#define PROTO_NAME "unit definition"

namespace opentxs
{
namespace proto
{

auto CheckProto_1(
    const UnitDefinition& input,
    const bool silent,
    const bool checkSig) -> bool
{
    if (!input.has_id()) { FAIL_1("missing id") }

    if (MIN_PLAUSIBLE_IDENTIFIER > input.id().size()) {
        FAIL_2("invalid id", input.id())
    }

    if (!input.has_nymid()) { FAIL_1("missing nym id") }

    if (MIN_PLAUSIBLE_IDENTIFIER > input.nymid().size()) {
        FAIL_2("invalid nym id", input.nymid())
    }

    if (input.has_publicnym()) {
        try {
            const bool goodPublicNym = Check(
                input.publicnym(),
                UnitDefinitionAllowedNym().at(input.version()).first,
                UnitDefinitionAllowedNym().at(input.version()).second,
                silent);

            if (!goodPublicNym) { FAIL_1("invalid nym") }
        } catch (const std::out_of_range&) {
            FAIL_2(
                "allowed credential index version not defined for version",
                input.version())
        }

        if (input.nymid() != input.publicnym().nymid()) { FAIL_1("wrong nym") }
    }

    if (!input.has_shortname()) { FAIL_1("missing shortname") }

    if (1 > input.shortname().size()) {
        FAIL_2("invalid shortname", input.shortname())
    }

    if (!input.has_terms()) { FAIL_1("missing terms") }

    if (1 > input.terms().size()) { FAIL_2("invalid terms", input.terms()) }

    if (!input.has_name()) { FAIL_1("missing name") }

    if (1 > input.name().size()) { FAIL_2("invalid name", input.name()) }

    if (!input.has_symbol()) { FAIL_1("missing symbol") }

    if (1 > input.symbol().size()) { FAIL_2("invalid symbol", input.symbol()) }

    if (!input.has_type()) { FAIL_1("missing type") }

    bool goodParams = false;

    switch (input.type()) {
        case UNITTYPE_CURRENCY: {
            if (!input.has_currency()) { FAIL_1("missing currency params") }

            try {
                goodParams = Check(
                    input.currency(),
                    UnitDefinitionAllowedCurrencyParams()
                        .at(input.version())
                        .first,
                    UnitDefinitionAllowedCurrencyParams()
                        .at(input.version())
                        .second,
                    silent);
            } catch (const std::out_of_range&) {
                FAIL_2(
                    "allowed currency params version not defined for version",
                    input.version())
            }

            if (!goodParams) { FAIL_1("invalid currency params") }
        } break;
        case UNITTYPE_SECURITY: {
            if (!input.has_security()) { FAIL_1("missing security params") }

            try {
                goodParams = Check(
                    input.security(),
                    UnitDefinitionAllowedSecurityParams()
                        .at(input.version())
                        .first,
                    UnitDefinitionAllowedSecurityParams()
                        .at(input.version())
                        .second,
                    silent);
            } catch (const std::out_of_range&) {
                FAIL_2(
                    "allowed security params version not defined for version",
                    input.version())
            }

            if (!goodParams) { FAIL_1("invalid security params") }
        } break;
        case UNITTYPE_BASKET: {
            if (!input.has_basket()) { FAIL_1("missing currency params") }

            try {
                goodParams = Check(
                    input.basket(),
                    UnitDefinitionAllowedBasketParams()
                        .at(input.version())
                        .first,
                    UnitDefinitionAllowedBasketParams()
                        .at(input.version())
                        .second,
                    silent);
            } catch (const std::out_of_range&) {
                FAIL_2(
                    "allowed basket params version not defined for version",
                    input.version())
            }

            if (!goodParams) { FAIL_1("invalid basket params") }
        } break;
        case UNITTYPE_ERROR:
        default: {
            FAIL_1("invalid type")
        }
    }

    CHECK_EXCLUDED(unitofaccount)

    if (checkSig) {
        try {
            const bool valid = Check(
                input.signature(),
                UnitDefinitionAllowedSignature().at(input.version()).first,
                UnitDefinitionAllowedSignature().at(input.version()).second,
                silent,
                SIGROLE_UNITDEFINITION);

            if (false == valid) { FAIL_1("invalid signature") }
        } catch (const std::out_of_range&) {
            FAIL_2(
                "allowed signature version not defined for version",
                input.version())
        }
    }

    return true;
}
}  // namespace proto
}  // namespace opentxs
