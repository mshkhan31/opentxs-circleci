// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/protobuf/verify/PeerObject.hpp"  // IWYU pragma: associated

#include <string>
#include <utility>

#include "opentxs/protobuf/Basic.hpp"
#include "opentxs/protobuf/Check.hpp"
#include "opentxs/protobuf/Nym.pb.h"
#include "opentxs/protobuf/PeerEnums.pb.h"
#include "opentxs/protobuf/PeerObject.pb.h"
#include "opentxs/protobuf/PeerReply.pb.h"
#include "opentxs/protobuf/PeerRequest.pb.h"
#include "opentxs/protobuf/verify/Nym.hpp"
#include "opentxs/protobuf/verify/PeerReply.hpp"
#include "opentxs/protobuf/verify/PeerRequest.hpp"
#include "opentxs/protobuf/verify/VerifyPeer.hpp"
#include "protobuf/Check.hpp"

#define PROTO_NAME "peer object"

namespace opentxs::proto
{
auto CheckProto_5(const PeerObject& input, const bool silent) -> bool
{
    if (!input.has_type()) { FAIL_1("missing type") }

    switch (input.type()) {
        case PEEROBJECT_MESSAGE: {
            if (false == input.has_otmessage()) { FAIL_1("missing otmessage") }

            if (input.has_otrequest()) { FAIL_1("otrequest not empty") }

            if (input.has_otreply()) { FAIL_1("otreply not empty") }

            if (input.has_otpayment()) { FAIL_1("otpayment not empty") }
        } break;
        case PEEROBJECT_REQUEST: {
            if (!input.has_otrequest()) { FAIL_1("missing otrequest") }

            const bool validrequest = Check(
                input.otrequest(),
                PeerObjectAllowedPeerRequest().at(input.version()).first,
                PeerObjectAllowedPeerRequest().at(input.version()).second,
                silent);

            if (!validrequest) { FAIL_1("invalid otrequest") }

            if (!input.has_nym()) { FAIL_1(" missing nym") }

            const bool validnym = Check(
                input.nym(),
                PeerObjectAllowedNym().at(input.version()).first,
                PeerObjectAllowedNym().at(input.version()).second,
                silent);

            if (!validnym) { FAIL_1("invalid nym") }

            if (input.has_otmessage()) { FAIL_1("otmessage not empty") }

            if (input.has_otreply()) { FAIL_1("otreply not empty") }

            if (input.has_otpayment()) { FAIL_1("otpayment not empty") }
        } break;
        case PEEROBJECT_RESPONSE: {
            if (!input.has_otrequest()) { FAIL_1("missing otrequest") }

            const bool validrequest = Check(
                input.otrequest(),
                PeerObjectAllowedPeerRequest().at(input.version()).first,
                PeerObjectAllowedPeerRequest().at(input.version()).second,
                silent);

            if (!validrequest) { FAIL_1("invalid otrequest") }

            if (!input.has_otreply()) { FAIL_1("missing otreply") }

            const bool validreply = Check(
                input.otreply(),
                PeerObjectAllowedPeerReply().at(input.version()).first,
                PeerObjectAllowedPeerReply().at(input.version()).second,
                silent);

            if (!validreply) { FAIL_1("invalid otreply") }

            const bool matchingID =
                (input.otrequest().id() == input.otreply().cookie());

            if (!matchingID) {
                FAIL_1("reply cookie does not match request id")
            }

            const bool matchingtype =
                (input.otrequest().type() == input.otreply().type());

            if (!matchingtype) {
                FAIL_1("reply type does not match request type")
            }

            const bool matchingInitiator =
                (input.otrequest().initiator() == input.otreply().initiator());

            if (!matchingInitiator) {
                FAIL_1("reply initiator does not match request initiator")
            }

            const bool matchingRecipient =
                (input.otrequest().recipient() == input.otreply().recipient());

            if (!matchingRecipient) {
                FAIL_1("reply recipient does not match request recipient")
            }

            if (input.has_otmessage()) { FAIL_1("otmessage not empty") }

            if (input.has_otpayment()) { FAIL_1("otpayment not empty") }
        } break;
        case PEEROBJECT_PAYMENT: {
            if (false == input.has_otpayment()) { FAIL_1("missing otpayment") }

            if (input.has_otrequest()) { FAIL_1("otrequest not empty") }

            if (input.has_otreply()) { FAIL_1("otreply not empty") }

            if (input.has_otmessage()) { FAIL_1("otmessage not empty") }
        } break;
        case PEEROBJECT_CASH:
        case PEEROBJECT_ERROR:
        default: {
            FAIL_1("invalid type")
        }
    }

    CHECK_EXCLUDED(purse);

    return true;
}

auto CheckProto_6(const PeerObject& input, const bool silent) -> bool
{
    return CheckProto_5(input, silent);
}
}  // namespace opentxs::proto
