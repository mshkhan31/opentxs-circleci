// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/protobuf/verify/RPCCommand.hpp"  // IWYU pragma: associated

#include <string>

#include "opentxs/protobuf/Basic.hpp"
#include "opentxs/protobuf/RPCCommand.pb.h"
#include "opentxs/protobuf/RPCEnums.pb.h"
#include "opentxs/protobuf/verify/APIArgument.hpp"  // IWYU pragma: keep
#include "opentxs/protobuf/verify/AcceptPendingPayment.hpp"  // IWYU pragma: keep
#include "opentxs/protobuf/verify/AddClaim.hpp"    // IWYU pragma: keep
#include "opentxs/protobuf/verify/AddContact.hpp"  // IWYU pragma: keep
#include "opentxs/protobuf/verify/CreateInstrumentDefinition.hpp"  // IWYU pragma: keep
#include "opentxs/protobuf/verify/CreateNym.hpp"       // IWYU pragma: keep
#include "opentxs/protobuf/verify/GetWorkflow.hpp"     // IWYU pragma: keep
#include "opentxs/protobuf/verify/HDSeed.hpp"          // IWYU pragma: keep
#include "opentxs/protobuf/verify/ModifyAccount.hpp"   // IWYU pragma: keep
#include "opentxs/protobuf/verify/MoveFunds.hpp"       // IWYU pragma: keep
#include "opentxs/protobuf/verify/SendMessage.hpp"     // IWYU pragma: keep
#include "opentxs/protobuf/verify/SendPayment.hpp"     // IWYU pragma: keep
#include "opentxs/protobuf/verify/ServerContract.hpp"  // IWYU pragma: keep
#include "opentxs/protobuf/verify/Verification.hpp"    // IWYU pragma: keep
#include "opentxs/protobuf/verify/VerifyClaim.hpp"     // IWYU pragma: keep
#include "opentxs/protobuf/verify/VerifyContacts.hpp"
#include "opentxs/protobuf/verify/VerifyRPC.hpp"
#include "protobuf/Check.hpp"

#define PROTO_NAME "RPC command"

namespace opentxs::proto
{
auto CheckProto_2(const RPCCommand& input, const bool silent) -> bool
{
    CHECK_IDENTIFIER(cookie)
    CHECK_EXISTS(type)

    switch (input.type()) {
        case RPCCOMMAND_ADDCLIENTSESSION: {
            if (-1 != input.session()) { FAIL_1("invalid session"); }

            OPTIONAL_IDENTIFIERS(associatenym);
            CHECK_EXCLUDED(owner);
            CHECK_EXCLUDED(notary);
            CHECK_EXCLUDED(unit);
            CHECK_NONE(identifier);
            CHECK_SUBOBJECTS(arg, RPCCommandAllowedAPIArgument());
            CHECK_EXCLUDED(hdseed);
            CHECK_EXCLUDED(createnym);
            CHECK_NONE(claim);
            CHECK_NONE(server);
            CHECK_EXCLUDED(createunit);
            CHECK_EXCLUDED(sendpayment);
            CHECK_EXCLUDED(movefunds);
            CHECK_NONE(addcontact);
            CHECK_NONE(verifyclaim);
            CHECK_NONE(sendmessage);
            CHECK_NONE(acceptverification);
            CHECK_NONE(acceptpendingpayment);
            CHECK_NONE(getworkflow);
            CHECK_EXCLUDED(param);
            CHECK_NONE(modifyaccount);
        } break;
        case RPCCOMMAND_ADDSERVERSESSION: {
            if (-1 != input.session()) { FAIL_1("invalid session"); }

            OPTIONAL_IDENTIFIERS(associatenym);
            CHECK_EXCLUDED(owner);
            CHECK_EXCLUDED(notary);
            CHECK_EXCLUDED(unit);
            CHECK_NONE(identifier);
            CHECK_SUBOBJECTS(arg, RPCCommandAllowedAPIArgument());
            CHECK_EXCLUDED(hdseed);
            CHECK_EXCLUDED(createnym);
            CHECK_NONE(claim);
            CHECK_NONE(server);
            CHECK_EXCLUDED(createunit);
            CHECK_EXCLUDED(sendpayment);
            CHECK_EXCLUDED(movefunds);
            CHECK_NONE(addcontact);
            CHECK_NONE(verifyclaim);
            CHECK_NONE(sendmessage);
            CHECK_NONE(acceptverification);
            CHECK_NONE(acceptpendingpayment);
            CHECK_NONE(getworkflow);
            CHECK_EXCLUDED(param);
            CHECK_NONE(modifyaccount);
        } break;
        case RPCCOMMAND_LISTCLIENTSESSIONS: {
            if (-1 != input.session()) { FAIL_1("invalid session"); }

            OPTIONAL_IDENTIFIERS(associatenym);
            CHECK_EXCLUDED(owner);
            CHECK_EXCLUDED(notary);
            CHECK_EXCLUDED(unit);
            CHECK_NONE(identifier);
            CHECK_NONE(arg);
            CHECK_EXCLUDED(hdseed);
            CHECK_EXCLUDED(createnym);
            CHECK_NONE(claim);
            CHECK_NONE(server);
            CHECK_EXCLUDED(createunit);
            CHECK_EXCLUDED(sendpayment);
            CHECK_EXCLUDED(movefunds);
            CHECK_NONE(addcontact);
            CHECK_NONE(verifyclaim);
            CHECK_NONE(sendmessage);
            CHECK_NONE(acceptverification);
            CHECK_NONE(acceptpendingpayment);
            CHECK_NONE(getworkflow);
            CHECK_EXCLUDED(param);
            CHECK_NONE(modifyaccount);
        } break;
        case RPCCOMMAND_LISTSERVERSESSIONS: {
            if (-1 != input.session()) { FAIL_1("invalid session"); }

            OPTIONAL_IDENTIFIERS(associatenym);
            CHECK_EXCLUDED(owner);
            CHECK_EXCLUDED(notary);
            CHECK_EXCLUDED(unit);
            CHECK_NONE(identifier);
            CHECK_NONE(arg);
            CHECK_EXCLUDED(hdseed);
            CHECK_EXCLUDED(createnym);
            CHECK_NONE(claim);
            CHECK_NONE(server);
            CHECK_EXCLUDED(createunit);
            CHECK_EXCLUDED(sendpayment);
            CHECK_EXCLUDED(movefunds);
            CHECK_NONE(addcontact);
            CHECK_NONE(verifyclaim);
            CHECK_NONE(sendmessage);
            CHECK_NONE(acceptverification);
            CHECK_NONE(acceptpendingpayment);
            CHECK_NONE(getworkflow);
            CHECK_EXCLUDED(param);
            CHECK_NONE(modifyaccount);
        } break;
        case RPCCOMMAND_IMPORTHDSEED: {
            if (0 > input.session()) { FAIL_1("invalid session"); }

            OPTIONAL_IDENTIFIERS(associatenym);
            CHECK_EXCLUDED(owner);
            CHECK_EXCLUDED(notary);
            CHECK_EXCLUDED(unit);
            CHECK_NONE(identifier);
            CHECK_NONE(arg);
            CHECK_SUBOBJECT(hdseed, RPCCommandAllowedHDSeed());
            CHECK_EXCLUDED(createnym);
            CHECK_NONE(claim);
            CHECK_NONE(server);
            CHECK_EXCLUDED(createunit);
            CHECK_EXCLUDED(sendpayment);
            CHECK_EXCLUDED(movefunds);
            CHECK_NONE(addcontact);
            CHECK_NONE(verifyclaim);
            CHECK_NONE(sendmessage);
            CHECK_NONE(acceptverification);
            CHECK_NONE(acceptpendingpayment);
            CHECK_NONE(getworkflow);
            CHECK_EXCLUDED(param);
            CHECK_NONE(modifyaccount);
        } break;
        case RPCCOMMAND_LISTHDSEEDS: {
            if (0 > input.session()) { FAIL_1("invalid session"); }

            OPTIONAL_IDENTIFIERS(associatenym);
            CHECK_EXCLUDED(owner);
            CHECK_EXCLUDED(notary);
            CHECK_EXCLUDED(unit);
            CHECK_NONE(identifier);
            CHECK_NONE(arg);
            CHECK_EXCLUDED(hdseed);
            CHECK_EXCLUDED(createnym);
            CHECK_NONE(claim);
            CHECK_NONE(server);
            CHECK_EXCLUDED(createunit);
            CHECK_EXCLUDED(sendpayment);
            CHECK_EXCLUDED(movefunds);
            CHECK_NONE(addcontact);
            CHECK_NONE(verifyclaim);
            CHECK_NONE(sendmessage);
            CHECK_NONE(acceptverification);
            CHECK_NONE(acceptpendingpayment);
            CHECK_NONE(getworkflow);
            CHECK_EXCLUDED(param);
            CHECK_NONE(modifyaccount);
        } break;
        case RPCCOMMAND_GETHDSEED: {
            if (0 > input.session()) { FAIL_1("invalid session"); }

            OPTIONAL_IDENTIFIERS(associatenym);
            CHECK_EXCLUDED(owner);
            CHECK_EXCLUDED(notary);
            CHECK_EXCLUDED(unit);
            CHECK_HAVE(identifier);
            CHECK_IDENTIFIERS(identifier);
            CHECK_NONE(arg);
            CHECK_EXCLUDED(hdseed);
            CHECK_EXCLUDED(createnym);
            CHECK_NONE(claim);
            CHECK_NONE(server);
            CHECK_EXCLUDED(createunit);
            CHECK_EXCLUDED(sendpayment);
            CHECK_EXCLUDED(movefunds);
            CHECK_NONE(addcontact);
            CHECK_NONE(verifyclaim);
            CHECK_NONE(sendmessage);
            CHECK_NONE(acceptverification);
            CHECK_NONE(acceptpendingpayment);
            CHECK_NONE(getworkflow);
            CHECK_EXCLUDED(param);
            CHECK_NONE(modifyaccount);
        } break;
        case RPCCOMMAND_CREATENYM: {
            if (0 > input.session()) { FAIL_1("invalid session"); }

            OPTIONAL_IDENTIFIERS(associatenym);
            CHECK_EXCLUDED(owner);
            CHECK_EXCLUDED(notary);
            CHECK_EXCLUDED(unit);
            CHECK_NONE(identifier);
            CHECK_NONE(arg);
            CHECK_EXCLUDED(hdseed);
            CHECK_SUBOBJECT(createnym, RPCCommandAllowedCreateNym());
            CHECK_NONE(claim);
            CHECK_NONE(server);
            CHECK_EXCLUDED(createunit);
            CHECK_EXCLUDED(sendpayment);
            CHECK_EXCLUDED(movefunds);
            CHECK_NONE(addcontact);
            CHECK_NONE(verifyclaim);
            CHECK_NONE(sendmessage);
            CHECK_NONE(acceptverification);
            CHECK_NONE(acceptpendingpayment);
            CHECK_NONE(getworkflow);
            CHECK_EXCLUDED(param);
            CHECK_NONE(modifyaccount);
        } break;
        case RPCCOMMAND_LISTNYMS: {
            if (0 > input.session()) { FAIL_1("invalid session"); }

            OPTIONAL_IDENTIFIERS(associatenym);
            CHECK_EXCLUDED(owner);
            CHECK_EXCLUDED(notary);
            CHECK_EXCLUDED(unit);
            CHECK_NONE(identifier);
            CHECK_NONE(arg);
            CHECK_EXCLUDED(hdseed);
            CHECK_EXCLUDED(createnym);
            CHECK_NONE(claim);
            CHECK_NONE(server);
            CHECK_EXCLUDED(createunit);
            CHECK_EXCLUDED(sendpayment);
            CHECK_EXCLUDED(movefunds);
            CHECK_NONE(addcontact);
            CHECK_NONE(verifyclaim);
            CHECK_NONE(sendmessage);
            CHECK_NONE(acceptverification);
            CHECK_NONE(acceptpendingpayment);
            CHECK_NONE(getworkflow);
            CHECK_EXCLUDED(param);
            CHECK_NONE(modifyaccount);
        } break;
        case RPCCOMMAND_GETNYM: {
            if (0 > input.session()) { FAIL_1("invalid session"); }

            OPTIONAL_IDENTIFIERS(associatenym);
            CHECK_EXCLUDED(owner);
            CHECK_EXCLUDED(notary);
            CHECK_EXCLUDED(unit);
            CHECK_HAVE(identifier);
            CHECK_IDENTIFIERS(identifier);
            CHECK_NONE(arg);
            CHECK_EXCLUDED(hdseed);
            CHECK_EXCLUDED(createnym);
            CHECK_NONE(claim);
            CHECK_NONE(server);
            CHECK_EXCLUDED(createunit);
            CHECK_EXCLUDED(sendpayment);
            CHECK_EXCLUDED(movefunds);
            CHECK_NONE(addcontact);
            CHECK_NONE(verifyclaim);
            CHECK_NONE(sendmessage);
            CHECK_NONE(acceptverification);
            CHECK_NONE(acceptpendingpayment);
            CHECK_NONE(getworkflow);
            CHECK_EXCLUDED(param);
            CHECK_NONE(modifyaccount);
        } break;
        case RPCCOMMAND_ADDCLAIM: {
            if (0 > input.session()) { FAIL_1("invalid session"); }

            OPTIONAL_IDENTIFIERS(associatenym);
            CHECK_IDENTIFIER(owner);
            CHECK_EXCLUDED(notary);
            CHECK_EXCLUDED(unit);
            CHECK_NONE(identifier);
            CHECK_NONE(arg);
            CHECK_EXCLUDED(hdseed);
            CHECK_EXCLUDED(createnym);
            CHECK_SUBOBJECTS(claim, RPCCommandAllowedAddClaim());
            CHECK_NONE(server);
            CHECK_EXCLUDED(createunit);
            CHECK_EXCLUDED(sendpayment);
            CHECK_EXCLUDED(movefunds);
            CHECK_NONE(addcontact);
            CHECK_NONE(verifyclaim);
            CHECK_NONE(sendmessage);
            CHECK_NONE(acceptverification);
            CHECK_NONE(acceptpendingpayment);
            CHECK_NONE(getworkflow);
            CHECK_EXCLUDED(param);
            CHECK_NONE(modifyaccount);
        } break;
        case RPCCOMMAND_DELETECLAIM: {
            if (0 > input.session()) { FAIL_1("invalid session"); }

            OPTIONAL_IDENTIFIERS(associatenym);
            CHECK_IDENTIFIER(owner);
            CHECK_EXCLUDED(notary);
            CHECK_EXCLUDED(unit);
            CHECK_HAVE(identifier);
            CHECK_IDENTIFIERS(identifier);
            CHECK_NONE(arg);
            CHECK_EXCLUDED(hdseed);
            CHECK_EXCLUDED(createnym);
            CHECK_NONE(claim);
            CHECK_NONE(server);
            CHECK_EXCLUDED(createunit);
            CHECK_EXCLUDED(sendpayment);
            CHECK_EXCLUDED(movefunds);
            CHECK_NONE(addcontact);
            CHECK_NONE(verifyclaim);
            CHECK_NONE(sendmessage);
            CHECK_NONE(acceptverification);
            CHECK_NONE(acceptpendingpayment);
            CHECK_NONE(getworkflow);
            CHECK_EXCLUDED(param);
            CHECK_NONE(modifyaccount);
        } break;
        case RPCCOMMAND_IMPORTSERVERCONTRACT: {
            if (0 > input.session()) { FAIL_1("invalid session"); }

            OPTIONAL_IDENTIFIERS(associatenym);
            CHECK_EXCLUDED(owner);
            CHECK_EXCLUDED(notary);
            CHECK_EXCLUDED(unit);
            CHECK_NONE(identifier);
            CHECK_NONE(arg);
            CHECK_EXCLUDED(hdseed);
            CHECK_EXCLUDED(createnym);
            CHECK_NONE(claim);
            CHECK_HAVE(server);
            CHECK_EXCLUDED(createunit);
            CHECK_SUBOBJECTS(server, RPCCommandAllowedServerContract());
            CHECK_EXCLUDED(sendpayment);
            CHECK_EXCLUDED(movefunds);
            CHECK_NONE(addcontact);
            CHECK_NONE(verifyclaim);
            CHECK_NONE(sendmessage);
            CHECK_NONE(acceptverification);
            CHECK_NONE(acceptpendingpayment);
            CHECK_NONE(getworkflow);
            CHECK_EXCLUDED(param);
            CHECK_NONE(modifyaccount);
        } break;
        case RPCCOMMAND_LISTSERVERCONTRACTS: {
            if (0 > input.session()) { FAIL_1("invalid session"); }

            OPTIONAL_IDENTIFIERS(associatenym);
            CHECK_EXCLUDED(owner);
            CHECK_EXCLUDED(notary);
            CHECK_EXCLUDED(unit);
            CHECK_NONE(identifier);
            CHECK_NONE(arg);
            CHECK_EXCLUDED(hdseed);
            CHECK_EXCLUDED(createnym);
            CHECK_NONE(claim);
            CHECK_NONE(server);
            CHECK_EXCLUDED(createunit);
            CHECK_EXCLUDED(sendpayment);
            CHECK_EXCLUDED(movefunds);
            CHECK_NONE(addcontact);
            CHECK_NONE(verifyclaim);
            CHECK_NONE(sendmessage);
            CHECK_NONE(acceptverification);
            CHECK_NONE(acceptpendingpayment);
            CHECK_NONE(getworkflow);
            CHECK_EXCLUDED(param);
            CHECK_NONE(modifyaccount);
        } break;
        case RPCCOMMAND_REGISTERNYM: {
            if (0 > input.session()) { FAIL_1("invalid session"); }

            OPTIONAL_IDENTIFIERS(associatenym);
            CHECK_IDENTIFIER(owner);
            CHECK_IDENTIFIER(notary);
            CHECK_EXCLUDED(unit);
            CHECK_NONE(identifier);
            CHECK_NONE(arg);
            CHECK_EXCLUDED(hdseed);
            CHECK_EXCLUDED(createnym);
            CHECK_NONE(claim);
            CHECK_NONE(server);
            CHECK_EXCLUDED(createunit);
            CHECK_EXCLUDED(sendpayment);
            CHECK_EXCLUDED(movefunds);
            CHECK_NONE(addcontact);
            CHECK_NONE(verifyclaim);
            CHECK_NONE(sendmessage);
            CHECK_NONE(acceptverification);
            CHECK_NONE(acceptpendingpayment);
            CHECK_NONE(getworkflow);
            CHECK_EXCLUDED(param);
            CHECK_NONE(modifyaccount);
        } break;
        case RPCCOMMAND_CREATEUNITDEFINITION: {
            if (0 > input.session()) { FAIL_1("invalid session"); }

            OPTIONAL_IDENTIFIERS(associatenym);
            CHECK_IDENTIFIER(owner);
            CHECK_EXCLUDED(notary);
            CHECK_EXCLUDED(unit);
            CHECK_NONE(identifier);
            CHECK_NONE(arg);
            CHECK_EXCLUDED(hdseed);
            CHECK_EXCLUDED(createnym);
            CHECK_NONE(claim);
            CHECK_NONE(server);
            CHECK_SUBOBJECT(
                createunit, RPCCommandAllowedCreateInstrumentDefinition());
            CHECK_EXCLUDED(sendpayment);
            CHECK_EXCLUDED(movefunds);
            CHECK_NONE(addcontact);
            CHECK_NONE(verifyclaim);
            CHECK_NONE(sendmessage);
            CHECK_NONE(acceptverification);
            CHECK_NONE(acceptpendingpayment);
            CHECK_NONE(getworkflow);
            CHECK_EXCLUDED(param);
            CHECK_NONE(modifyaccount);
        } break;
        case RPCCOMMAND_LISTUNITDEFINITIONS: {
            if (0 > input.session()) { FAIL_1("invalid session"); }

            OPTIONAL_IDENTIFIERS(associatenym);
            CHECK_EXCLUDED(owner);
            CHECK_EXCLUDED(notary);
            CHECK_EXCLUDED(unit);
            CHECK_NONE(identifier);
            CHECK_NONE(arg);
            CHECK_EXCLUDED(hdseed);
            CHECK_EXCLUDED(createnym);
            CHECK_NONE(claim);
            CHECK_NONE(server);
            CHECK_EXCLUDED(createunit);
            CHECK_EXCLUDED(sendpayment);
            CHECK_EXCLUDED(movefunds);
            CHECK_NONE(addcontact);
            CHECK_NONE(verifyclaim);
            CHECK_NONE(sendmessage);
            CHECK_NONE(acceptverification);
            CHECK_NONE(acceptpendingpayment);
            CHECK_NONE(getworkflow);
            CHECK_EXCLUDED(param);
            CHECK_NONE(modifyaccount);
        } break;
        case RPCCOMMAND_ISSUEUNITDEFINITION: {
            if (0 > input.session()) { FAIL_1("invalid session"); }

            OPTIONAL_IDENTIFIERS(associatenym);
            CHECK_IDENTIFIER(owner);
            CHECK_IDENTIFIER(notary);
            CHECK_IDENTIFIER(unit);

            if (0 != input.identifier().size()) {
                CHECK_SIZE(identifier, 1);
                const auto size = input.identifier(0).size();

                if (MAX_VALID_CONTACT_VALUE < size) {
                    FAIL_2("invalid alias size", size);
                }
            }

            CHECK_NONE(arg);
            CHECK_EXCLUDED(hdseed);
            CHECK_EXCLUDED(createnym);
            CHECK_NONE(claim);
            CHECK_NONE(server);
            CHECK_EXCLUDED(createunit);
            CHECK_EXCLUDED(sendpayment);
            CHECK_EXCLUDED(movefunds);
            CHECK_NONE(addcontact);
            CHECK_NONE(verifyclaim);
            CHECK_NONE(sendmessage);
            CHECK_NONE(acceptverification);
            CHECK_NONE(acceptpendingpayment);
            CHECK_NONE(getworkflow);
            CHECK_EXCLUDED(param);
            CHECK_NONE(modifyaccount);
        } break;
        case RPCCOMMAND_CREATEACCOUNT: {
            if (0 > input.session()) { FAIL_1("invalid session"); }

            OPTIONAL_IDENTIFIERS(associatenym);
            CHECK_IDENTIFIER(owner);
            CHECK_IDENTIFIER(notary);
            CHECK_IDENTIFIER(unit);

            if (0 != input.identifier().size()) {
                CHECK_SIZE(identifier, 1);
                const auto size = input.identifier(0).size();

                if (MAX_VALID_CONTACT_VALUE < size) {
                    FAIL_2("invalid alias size", size);
                }
            }

            CHECK_NONE(arg);
            CHECK_EXCLUDED(hdseed);
            CHECK_EXCLUDED(createnym);
            CHECK_NONE(claim);
            CHECK_NONE(server);
            CHECK_EXCLUDED(createunit);
            CHECK_EXCLUDED(sendpayment);
            CHECK_EXCLUDED(movefunds);
            CHECK_NONE(addcontact);
            CHECK_NONE(verifyclaim);
            CHECK_NONE(sendmessage);
            CHECK_NONE(acceptverification);
            CHECK_NONE(acceptpendingpayment);
            CHECK_NONE(getworkflow);
            CHECK_EXCLUDED(param);
            CHECK_NONE(modifyaccount);
        } break;
        case RPCCOMMAND_LISTACCOUNTS: {
            if (0 > input.session()) { FAIL_1("invalid session"); }

            OPTIONAL_IDENTIFIERS(associatenym);
            CHECK_EXCLUDED(owner);
            OPTIONAL_IDENTIFIER(notary);
            OPTIONAL_IDENTIFIER(unit);
            CHECK_NONE(identifier);
            CHECK_NONE(arg);
            CHECK_EXCLUDED(hdseed);
            CHECK_EXCLUDED(createnym);
            CHECK_NONE(claim);
            CHECK_NONE(server);
            CHECK_EXCLUDED(createunit);
            CHECK_EXCLUDED(sendpayment);
            CHECK_EXCLUDED(movefunds);
            CHECK_NONE(addcontact);
            CHECK_NONE(verifyclaim);
            CHECK_NONE(sendmessage);
            CHECK_NONE(acceptverification);
            CHECK_NONE(acceptpendingpayment);
            CHECK_NONE(getworkflow);
            CHECK_EXCLUDED(param);
            CHECK_NONE(modifyaccount);
        } break;
        case RPCCOMMAND_GETACCOUNTBALANCE: {
            if (0 > input.session()) { FAIL_1("invalid session"); }

            OPTIONAL_IDENTIFIERS(associatenym);
            CHECK_EXCLUDED(owner);
            CHECK_EXCLUDED(notary);
            CHECK_EXCLUDED(unit);
            CHECK_HAVE(identifier);
            CHECK_IDENTIFIERS(identifier);
            CHECK_NONE(arg);
            CHECK_EXCLUDED(hdseed);
            CHECK_EXCLUDED(createnym);
            CHECK_NONE(claim);
            CHECK_NONE(server);
            CHECK_EXCLUDED(createunit);
            CHECK_EXCLUDED(sendpayment);
            CHECK_EXCLUDED(movefunds);
            CHECK_NONE(addcontact);
            CHECK_NONE(verifyclaim);
            CHECK_NONE(sendmessage);
            CHECK_NONE(acceptverification);
            CHECK_NONE(acceptpendingpayment);
            CHECK_NONE(getworkflow);
            CHECK_EXCLUDED(param);
            CHECK_NONE(modifyaccount);
        } break;
        case RPCCOMMAND_GETACCOUNTACTIVITY: {
            if (0 > input.session()) { FAIL_1("invalid session"); }

            OPTIONAL_IDENTIFIERS(associatenym);
            CHECK_EXCLUDED(owner);
            CHECK_EXCLUDED(notary);
            CHECK_EXCLUDED(unit);
            CHECK_HAVE(identifier);
            CHECK_IDENTIFIERS(identifier);
            CHECK_NONE(arg);
            CHECK_EXCLUDED(hdseed);
            CHECK_EXCLUDED(createnym);
            CHECK_NONE(claim);
            CHECK_NONE(server);
            CHECK_EXCLUDED(createunit);
            CHECK_EXCLUDED(sendpayment);
            CHECK_EXCLUDED(movefunds);
            CHECK_NONE(addcontact);
            CHECK_NONE(verifyclaim);
            CHECK_NONE(sendmessage);
            CHECK_NONE(acceptverification);
            CHECK_NONE(acceptpendingpayment);
            CHECK_NONE(getworkflow);
            CHECK_EXCLUDED(param);
            CHECK_NONE(modifyaccount);
        } break;
        case RPCCOMMAND_SENDPAYMENT: {
            if (0 > input.session()) { FAIL_1("invalid session"); }

            OPTIONAL_IDENTIFIERS(associatenym);
            CHECK_EXCLUDED(owner);
            CHECK_EXCLUDED(notary);
            CHECK_EXCLUDED(unit);
            CHECK_NONE(identifier);
            CHECK_NONE(arg);
            CHECK_EXCLUDED(hdseed);
            CHECK_EXCLUDED(createnym);
            CHECK_NONE(claim);
            CHECK_NONE(server);
            CHECK_EXCLUDED(createunit);
            CHECK_SUBOBJECT(sendpayment, RPCCommandAllowedSendPayment());
            CHECK_EXCLUDED(movefunds);
            CHECK_NONE(addcontact);
            CHECK_NONE(verifyclaim);
            CHECK_NONE(sendmessage);
            CHECK_NONE(acceptverification);
            CHECK_NONE(acceptpendingpayment);
            CHECK_NONE(getworkflow);
            CHECK_EXCLUDED(param);
            CHECK_NONE(modifyaccount);
        } break;
        case RPCCOMMAND_MOVEFUNDS: {
            if (0 > input.session()) { FAIL_1("invalid session"); }

            OPTIONAL_IDENTIFIERS(associatenym);
            CHECK_EXCLUDED(owner);
            CHECK_EXCLUDED(notary);
            CHECK_EXCLUDED(unit);
            CHECK_NONE(identifier);
            CHECK_NONE(arg);
            CHECK_EXCLUDED(hdseed);
            CHECK_EXCLUDED(createnym);
            CHECK_NONE(claim);
            CHECK_NONE(server);
            CHECK_EXCLUDED(createunit);
            CHECK_EXCLUDED(sendpayment);
            CHECK_SUBOBJECT(movefunds, RPCCommandAllowedSendPayment());
            CHECK_NONE(addcontact);
            CHECK_NONE(verifyclaim);
            CHECK_NONE(sendmessage);
            CHECK_NONE(acceptverification);
            CHECK_NONE(acceptpendingpayment);
            CHECK_NONE(getworkflow);
            CHECK_EXCLUDED(param);
            CHECK_NONE(modifyaccount);
        } break;
        case RPCCOMMAND_ADDCONTACT: {
            if (0 > input.session()) { FAIL_1("invalid session"); }

            OPTIONAL_IDENTIFIERS(associatenym);
            CHECK_EXCLUDED(owner);
            CHECK_EXCLUDED(notary);
            CHECK_EXCLUDED(unit);
            CHECK_NONE(identifier);
            CHECK_NONE(arg);
            CHECK_EXCLUDED(hdseed);
            CHECK_EXCLUDED(createnym);
            CHECK_NONE(claim);
            CHECK_NONE(server);
            CHECK_EXCLUDED(createunit);
            CHECK_EXCLUDED(sendpayment);
            CHECK_EXCLUDED(movefunds);
            CHECK_SUBOBJECTS(addcontact, RPCCommandAllowedAddContact());
            CHECK_NONE(verifyclaim);
            CHECK_NONE(sendmessage);
            CHECK_NONE(acceptverification);
            CHECK_NONE(acceptpendingpayment);
            CHECK_NONE(getworkflow);
            CHECK_EXCLUDED(param);
            CHECK_NONE(modifyaccount);
        } break;
        case RPCCOMMAND_LISTCONTACTS: {
            if (0 > input.session()) { FAIL_1("invalid session"); }

            OPTIONAL_IDENTIFIERS(associatenym);
            CHECK_EXCLUDED(owner);
            CHECK_EXCLUDED(notary);
            CHECK_EXCLUDED(unit);
            CHECK_NONE(identifier);
            CHECK_NONE(arg);
            CHECK_EXCLUDED(hdseed);
            CHECK_EXCLUDED(createnym);
            CHECK_NONE(claim);
            CHECK_NONE(server);
            CHECK_EXCLUDED(createunit);
            CHECK_EXCLUDED(sendpayment);
            CHECK_EXCLUDED(movefunds);
            CHECK_NONE(addcontact);
            CHECK_NONE(verifyclaim);
            CHECK_NONE(sendmessage);
            CHECK_NONE(acceptverification);
            CHECK_NONE(acceptpendingpayment);
            CHECK_NONE(getworkflow);
            CHECK_EXCLUDED(param);
            CHECK_NONE(modifyaccount);
        } break;
        case RPCCOMMAND_GETCONTACT: {
            if (0 > input.session()) { FAIL_1("invalid session"); }

            OPTIONAL_IDENTIFIERS(associatenym);
            CHECK_EXCLUDED(owner);
            CHECK_EXCLUDED(notary);
            CHECK_EXCLUDED(unit);
            CHECK_HAVE(identifier);
            CHECK_IDENTIFIERS(identifier);
            CHECK_NONE(arg);
            CHECK_EXCLUDED(hdseed);
            CHECK_EXCLUDED(createnym);
            CHECK_NONE(claim);
            CHECK_NONE(server);
            CHECK_EXCLUDED(createunit);
            CHECK_EXCLUDED(sendpayment);
            CHECK_EXCLUDED(movefunds);
            CHECK_NONE(addcontact);
            CHECK_NONE(verifyclaim);
            CHECK_NONE(sendmessage);
            CHECK_NONE(acceptverification);
            CHECK_NONE(acceptpendingpayment);
            CHECK_NONE(getworkflow);
            CHECK_EXCLUDED(param);
            CHECK_NONE(modifyaccount);
        } break;
        case RPCCOMMAND_ADDCONTACTCLAIM: {
            if (0 > input.session()) { FAIL_1("invalid session"); }

            OPTIONAL_IDENTIFIERS(associatenym);
            CHECK_IDENTIFIER(owner);
            CHECK_EXCLUDED(notary);
            CHECK_EXCLUDED(unit);
            CHECK_NONE(identifier);
            CHECK_NONE(arg);
            CHECK_EXCLUDED(hdseed);
            CHECK_EXCLUDED(createnym);
            CHECK_SUBOBJECTS(claim, RPCCommandAllowedAddClaim());
            CHECK_NONE(server);
            CHECK_EXCLUDED(createunit);
            CHECK_EXCLUDED(sendpayment);
            CHECK_EXCLUDED(movefunds);
            CHECK_NONE(addcontact);
            CHECK_NONE(verifyclaim);
            CHECK_NONE(sendmessage);
            CHECK_NONE(acceptverification);
            CHECK_NONE(acceptpendingpayment);
            CHECK_NONE(getworkflow);
            CHECK_EXCLUDED(param);
            CHECK_NONE(modifyaccount);
        } break;
        case RPCCOMMAND_DELETECONTACTCLAIM: {
            if (0 > input.session()) { FAIL_1("invalid session"); }

            OPTIONAL_IDENTIFIERS(associatenym);
            CHECK_IDENTIFIER(owner);
            CHECK_EXCLUDED(notary);
            CHECK_EXCLUDED(unit);
            CHECK_HAVE(identifier);
            CHECK_IDENTIFIERS(identifier);
            CHECK_NONE(arg);
            CHECK_EXCLUDED(hdseed);
            CHECK_EXCLUDED(createnym);
            CHECK_NONE(claim);
            CHECK_NONE(server);
            CHECK_EXCLUDED(createunit);
            CHECK_EXCLUDED(sendpayment);
            CHECK_EXCLUDED(movefunds);
            CHECK_NONE(addcontact);
            CHECK_NONE(verifyclaim);
            CHECK_NONE(sendmessage);
            CHECK_NONE(acceptverification);
            CHECK_NONE(acceptpendingpayment);
            CHECK_NONE(getworkflow);
            CHECK_EXCLUDED(param);
            CHECK_NONE(modifyaccount);
        } break;
        case RPCCOMMAND_VERIFYCLAIM: {
            if (0 > input.session()) { FAIL_1("invalid session"); }

            OPTIONAL_IDENTIFIERS(associatenym);
            CHECK_EXCLUDED(owner);
            CHECK_EXCLUDED(notary);
            CHECK_EXCLUDED(unit);
            CHECK_NONE(identifier);
            CHECK_NONE(arg);
            CHECK_EXCLUDED(hdseed);
            CHECK_EXCLUDED(createnym);
            CHECK_NONE(claim);
            CHECK_NONE(server);
            CHECK_EXCLUDED(createunit);
            CHECK_EXCLUDED(sendpayment);
            CHECK_EXCLUDED(movefunds);
            CHECK_NONE(addcontact);
            CHECK_SUBOBJECTS(verifyclaim, RPCCommandAllowedVerifyClaim());
            CHECK_NONE(sendmessage);
            CHECK_NONE(acceptverification);
            CHECK_NONE(acceptpendingpayment);
            CHECK_NONE(getworkflow);
            CHECK_EXCLUDED(param);
            CHECK_NONE(modifyaccount);
        } break;
        case RPCCOMMAND_ACCEPTVERIFICATION: {
            if (0 > input.session()) { FAIL_1("invalid session"); }

            OPTIONAL_IDENTIFIERS(associatenym);
            CHECK_IDENTIFIER(owner);
            CHECK_EXCLUDED(notary);
            CHECK_EXCLUDED(unit);
            CHECK_NONE(identifier);
            CHECK_NONE(arg);
            CHECK_EXCLUDED(hdseed);
            CHECK_EXCLUDED(createnym);
            CHECK_NONE(claim);
            CHECK_NONE(server);
            CHECK_EXCLUDED(createunit);
            CHECK_EXCLUDED(sendpayment);
            CHECK_EXCLUDED(movefunds);
            CHECK_NONE(addcontact);
            CHECK_NONE(verifyclaim);
            CHECK_NONE(sendmessage);
            CHECK_SUBOBJECTS_VA(
                acceptverification,
                RPCCommandAllowedVerification(),
                VerificationType::Indexed);
            CHECK_NONE(acceptpendingpayment);
            CHECK_NONE(getworkflow);
            CHECK_EXCLUDED(param);
            CHECK_NONE(modifyaccount);
        } break;
        case RPCCOMMAND_SENDCONTACTMESSAGE: {
            if (0 > input.session()) { FAIL_1("invalid session"); }

            OPTIONAL_IDENTIFIERS(associatenym);
            CHECK_EXCLUDED(owner);
            CHECK_EXCLUDED(notary);
            CHECK_EXCLUDED(unit);
            CHECK_NONE(identifier);
            CHECK_NONE(arg);
            CHECK_EXCLUDED(hdseed);
            CHECK_EXCLUDED(createnym);
            CHECK_NONE(claim);
            CHECK_NONE(server);
            CHECK_EXCLUDED(createunit);
            CHECK_EXCLUDED(sendpayment);
            CHECK_EXCLUDED(movefunds);
            CHECK_NONE(addcontact);
            CHECK_NONE(verifyclaim);
            CHECK_SUBOBJECTS(sendmessage, RPCCommandAllowedSendMessage());
            CHECK_NONE(acceptverification);
            CHECK_NONE(acceptpendingpayment);
            CHECK_NONE(getworkflow);
            CHECK_EXCLUDED(param);
            CHECK_NONE(modifyaccount);
        } break;
        case RPCCOMMAND_GETCONTACTACTIVITY: {
            if (0 > input.session()) { FAIL_1("invalid session"); }

            OPTIONAL_IDENTIFIERS(associatenym);
            CHECK_EXCLUDED(owner);
            CHECK_EXCLUDED(notary);
            CHECK_EXCLUDED(unit);
            CHECK_HAVE(identifier);
            CHECK_IDENTIFIERS(identifier);
            CHECK_NONE(arg);
            CHECK_EXCLUDED(hdseed);
            CHECK_EXCLUDED(createnym);
            CHECK_NONE(claim);
            CHECK_NONE(server);
            CHECK_EXCLUDED(createunit);
            CHECK_EXCLUDED(sendpayment);
            CHECK_EXCLUDED(movefunds);
            CHECK_NONE(addcontact);
            CHECK_NONE(verifyclaim);
            CHECK_NONE(sendmessage);
            CHECK_NONE(acceptverification);
            CHECK_NONE(acceptpendingpayment);
            CHECK_NONE(getworkflow);
            CHECK_EXCLUDED(param);
            CHECK_NONE(modifyaccount);
        } break;
        case RPCCOMMAND_GETSERVERCONTRACT: {
            if (0 > input.session()) { FAIL_1("invalid session"); }

            OPTIONAL_IDENTIFIERS(associatenym);
            CHECK_EXCLUDED(owner);
            CHECK_EXCLUDED(notary);
            CHECK_EXCLUDED(unit);
            CHECK_HAVE(identifier);
            CHECK_IDENTIFIERS(identifier);
            CHECK_NONE(arg);
            CHECK_EXCLUDED(hdseed);
            CHECK_EXCLUDED(createnym);
            CHECK_NONE(claim);
            CHECK_NONE(server);
            CHECK_EXCLUDED(createunit);
            CHECK_EXCLUDED(sendpayment);
            CHECK_EXCLUDED(movefunds);
            CHECK_NONE(addcontact);
            CHECK_NONE(verifyclaim);
            CHECK_NONE(sendmessage);
            CHECK_NONE(acceptverification);
            CHECK_NONE(acceptpendingpayment);
            CHECK_NONE(getworkflow);
            CHECK_EXCLUDED(param);
            CHECK_NONE(modifyaccount);
        } break;
        case RPCCOMMAND_GETPENDINGPAYMENTS: {
            if (0 > input.session()) { FAIL_1("invalid session"); }

            OPTIONAL_IDENTIFIERS(associatenym);
            CHECK_HAVE(owner);
            CHECK_EXCLUDED(notary);
            CHECK_EXCLUDED(unit);
            CHECK_NONE(identifier);
            CHECK_NONE(arg);
            CHECK_EXCLUDED(hdseed);
            CHECK_EXCLUDED(createnym);
            CHECK_NONE(claim);
            CHECK_NONE(server);
            CHECK_EXCLUDED(createunit);
            CHECK_EXCLUDED(sendpayment);
            CHECK_EXCLUDED(movefunds);
            CHECK_NONE(addcontact);
            CHECK_NONE(verifyclaim);
            CHECK_NONE(sendmessage);
            CHECK_NONE(acceptverification);
            CHECK_NONE(acceptpendingpayment);
            CHECK_NONE(getworkflow);
            CHECK_EXCLUDED(param);
            CHECK_NONE(modifyaccount);
        } break;
        case RPCCOMMAND_ACCEPTPENDINGPAYMENTS: {
            if (0 > input.session()) { FAIL_1("invalid session"); }

            OPTIONAL_IDENTIFIERS(associatenym);
            CHECK_EXCLUDED(owner);
            CHECK_EXCLUDED(notary);
            CHECK_EXCLUDED(unit);
            CHECK_NONE(identifier);
            CHECK_NONE(arg);
            CHECK_EXCLUDED(hdseed);
            CHECK_EXCLUDED(createnym);
            CHECK_NONE(claim);
            CHECK_NONE(server);
            CHECK_EXCLUDED(createunit);
            CHECK_EXCLUDED(sendpayment);
            CHECK_EXCLUDED(movefunds);
            CHECK_NONE(addcontact);
            CHECK_NONE(verifyclaim);
            CHECK_NONE(sendmessage);
            CHECK_NONE(acceptverification);
            CHECK_SUBOBJECTS(
                acceptpendingpayment, RPCCommandAllowedAcceptPendingPayment());
            CHECK_NONE(getworkflow);
            CHECK_EXCLUDED(param);
            CHECK_NONE(modifyaccount);
        } break;
        case RPCCOMMAND_CREATECOMPATIBLEACCOUNT:
        case RPCCOMMAND_GETCOMPATIBLEACCOUNTS: {
            if (0 > input.session()) { FAIL_1("invalid session"); }

            OPTIONAL_IDENTIFIERS(associatenym);
            CHECK_HAVE(owner);
            CHECK_EXCLUDED(notary);
            CHECK_EXCLUDED(unit);
            CHECK_HAVE(identifier);
            CHECK_IDENTIFIERS(identifier);
            CHECK_NONE(arg);
            CHECK_EXCLUDED(hdseed);
            CHECK_EXCLUDED(createnym);
            CHECK_NONE(claim);
            CHECK_NONE(server);
            CHECK_EXCLUDED(createunit);
            CHECK_EXCLUDED(sendpayment);
            CHECK_EXCLUDED(movefunds);
            CHECK_NONE(addcontact);
            CHECK_NONE(verifyclaim);
            CHECK_NONE(sendmessage);
            CHECK_NONE(acceptverification);
            CHECK_NONE(acceptpendingpayment);
            CHECK_NONE(getworkflow);
            CHECK_EXCLUDED(param);
            CHECK_NONE(modifyaccount);
        } break;
        case RPCCOMMAND_GETWORKFLOW: {
            if (0 > input.session()) { FAIL_1("invalid session"); }

            OPTIONAL_IDENTIFIERS(associatenym);
            CHECK_EXCLUDED(owner);
            CHECK_EXCLUDED(notary);
            CHECK_EXCLUDED(unit);
            CHECK_NONE(identifier);
            CHECK_SUBOBJECTS(arg, RPCCommandAllowedAPIArgument());
            CHECK_EXCLUDED(hdseed);
            CHECK_EXCLUDED(createnym);
            CHECK_NONE(claim);
            CHECK_NONE(server);
            CHECK_EXCLUDED(createunit);
            CHECK_EXCLUDED(sendpayment);
            CHECK_EXCLUDED(movefunds);
            CHECK_NONE(addcontact);
            CHECK_NONE(verifyclaim);
            CHECK_NONE(sendmessage);
            CHECK_NONE(acceptverification);
            CHECK_NONE(acceptpendingpayment);
            CHECK_SUBOBJECTS(getworkflow, RPCCommandAllowedGetWorkflow());
            CHECK_EXCLUDED(param);
            CHECK_NONE(modifyaccount);
        } break;
        case RPCCOMMAND_GETSERVERPASSWORD: {
            if (0 > input.session()) { FAIL_1("invalid session"); }

            OPTIONAL_IDENTIFIERS(associatenym);
            CHECK_EXCLUDED(owner);
            CHECK_EXCLUDED(notary);
            CHECK_EXCLUDED(unit);
            CHECK_NONE(identifier);
            CHECK_NONE(arg);
            CHECK_EXCLUDED(hdseed);
            CHECK_EXCLUDED(createnym);
            CHECK_NONE(claim);
            CHECK_NONE(server);
            CHECK_EXCLUDED(createunit);
            CHECK_EXCLUDED(sendpayment);
            CHECK_EXCLUDED(movefunds);
            CHECK_NONE(addcontact);
            CHECK_NONE(verifyclaim);
            CHECK_NONE(sendmessage);
            CHECK_NONE(acceptverification);
            CHECK_NONE(acceptpendingpayment);
            CHECK_NONE(getworkflow);
            CHECK_EXCLUDED(param);
            CHECK_NONE(modifyaccount);
        } break;
        case RPCCOMMAND_GETADMINNYM: {
            if (0 > input.session()) { FAIL_1("invalid session"); }

            OPTIONAL_IDENTIFIERS(associatenym);
            CHECK_EXCLUDED(owner);
            CHECK_EXCLUDED(notary);
            CHECK_EXCLUDED(unit);
            CHECK_NONE(identifier);
            CHECK_NONE(arg);
            CHECK_EXCLUDED(hdseed);
            CHECK_EXCLUDED(createnym);
            CHECK_NONE(claim);
            CHECK_NONE(server);
            CHECK_EXCLUDED(createunit);
            CHECK_EXCLUDED(sendpayment);
            CHECK_EXCLUDED(movefunds);
            CHECK_NONE(addcontact);
            CHECK_NONE(verifyclaim);
            CHECK_NONE(sendmessage);
            CHECK_NONE(acceptverification);
            CHECK_NONE(acceptpendingpayment);
            CHECK_NONE(getworkflow);
            CHECK_EXCLUDED(param);
            CHECK_NONE(modifyaccount);
        } break;
        case RPCCOMMAND_GETUNITDEFINITION: {
            if (0 > input.session()) { FAIL_1("invalid session"); }

            OPTIONAL_IDENTIFIERS(associatenym);
            CHECK_EXCLUDED(owner);
            CHECK_EXCLUDED(notary);
            CHECK_EXCLUDED(unit);
            CHECK_HAVE(identifier);
            CHECK_IDENTIFIERS(identifier);
            CHECK_NONE(arg);
            CHECK_EXCLUDED(hdseed);
            CHECK_EXCLUDED(createnym);
            CHECK_NONE(claim);
            CHECK_NONE(server);
            CHECK_EXCLUDED(createunit);
            CHECK_EXCLUDED(sendpayment);
            CHECK_EXCLUDED(movefunds);
            CHECK_NONE(addcontact);
            CHECK_NONE(verifyclaim);
            CHECK_NONE(sendmessage);
            CHECK_NONE(acceptverification);
            CHECK_NONE(acceptpendingpayment);
            CHECK_NONE(getworkflow);
            CHECK_EXCLUDED(param);
            CHECK_NONE(modifyaccount);
        } break;
        case RPCCOMMAND_GETTRANSACTIONDATA: {
            if (0 > input.session()) { FAIL_1("invalid session"); }

            OPTIONAL_IDENTIFIERS(associatenym);
            CHECK_EXCLUDED(owner);
            CHECK_EXCLUDED(notary);
            CHECK_EXCLUDED(unit);
            CHECK_HAVE(identifier);
            CHECK_IDENTIFIERS(identifier);
            CHECK_NONE(arg);
            CHECK_EXCLUDED(hdseed);
            CHECK_EXCLUDED(createnym);
            CHECK_NONE(claim);
            CHECK_NONE(server);
            CHECK_EXCLUDED(createunit);
            CHECK_EXCLUDED(sendpayment);
            CHECK_EXCLUDED(movefunds);
            CHECK_NONE(addcontact);
            CHECK_NONE(verifyclaim);
            CHECK_NONE(sendmessage);
            CHECK_NONE(acceptverification);
            CHECK_NONE(acceptpendingpayment);
            CHECK_NONE(getworkflow);
            CHECK_EXCLUDED(param);
            CHECK_NONE(modifyaccount);
        } break;
        case RPCCOMMAND_LOOKUPACCOUNTID: {
            if (0 > input.session()) { FAIL_1("invalid session"); }

            OPTIONAL_IDENTIFIERS(associatenym);
            CHECK_EXCLUDED(owner);
            CHECK_EXCLUDED(notary);
            CHECK_EXCLUDED(unit);
            CHECK_NONE(identifier);
            CHECK_NONE(arg);
            CHECK_EXCLUDED(hdseed);
            CHECK_EXCLUDED(createnym);
            CHECK_NONE(claim);
            CHECK_NONE(server);
            CHECK_EXCLUDED(createunit);
            CHECK_EXCLUDED(sendpayment);
            CHECK_EXCLUDED(movefunds);
            CHECK_NONE(addcontact);
            CHECK_NONE(verifyclaim);
            CHECK_NONE(sendmessage);
            CHECK_NONE(acceptverification);
            CHECK_NONE(acceptpendingpayment);
            CHECK_NONE(getworkflow);
            CHECK_NAME(param);
            CHECK_NONE(modifyaccount);
        } break;
        case RPCCOMMAND_RENAMEACCOUNT: {
            if (0 > input.session()) { FAIL_1("invalid session"); }

            OPTIONAL_IDENTIFIERS(associatenym);
            CHECK_EXCLUDED(owner);
            CHECK_EXCLUDED(notary);
            CHECK_EXCLUDED(unit);
            CHECK_NONE(identifier);
            CHECK_NONE(arg);
            CHECK_EXCLUDED(hdseed);
            CHECK_EXCLUDED(createnym);
            CHECK_NONE(claim);
            CHECK_NONE(server);
            CHECK_EXCLUDED(createunit);
            CHECK_EXCLUDED(sendpayment);
            CHECK_EXCLUDED(movefunds);
            CHECK_NONE(addcontact);
            CHECK_NONE(verifyclaim);
            CHECK_NONE(sendmessage);
            CHECK_NONE(acceptverification);
            CHECK_NONE(acceptpendingpayment);
            CHECK_NONE(getworkflow);
            CHECK_EXCLUDED(param);
            CHECK_SUBOBJECTS(modifyaccount, RPCCommandAllowedModifyAccount());
        } break;
        case RPCCOMMAND_ERROR:
        default: {
            FAIL_1("invalid type")
        }
    }

    return true;
}

auto CheckProto_3(const RPCCommand& input, const bool silent) -> bool
{
    CHECK_IDENTIFIER(cookie)
    CHECK_EXISTS(type)

    switch (input.type()) {
        case RPCCOMMAND_LISTACCOUNTS: {
            if (0 > input.session()) { FAIL_1("invalid session"); }

            OPTIONAL_IDENTIFIERS(associatenym);
            OPTIONAL_IDENTIFIER(owner);
            OPTIONAL_IDENTIFIER(notary);
            OPTIONAL_IDENTIFIER(unit);
            CHECK_NONE(identifier);
            CHECK_NONE(arg);
            CHECK_EXCLUDED(hdseed);
            CHECK_EXCLUDED(createnym);
            CHECK_NONE(claim);
            CHECK_NONE(server);
            CHECK_EXCLUDED(createunit);
            CHECK_EXCLUDED(sendpayment);
            CHECK_EXCLUDED(movefunds);
            CHECK_NONE(addcontact);
            CHECK_NONE(verifyclaim);
            CHECK_NONE(sendmessage);
            CHECK_NONE(acceptverification);
            CHECK_NONE(acceptpendingpayment);
            CHECK_NONE(getworkflow);
            CHECK_EXCLUDED(param);
            CHECK_NONE(modifyaccount);
        } break;
        default: {
            return CheckProto_2(input, silent);
        }
    }

    return true;
}

auto CheckProto_4(const RPCCommand& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(4)
}

auto CheckProto_5(const RPCCommand& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(5)
}

auto CheckProto_6(const RPCCommand& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(6)
}

auto CheckProto_7(const RPCCommand& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(7)
}

auto CheckProto_8(const RPCCommand& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(8)
}

auto CheckProto_9(const RPCCommand& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(9)
}

auto CheckProto_10(const RPCCommand& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(10)
}

auto CheckProto_11(const RPCCommand& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(11)
}

auto CheckProto_12(const RPCCommand& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(12)
}

auto CheckProto_13(const RPCCommand& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(13)
}

auto CheckProto_14(const RPCCommand& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(14)
}

auto CheckProto_15(const RPCCommand& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(15)
}

auto CheckProto_16(const RPCCommand& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(16)
}

auto CheckProto_17(const RPCCommand& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(17)
}

auto CheckProto_18(const RPCCommand& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(18)
}

auto CheckProto_19(const RPCCommand& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(19)
}

auto CheckProto_20(const RPCCommand& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(20)
}
}  // namespace opentxs::proto
