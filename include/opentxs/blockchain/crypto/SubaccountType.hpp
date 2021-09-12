// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_BLOCKCHAIN_CRYPTO_SUBACCOUNT_TYPE_HPP
#define OPENTXS_BLOCKCHAIN_CRYPTO_SUBACCOUNT_TYPE_HPP

#include "opentxs/blockchain/crypto/Types.hpp"  // IWYU pragma: associated

#include <limits>

namespace opentxs
{
namespace blockchain
{
namespace crypto
{
enum class SubaccountType : std::uint16_t {
    Error = 0,
    HD = 1,
    PaymentCode = 2,
    Imported = 3,
    Notification = std::numeric_limits<std::uint16_t>::max(),
};
}  // namespace crypto
}  // namespace blockchain
}  // namespace opentxs
#endif
