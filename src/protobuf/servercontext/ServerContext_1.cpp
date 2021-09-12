// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/protobuf/verify/ServerContext.hpp"  // IWYU pragma: associated

#include "opentxs/protobuf/ServerContext.pb.h"
#include "protobuf/Check.hpp"

#define PROTO_NAME "server context"

namespace opentxs
{
namespace proto
{
auto CheckProto_1(const ServerContext& input, const bool silent) -> bool
{
    CHECK_IDENTIFIER(serverid);
    CHECK_EXCLUDED(revision);
    CHECK_EXCLUDED(adminpassword);
    CHECK_EXCLUDED(adminattempted);
    CHECK_EXCLUDED(adminsuccess);
    CHECK_EXCLUDED(state);
    CHECK_EXCLUDED(laststatus);
    CHECK_EXCLUDED(pending);

    return true;
}
}  // namespace proto
}  // namespace opentxs
