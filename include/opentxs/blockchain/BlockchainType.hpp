// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_BLOCKCHAIN_BLOCKCHAINTYPE_HPP
#define OPENTXS_BLOCKCHAIN_BLOCKCHAINTYPE_HPP

#include "opentxs/blockchain/Types.hpp"  // IWYU pragma: associated

#include <limits>

namespace opentxs
{
namespace blockchain
{
enum class Type : TypeEnum {
    Unknown = 0,
    Bitcoin = 1,
    Bitcoin_testnet3 = 2,
    BitcoinCash = 3,
    BitcoinCash_testnet3 = 4,
    Ethereum_frontier = 5,
    Ethereum_ropsten = 6,
    Litecoin = 7,
    Litecoin_testnet4 = 8,
    PKT = 9,
    PKT_testnet = 10,
    UnitTest = std::numeric_limits<TypeEnum>::max(),
};
}  // namespace blockchain
}  // namespace opentxs
#endif
