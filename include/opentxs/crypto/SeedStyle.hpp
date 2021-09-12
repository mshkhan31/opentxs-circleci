// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CRYPTO_SEED_STYLE_HPP
#define OPENTXS_CRYPTO_SEED_STYLE_HPP

#include "opentxs/crypto/Types.hpp"  // IWYU pragma: associated

#include <cstdint>
#include <limits>

namespace opentxs
{
namespace crypto
{
enum class SeedStyle : std::uint8_t {
    BIP32 = 0,
    BIP39 = 1,
    PKT = 2,
    Error = std::numeric_limits<std::uint8_t>::max(),
};
}  // namespace crypto
}  // namespace opentxs
#endif
