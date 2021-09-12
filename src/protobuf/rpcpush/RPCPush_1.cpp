// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/protobuf/verify/RPCPush.hpp"  // IWYU pragma: associated

#include "opentxs/protobuf/Basic.hpp"
#include "opentxs/protobuf/RPCEnums.pb.h"
#include "opentxs/protobuf/RPCPush.pb.h"
#include "opentxs/protobuf/verify/AccountEvent.hpp"  // IWYU pragma: keep
#include "opentxs/protobuf/verify/ContactEvent.hpp"  // IWYU pragma: keep
#include "opentxs/protobuf/verify/TaskComplete.hpp"  // IWYU pragma: keep
#include "opentxs/protobuf/verify/VerifyRPC.hpp"
#include "protobuf/Check.hpp"

#define PROTO_NAME "RPC push"

namespace opentxs
{
namespace proto
{
auto CheckProto_1(const RPCPush& input, const bool silent) -> bool
{
    CHECK_IDENTIFIER(id);

    switch (input.type()) {
        case RPCPUSH_ACCOUNT: {
            CHECK_SUBOBJECT(accountevent, RPCPushAllowedAccountEvent());
            CHECK_EXCLUDED(contactevent);
            CHECK_EXCLUDED(taskcomplete);
        } break;
        case RPCPUSH_CONTACT: {
            CHECK_EXCLUDED(accountevent);
            CHECK_SUBOBJECT(contactevent, RPCPushAllowedContactEvent());
            CHECK_EXCLUDED(taskcomplete);
        } break;
        case RPCPUSH_TASK: {
            CHECK_EXCLUDED(accountevent);
            CHECK_EXCLUDED(contactevent);
            CHECK_SUBOBJECT(taskcomplete, RPCPushAllowedTaskComplete());
        } break;
        case RPCPUSH_ERROR:
        default: {
            FAIL_2("Invalid type", input.type());
        }
    }

    return true;
}

auto CheckProto_2(const RPCPush& input, const bool silent) -> bool
{
    return CheckProto_1(input, silent);
}

auto CheckProto_3(const RPCPush& input, const bool silent) -> bool
{
    return CheckProto_1(input, silent);
}

auto CheckProto_4(const RPCPush& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(4)
}

auto CheckProto_5(const RPCPush& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(5)
}

auto CheckProto_6(const RPCPush& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(6)
}

auto CheckProto_7(const RPCPush& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(7)
}

auto CheckProto_8(const RPCPush& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(8)
}

auto CheckProto_9(const RPCPush& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(9)
}

auto CheckProto_10(const RPCPush& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(10)
}

auto CheckProto_11(const RPCPush& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(11)
}

auto CheckProto_12(const RPCPush& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(12)
}

auto CheckProto_13(const RPCPush& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(13)
}

auto CheckProto_14(const RPCPush& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(14)
}

auto CheckProto_15(const RPCPush& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(15)
}

auto CheckProto_16(const RPCPush& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(16)
}

auto CheckProto_17(const RPCPush& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(17)
}

auto CheckProto_18(const RPCPush& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(18)
}

auto CheckProto_19(const RPCPush& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(19)
}

auto CheckProto_20(const RPCPush& input, const bool silent) -> bool
{
    UNDEFINED_VERSION(20)
}
}  // namespace proto
}  // namespace opentxs
