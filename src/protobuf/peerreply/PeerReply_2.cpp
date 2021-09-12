// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/protobuf/verify/PeerReply.hpp"  // IWYU pragma: associated

#include <stdexcept>
#include <string>
#include <utility>

#include "opentxs/protobuf/BailmentReply.pb.h"
#include "opentxs/protobuf/Basic.hpp"
#include "opentxs/protobuf/Check.hpp"
#include "opentxs/protobuf/ConnectionInfoReply.pb.h"
#include "opentxs/protobuf/Enums.pb.h"
#include "opentxs/protobuf/NoticeAcknowledgement.pb.h"
#include "opentxs/protobuf/OutBailmentReply.pb.h"
#include "opentxs/protobuf/PeerEnums.pb.h"
#include "opentxs/protobuf/PeerReply.pb.h"
#include "opentxs/protobuf/Signature.pb.h"
#include "opentxs/protobuf/verify/BailmentReply.hpp"
#include "opentxs/protobuf/verify/ConnectionInfoReply.hpp"
#include "opentxs/protobuf/verify/NoticeAcknowledgement.hpp"
#include "opentxs/protobuf/verify/OutBailmentReply.hpp"
#include "opentxs/protobuf/verify/Signature.hpp"
#include "opentxs/protobuf/verify/VerifyPeer.hpp"
#include "protobuf/Check.hpp"

#define PROTO_NAME "peer reply"

namespace opentxs
{
namespace proto
{
auto CheckProto_2(const PeerReply& input, const bool silent) -> bool
{
    if (!input.has_id()) { FAIL_1("missing id") }

    if (MIN_PLAUSIBLE_IDENTIFIER > input.id().size()) { FAIL_1("invalid id") }

    if (!input.has_initiator()) { FAIL_1("missing initiator") }

    if (MIN_PLAUSIBLE_IDENTIFIER > input.initiator().size()) {
        FAIL_2("invalid initiator", input.initiator())
    }

    if (!input.has_recipient()) { FAIL_1("missing recipient") }

    if (MIN_PLAUSIBLE_IDENTIFIER > input.recipient().size()) {
        FAIL_2("invalid recipient", input.recipient())
    }

    if (!input.has_type()) { FAIL_1("missing type") }

    if (!input.has_cookie()) { FAIL_1("missing cookie") }

    try {
        const bool validSig = Check(
            input.signature(),
            PeerReplyAllowedSignature().at(input.version()).first,
            PeerReplyAllowedSignature().at(input.version()).second,
            silent,
            SIGROLE_PEERREPLY);

        if (!validSig) { FAIL_1("invalid signature") }
    } catch (const std::out_of_range&) {
        FAIL_2(
            "allowed signature version not defined for version",
            input.version())
    }

    if (!input.has_server()) { FAIL_1("missing server") }

    if (MIN_PLAUSIBLE_IDENTIFIER > input.server().size()) {
        FAIL_2("invalid server", input.server())
    }

    switch (input.type()) {
        case PEERREQUEST_BAILMENT: {
            if (!input.has_bailment()) { FAIL_1("missing bailment") }

            try {
                const bool validbailment = Check(
                    input.bailment(),
                    PeerReplyAllowedBailment().at(input.version()).first,
                    PeerReplyAllowedBailment().at(input.version()).second,
                    silent);

                if (!validbailment) { FAIL_1("invalid bailment") }
            } catch (const std::out_of_range&) {
                FAIL_2(
                    "allowed bailment version not defined for version",
                    input.version())
            }
        } break;
        case PEERREQUEST_OUTBAILMENT: {
            if (!input.has_outbailment()) { FAIL_1("missing outbailment") }

            try {
                const bool validoutbailment = Check(
                    input.outbailment(),
                    PeerReplyAllowedOutBailment().at(input.version()).first,
                    PeerReplyAllowedOutBailment().at(input.version()).second,
                    silent);

                if (!validoutbailment) { FAIL_1("invalid outbailment") }
            } catch (const std::out_of_range&) {
                FAIL_2(
                    "allowed outbailment version not defined for version",
                    input.version())
            }
        } break;
        case PEERREQUEST_PENDINGBAILMENT:
        case PEERREQUEST_STORESECRET: {
            if (!input.has_notice()) { FAIL_1("missing notice") }

            try {
                const bool validnotice = Check(
                    input.notice(),
                    PeerReplyAllowedNotice().at(input.version()).first,
                    PeerReplyAllowedNotice().at(input.version()).second,
                    silent);

                if (!validnotice) { FAIL_1("invalid notice") }
            } catch (const std::out_of_range&) {
                FAIL_2(
                    "allowed peer notice version not defined for version",
                    input.version())
            }
        } break;
        case PEERREQUEST_CONNECTIONINFO: {
            if (!input.has_connectioninfo()) {
                FAIL_1("missing connectioninfo")
            }

            try {
                const bool validconnectioninfo = Check(
                    input.connectioninfo(),
                    PeerReplyAllowedConnectionInfo().at(input.version()).first,
                    PeerReplyAllowedConnectionInfo().at(input.version()).second,
                    silent);

                if (!validconnectioninfo) { FAIL_1("invalid connectioninfo") }
            } catch (const std::out_of_range&) {
                FAIL_2(
                    "allowed connection info version not defined for version",
                    input.version())
            }
        } break;
        case PEERREQUEST_VERIFICATIONOFFER:
        case PEERREQUEST_FAUCET:
        case PEERREQUEST_ERROR:
        default: {
            FAIL_1("invalid type")
        }
    }

    return true;
}
}  // namespace proto
}  // namespace opentxs
