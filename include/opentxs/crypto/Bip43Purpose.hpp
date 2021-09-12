// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CRYPTO_BIP43PURPOSE_HPP
#define OPENTXS_CRYPTO_BIP43PURPOSE_HPP

#include "opentxs/crypto/Types.hpp"  // IWYU pragma: associated

namespace opentxs
{
enum class Bip43Purpose : Bip32Index {
    HDWALLET = 44,     // BIP-44
    PAYCODE = 47,      // BIP-47
    P2SH_P2WPKH = 49,  // BIP-49
    P2WPKH = 84,       // BIP-84
    FS = 0x4f544653,   // OTFS
    NYM = 0x4f544e4d   // OTNM
};
}  // namespace opentxs
#endif
