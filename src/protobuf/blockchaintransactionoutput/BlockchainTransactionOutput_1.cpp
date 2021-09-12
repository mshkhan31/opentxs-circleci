// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/protobuf/verify/BlockchainTransactionOutput.hpp"  // IWYU pragma: associated

#include "opentxs/protobuf/Basic.hpp"
#include "opentxs/protobuf/BlockchainTransactionOutput.pb.h"
#include "opentxs/protobuf/verify/BlockchainWalletKey.hpp"  // IWYU pragma: keep
#include "opentxs/protobuf/verify/VerifyBlockchain.hpp"
#include "protobuf/Check.hpp"

#define PROTO_NAME "blockchain transaction output"

namespace opentxs
{
namespace proto
{
auto CheckProto_1(const BlockchainTransactionOutput& input, const bool silent)
    -> bool
{
    CHECK_SCRIPT(script);
    OPTIONAL_SUBOBJECTS(
        key, BlockchainTransactionOutputAllowedBlockchainWalletKey());
    OPTIONAL_IDENTIFIER(confirmedspend);
    OPTIONAL_IDENTIFIERS(orphanedspend);

    return true;
}

auto CheckProto_2(const BlockchainTransactionOutput& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(2)
}

auto CheckProto_3(const BlockchainTransactionOutput& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(3)
}

auto CheckProto_4(const BlockchainTransactionOutput& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(4)
}

auto CheckProto_5(const BlockchainTransactionOutput& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(5)
}

auto CheckProto_6(const BlockchainTransactionOutput& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(6)
}

auto CheckProto_7(const BlockchainTransactionOutput& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(7)
}

auto CheckProto_8(const BlockchainTransactionOutput& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(8)
}

auto CheckProto_9(const BlockchainTransactionOutput& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(9)
}

auto CheckProto_10(const BlockchainTransactionOutput& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(10)
}

auto CheckProto_11(const BlockchainTransactionOutput& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(11)
}

auto CheckProto_12(const BlockchainTransactionOutput& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(12)
}

auto CheckProto_13(const BlockchainTransactionOutput& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(13)
}

auto CheckProto_14(const BlockchainTransactionOutput& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(14)
}

auto CheckProto_15(const BlockchainTransactionOutput& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(15)
}

auto CheckProto_16(const BlockchainTransactionOutput& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(16)
}

auto CheckProto_17(const BlockchainTransactionOutput& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(17)
}

auto CheckProto_18(const BlockchainTransactionOutput& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(18)
}

auto CheckProto_19(const BlockchainTransactionOutput& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(19)
}

auto CheckProto_20(const BlockchainTransactionOutput& input, const bool silent)
    -> bool
{
    UNDEFINED_VERSION(20)
}
}  // namespace proto
}  // namespace opentxs
