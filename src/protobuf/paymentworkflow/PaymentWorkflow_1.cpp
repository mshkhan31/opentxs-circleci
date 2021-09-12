// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/protobuf/verify/PaymentWorkflow.hpp"  // IWYU pragma: associated

#include <cstdint>
#include <iosfwd>
#include <map>
#include <set>
#include <stdexcept>
#include <string>
#include <utility>

#include "Proto.hpp"
#include "opentxs/protobuf/Basic.hpp"
#include "opentxs/protobuf/Check.hpp"
#include "opentxs/protobuf/PaymentEvent.pb.h"
#include "opentxs/protobuf/PaymentWorkflow.pb.h"
#include "opentxs/protobuf/PaymentWorkflowEnums.pb.h"
#include "opentxs/protobuf/verify/InstrumentRevision.hpp"  // IWYU pragma: keep
#include "opentxs/protobuf/verify/PaymentEvent.hpp"
#include "opentxs/protobuf/verify/VerifyWorkflows.hpp"
#include "protobuf/Check.hpp"

#define BAD_EVENTS(a, b)                                                       \
    {                                                                          \
        FAIL_6(                                                                \
            a,                                                                 \
            b,                                                                 \
            " Type: ",                                                         \
            std::to_string(static_cast<std::uint32_t>(input.type())),          \
            " State: ",                                                        \
            std::to_string(static_cast<std::uint32_t>(input.state())))         \
    }

#define PROTO_NAME "payment workflow"

namespace opentxs::proto
{
auto CheckProto_1(const PaymentWorkflow& input, const bool silent) -> bool
{
    CHECK_IDENTIFIER(id)

    try {
        const bool valid =
            (1 == PaymentWorkflowAllowedState()
                      .at({input.version(), input.type()})
                      .count(input.state()));

        if (false == valid) { FAIL_2("Invalid state", __LINE__) }
    } catch (const std::out_of_range&) {
        FAIL_2("Invalid type", __LINE__)
    }

    switch (input.type()) {
        case PAYMENTWORKFLOWTYPE_OUTGOINGCHEQUE:
        case PAYMENTWORKFLOWTYPE_INCOMINGCHEQUE:
        case PAYMENTWORKFLOWTYPE_OUTGOINGINVOICE:
        case PAYMENTWORKFLOWTYPE_INCOMINGINVOICE: {
            if (1 != input.source().size()) {
                FAIL_2(
                    "Incorrect number of source objects", input.source().size())
            }

            if (1 != input.party().size()) {
                FAIL_2("Incorrect number of parties", input.party().size())
            }
        } break;
        case PAYMENTWORKFLOWTYPE_OUTGOINGTRANSFER:
        case PAYMENTWORKFLOWTYPE_INCOMINGTRANSFER:
        case PAYMENTWORKFLOWTYPE_INTERNALTRANSFER:
        case PAYMENTWORKFLOWTYPE_INCOMINGCASH:
        case PAYMENTWORKFLOWTYPE_OUTGOINGCASH:
        case PAYMENTWORKFLOWTYPE_ERROR:
        default: {
            FAIL_2("Invalid type", __LINE__)
        }
    }

    CHECK_SUBOBJECTS(source, PaymentWorkflowAllowedInstrumentRevision())
    CHECK_IDENTIFIERS(party)
    std::map<PaymentEventType, std::size_t> events{};

    for (const auto& it : input.event()) {
        try {
            const bool validevent = Check(
                it,
                PaymentWorkflowAllowedPaymentEvent().at(input.version()).first,
                PaymentWorkflowAllowedPaymentEvent().at(input.version()).second,
                silent,
                input.version(),
                input.type(),
                events);

            if (false == validevent) {
                const auto fail = std::string("invalid ") + "event";
                FAIL_1(fail)
            }
        } catch (const std::out_of_range&) {
            const auto fail = std::string("allowed ") + "event" +
                              " version not defined for version";
            FAIL_2(fail.c_str(), input.version())
        }
    }

    CHECK_IDENTIFIERS(unit)
    CHECK_IDENTIFIERS(account)

    const auto& createEvents = events[PAYMENTEVENTTYPE_CREATE];
    const auto& conveyEvents = events[PAYMENTEVENTTYPE_CONVEY];
    const auto& cancelEvents = events[PAYMENTEVENTTYPE_CANCEL];
    const auto& acceptEvents = events[PAYMENTEVENTTYPE_ACCEPT];
    const auto& completeEvents = events[PAYMENTEVENTTYPE_COMPLETE];
    const auto accounts = input.account().size();

    switch (input.type()) {
        case PAYMENTWORKFLOWTYPE_OUTGOINGCHEQUE:
        case PAYMENTWORKFLOWTYPE_OUTGOINGINVOICE: {
            CHECK_IDENTIFIER(notary);

            if (1 != accounts) { FAIL_2("Wrong number of accounts", accounts) }

            switch (input.state()) {
                case PAYMENTWORKFLOWSTATE_UNSENT: {
                    if (1 != createEvents) {
                        BAD_EVENTS(
                            "Wrong number of create events", createEvents)
                    }

                    // TODO: convey events are allowed only if they are
                    //       all failed

                    if (0 < cancelEvents) {
                        BAD_EVENTS(
                            "Wrong number of cancel events", cancelEvents)
                    }

                    if (0 < acceptEvents) {
                        BAD_EVENTS(
                            "Wrong number of accept events", acceptEvents)
                    }

                    if (0 < completeEvents) {
                        BAD_EVENTS(
                            "Wrong number of complete events", completeEvents)
                    }
                } break;
                case PAYMENTWORKFLOWSTATE_CONVEYED: {
                    if (1 != createEvents) {
                        BAD_EVENTS(
                            "Wrong number of create events", createEvents)
                    }

                    if (0 == conveyEvents) {
                        BAD_EVENTS(
                            "Wrong number of convey events", conveyEvents)
                    }

                    // TODO: cancel events are allowed only if they are
                    //       all failed

                    // TODO: accept events are allowed only if they are
                    //       all failed

                    if (0 < completeEvents) {
                        BAD_EVENTS(
                            "Wrong number of complete events", completeEvents)
                    }
                } break;
                case PAYMENTWORKFLOWSTATE_CANCELLED: {
                    if (1 != createEvents) {
                        BAD_EVENTS(
                            "Wrong number of create events", createEvents)
                    }

                    // Any number of convey events are allowed

                    if (0 == cancelEvents) {
                        BAD_EVENTS(
                            "Wrong number of cancel events", cancelEvents)
                    }

                    if (0 < acceptEvents) {
                        BAD_EVENTS(
                            "Wrong number of accept events", acceptEvents)
                    }

                    if (0 < completeEvents) {
                        BAD_EVENTS(
                            "Wrong number of complete events", completeEvents)
                    }
                } break;
                case PAYMENTWORKFLOWSTATE_ACCEPTED: {
                    if (1 != createEvents) {
                        BAD_EVENTS(
                            "Wrong number of create events", createEvents)
                    }

                    if (0 == conveyEvents) {
                        BAD_EVENTS(
                            "Wrong number of convey events", conveyEvents)
                    }

                    // TODO: cancel events are allowed only if they are
                    //       all failed

                    if (0 == acceptEvents) {
                        BAD_EVENTS(
                            "Wrong number of accept events", acceptEvents)
                    }

                    // TODO: complete events are allowed only if they are
                    //       all failed
                } break;
                case PAYMENTWORKFLOWSTATE_COMPLETED: {
                    if (1 != createEvents) {
                        BAD_EVENTS(
                            "Wrong number of create events", createEvents)
                    }

                    if (0 == conveyEvents) {
                        BAD_EVENTS(
                            "Wrong number of convey events", conveyEvents)
                    }

                    // TODO: cancel events are allowed only if they are
                    //       all failed

                    if (0 == acceptEvents) {
                        BAD_EVENTS(
                            "Wrong number of accept events", acceptEvents)
                    }

                    if (0 == completeEvents) {
                        BAD_EVENTS(
                            "Wrong number of complete events", completeEvents)
                    }
                } break;
                case PAYMENTWORKFLOWSTATE_EXPIRED: {
                    if (1 != createEvents) {
                        BAD_EVENTS(
                            "Wrong number of create events", createEvents)
                    }

                    // Any number of convey events are allowed

                    // TODO: cancel events are allowed only if they are
                    //       all failed

                    if (0 < acceptEvents) {
                        BAD_EVENTS(
                            "Wrong number of accept events", acceptEvents)
                    }

                    if (0 < completeEvents) {
                        BAD_EVENTS(
                            "Wrong number of complete events", completeEvents)
                    }
                } break;
                case PAYMENTWORKFLOWSTATE_INITIATED:
                case PAYMENTWORKFLOWSTATE_ABORTED:
                case PAYMENTWORKFLOWSTATE_ACKNOWLEDGED:
                case PAYMENTWORKFLOWSTATE_REJECTED:
                case PAYMENTWORKFLOWSTATE_ERROR:
                default: {
                    FAIL_2("Invalid state", __LINE__)
                }
            }
        } break;
        case PAYMENTWORKFLOWTYPE_INCOMINGCHEQUE:
        case PAYMENTWORKFLOWTYPE_INCOMINGINVOICE: {
            OPTIONAL_IDENTIFIER(notary);

            switch (input.state()) {
                case PAYMENTWORKFLOWSTATE_CONVEYED: {
                    if (0 != accounts) {
                        BAD_EVENTS("Wrong number of accounts", accounts)
                    }

                    if (0 < createEvents) {
                        BAD_EVENTS(
                            "Wrong number of create events", createEvents)
                    }

                    if (1 != conveyEvents) {
                        BAD_EVENTS(
                            "Wrong number of convey events", conveyEvents)
                    }

                    if (0 < cancelEvents) {
                        BAD_EVENTS(
                            "Wrong number of cancel events", cancelEvents)
                    }

                    // TODO: accept events are allowed only if they are
                    //       all failed

                    if (0 < completeEvents) {
                        BAD_EVENTS(
                            "Wrong number of complete events", completeEvents)
                    }
                } break;
                case PAYMENTWORKFLOWSTATE_COMPLETED: {
                    CHECK_IDENTIFIER(notary);

                    if (1 != accounts) {
                        BAD_EVENTS("Wrong number of accounts", accounts)
                    }

                    if (0 < createEvents) {
                        BAD_EVENTS(
                            "Wrong number of create events", createEvents)
                    }

                    if (1 != conveyEvents) {
                        BAD_EVENTS(
                            "Wrong number of convey events", conveyEvents)
                    }

                    if (0 < cancelEvents) {
                        BAD_EVENTS(
                            "Wrong number of cancel events", cancelEvents)
                    }

                    if (0 == acceptEvents) {
                        BAD_EVENTS(
                            "Wrong number of accept events", acceptEvents)
                    }

                    if (0 < completeEvents) {
                        BAD_EVENTS(
                            "Wrong number of complete events", completeEvents)
                    }
                } break;
                case PAYMENTWORKFLOWSTATE_EXPIRED: {
                    if (1 < accounts) {
                        BAD_EVENTS("Wrong number of accounts", accounts)
                    }

                    if (0 < createEvents) {
                        BAD_EVENTS(
                            "Wrong number of create events", createEvents)
                    }

                    if (1 != conveyEvents) {
                        BAD_EVENTS(
                            "Wrong number of convey events", conveyEvents)
                    }

                    if (0 < cancelEvents) {
                        BAD_EVENTS(
                            "Wrong number of cancel events", cancelEvents)
                    }

                    // TODO: accept events are allowed only if they are
                    //       all failed

                    if (0 < completeEvents) {
                        BAD_EVENTS(
                            "Wrong number of complete events", completeEvents)
                    }
                } break;
                case PAYMENTWORKFLOWSTATE_INITIATED:
                case PAYMENTWORKFLOWSTATE_ABORTED:
                case PAYMENTWORKFLOWSTATE_ACKNOWLEDGED:
                case PAYMENTWORKFLOWSTATE_UNSENT:
                case PAYMENTWORKFLOWSTATE_CANCELLED:
                case PAYMENTWORKFLOWSTATE_ACCEPTED:
                case PAYMENTWORKFLOWSTATE_REJECTED:
                case PAYMENTWORKFLOWSTATE_ERROR:
                default: {
                    FAIL_2("Invalid state", __LINE__)
                }
            }
        } break;
        case PAYMENTWORKFLOWTYPE_OUTGOINGTRANSFER:
        case PAYMENTWORKFLOWTYPE_INCOMINGTRANSFER:
        case PAYMENTWORKFLOWTYPE_INTERNALTRANSFER:
        case PAYMENTWORKFLOWTYPE_INCOMINGCASH:
        case PAYMENTWORKFLOWTYPE_OUTGOINGCASH:
        case PAYMENTWORKFLOWTYPE_ERROR:
        default: {
            FAIL_2("Invalid type", __LINE__)
        }
    }

    switch (input.type()) {
        case PAYMENTWORKFLOWTYPE_OUTGOINGCHEQUE:
        case PAYMENTWORKFLOWTYPE_INCOMINGCHEQUE:
        case PAYMENTWORKFLOWTYPE_OUTGOINGINVOICE:
        case PAYMENTWORKFLOWTYPE_INCOMINGINVOICE: {
            if (1 != input.unit().size()) { FAIL_1("Missing unit") }
        } break;
        case PAYMENTWORKFLOWTYPE_OUTGOINGTRANSFER:
        case PAYMENTWORKFLOWTYPE_INCOMINGTRANSFER:
        case PAYMENTWORKFLOWTYPE_INTERNALTRANSFER:
        case PAYMENTWORKFLOWTYPE_INCOMINGCASH:
        case PAYMENTWORKFLOWTYPE_OUTGOINGCASH:
        case PAYMENTWORKFLOWTYPE_ERROR:
        default: {
            FAIL_2("Invalid type", __LINE__)
        }
    }

    return true;
}
}  // namespace opentxs::proto
