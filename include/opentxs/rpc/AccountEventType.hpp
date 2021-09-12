// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_RPC_ACCOUNT_EVENT_TYPE_HPP
#define OPENTXS_RPC_ACCOUNT_EVENT_TYPE_HPP

#include "opentxs/rpc/Types.hpp"  // IWYU pragma: associated

#include <limits>

namespace opentxs
{
namespace rpc
{
enum class AccountEventType : TypeEnum {
    error = 0,
    incoming_cheque = 1,
    outgoing_cheque = 2,
    incoming_transfer = 3,
    outgoing_transfer = 4,
    incoming_invoice = 5,
    outgoing_invoice = 6,
    incoming_voucher = 7,
    outgoing_voucher = 8,
    incoming_blockchain = 9,
    outgoing_blockchain = 10,
};
}  // namespace rpc
}  // namespace opentxs
#endif
