// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_BLOCKCHAIN_BLOCKCHAIN_HPP
#define OPENTXS_BLOCKCHAIN_BLOCKCHAIN_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <cstdint>
#include <optional>  // IWYU pragma: keep
#include <set>
#include <tuple>
#include <vector>

#include "opentxs/Bytes.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/p2p/Types.hpp"
#include "opentxs/core/Data.hpp"

namespace opentxs
{
namespace api
{
class Core;
}  // namespace api
}  // namespace opentxs

namespace opentxs
{
namespace blockchain
{
using PatternID = std::uint64_t;
using Hash = Data;
using pHash = OTData;

OPENTXS_EXPORT auto BlockHash(
    const api::Core& api,
    const Type chain,
    const ReadView input,
    const AllocateOutput output) noexcept -> bool;
OPENTXS_EXPORT auto DefinedChains() noexcept -> const std::set<Type>&;
OPENTXS_EXPORT auto DisplayString(const Type type) noexcept -> std::string;
OPENTXS_EXPORT auto FilterHash(
    const api::Core& api,
    const Type chain,
    const ReadView input,
    const AllocateOutput output) noexcept -> bool;
OPENTXS_EXPORT auto HashToNumber(const api::Core& api, ReadView hex) noexcept
    -> std::string;
OPENTXS_EXPORT auto HashToNumber(const Hash& hash) noexcept -> std::string;
OPENTXS_EXPORT auto HasSegwit(const Type type) noexcept -> bool;
OPENTXS_EXPORT auto IsTestnet(const Type type) noexcept -> bool;
OPENTXS_EXPORT auto MerkleHash(
    const api::Core& api,
    const Type chain,
    const ReadView input,
    const AllocateOutput output) noexcept -> bool;
OPENTXS_EXPORT auto NumberToHash(const api::Core& api, ReadView hex) noexcept
    -> pHash;
OPENTXS_EXPORT auto P2PMessageHash(
    const api::Core& api,
    const Type chain,
    const ReadView input,
    const AllocateOutput output) noexcept -> bool;
OPENTXS_EXPORT auto ProofOfWorkHash(
    const api::Core& api,
    const Type chain,
    const ReadView input,
    const AllocateOutput output) noexcept -> bool;
OPENTXS_EXPORT auto PubkeyHash(
    const api::Core& api,
    const Type chain,
    const ReadView input,
    const AllocateOutput output) noexcept -> bool;
OPENTXS_EXPORT auto ScriptHash(
    const api::Core& api,
    const Type chain,
    const ReadView input,
    const AllocateOutput output) noexcept -> bool;
OPENTXS_EXPORT auto ScriptHashSegwit(
    const api::Core& api,
    const Type chain,
    const ReadView input,
    const AllocateOutput output) noexcept -> bool;
OPENTXS_EXPORT auto SupportedChains() noexcept -> const std::set<Type>&;
OPENTXS_EXPORT auto TickerSymbol(const Type type) noexcept -> std::string;
OPENTXS_EXPORT auto TransactionHash(
    const api::Core& api,
    const Type chain,
    const ReadView input,
    const AllocateOutput output) noexcept -> bool;

namespace block
{
namespace bitcoin
{
enum class OP : std::uint8_t {
    ZERO = 0,
    PUSHDATA_1 = 1,
    PUSHDATA_2 = 2,
    PUSHDATA_3 = 3,
    PUSHDATA_4 = 4,
    PUSHDATA_5 = 5,
    PUSHDATA_6 = 6,
    PUSHDATA_7 = 7,
    PUSHDATA_8 = 8,
    PUSHDATA_9 = 9,
    PUSHDATA_10 = 10,
    PUSHDATA_11 = 11,
    PUSHDATA_12 = 12,
    PUSHDATA_13 = 13,
    PUSHDATA_14 = 14,
    PUSHDATA_15 = 15,
    PUSHDATA_16 = 16,
    PUSHDATA_17 = 17,
    PUSHDATA_18 = 18,
    PUSHDATA_19 = 19,
    PUSHDATA_20 = 20,
    PUSHDATA_21 = 21,
    PUSHDATA_22 = 22,
    PUSHDATA_23 = 23,
    PUSHDATA_24 = 24,
    PUSHDATA_25 = 25,
    PUSHDATA_26 = 26,
    PUSHDATA_27 = 27,
    PUSHDATA_28 = 28,
    PUSHDATA_29 = 29,
    PUSHDATA_30 = 30,
    PUSHDATA_31 = 31,
    PUSHDATA_32 = 32,
    PUSHDATA_33 = 33,
    PUSHDATA_34 = 34,
    PUSHDATA_35 = 35,
    PUSHDATA_36 = 36,
    PUSHDATA_37 = 37,
    PUSHDATA_38 = 38,
    PUSHDATA_39 = 39,
    PUSHDATA_40 = 40,
    PUSHDATA_41 = 41,
    PUSHDATA_42 = 42,
    PUSHDATA_43 = 43,
    PUSHDATA_44 = 44,
    PUSHDATA_45 = 45,
    PUSHDATA_46 = 46,
    PUSHDATA_47 = 47,
    PUSHDATA_48 = 48,
    PUSHDATA_49 = 49,
    PUSHDATA_50 = 50,
    PUSHDATA_51 = 51,
    PUSHDATA_52 = 52,
    PUSHDATA_53 = 53,
    PUSHDATA_54 = 54,
    PUSHDATA_55 = 55,
    PUSHDATA_56 = 56,
    PUSHDATA_57 = 57,
    PUSHDATA_58 = 58,
    PUSHDATA_59 = 59,
    PUSHDATA_60 = 60,
    PUSHDATA_61 = 61,
    PUSHDATA_62 = 62,
    PUSHDATA_63 = 63,
    PUSHDATA_64 = 64,
    PUSHDATA_65 = 65,
    PUSHDATA_66 = 66,
    PUSHDATA_67 = 67,
    PUSHDATA_68 = 68,
    PUSHDATA_69 = 69,
    PUSHDATA_70 = 70,
    PUSHDATA_71 = 71,
    PUSHDATA_72 = 72,
    PUSHDATA_73 = 73,
    PUSHDATA_74 = 74,
    PUSHDATA_75 = 75,
    PUSHDATA1 = 76,
    PUSHDATA2 = 77,
    PUSHDATA4 = 78,
    ONE_NEGATE = 79,
    RESERVED = 80,
    ONE = 81,
    TWO = 82,
    THREE = 83,
    FOUR = 84,
    FIVE = 85,
    SIX = 86,
    SEVEN = 87,
    EIGHT = 88,
    NINE = 89,
    TEN = 90,
    ELEVEN = 91,
    TWELVE = 92,
    THIRTEEN = 93,
    FOURTEEN = 94,
    FIFTEEN = 95,
    SIXTEEN = 96,
    NOP = 97,
    VER = 98,
    IF = 99,
    NOTIF = 100,
    VERIF = 101,
    VERNOTIF = 102,
    ELSE = 103,
    ENDIF = 104,
    VERIFY = 105,
    RETURN = 106,
    TOALTSTACK = 107,
    FROMALTSTACK = 108,
    TWO_DROP = 109,
    TWO_DUP = 110,
    THREE_DUP = 111,
    TWO_OVER = 112,
    TWO_ROT = 113,
    TWO_SWAP = 114,
    IFDUP = 115,
    DEPTH = 116,
    DROP = 117,
    DUP = 118,
    NIP = 119,
    OVER = 120,
    PICK = 121,
    ROLL = 122,
    ROT = 123,
    SWAP = 124,
    TUCK = 125,
    CAT = 126,
    SUBSTR = 127,
    LEFT = 128,
    RIGHT = 129,
    SIZE = 130,
    INVERT = 131,
    AND = 132,
    OR = 133,
    XOR = 134,
    EQUAL = 135,
    EQUALVERIFY = 136,
    RESERVED1 = 137,
    RESERVED2 = 138,
    ONE_ADD = 139,
    ONE_SUB = 140,
    TWO_MUL = 141,
    TWO_DIV = 142,
    NEGATE = 143,
    ABS = 144,
    NOT = 145,
    ZERO_NOTEQUAL = 146,
    ADD = 147,
    SUB = 148,
    MUL = 149,
    DIV = 150,
    MOD = 151,
    LSHIFT = 152,
    RSHIFT = 153,
    BOOLAND = 154,
    BOOLOR = 155,
    NUMEQUAL = 156,
    NUMEQUALVERIFY = 157,
    NUMNOTEQUAL = 158,
    LESSTHAN = 159,
    GREATERTHAN = 160,
    LESSTHANOREQUAL = 161,
    GREATERTHANOREQUAL = 162,
    MIN = 163,
    MAX = 164,
    WITHIN = 165,
    RIPEMD160 = 166,
    SHA1 = 167,
    SHA256 = 168,
    HASH160 = 169,
    HASH256 = 170,
    CODESEPARATOR = 171,
    CHECKSIG = 172,
    CHECKSIGVERIFY = 173,
    CHECKMULTISIG = 174,
    CHECKMULTISIGVERIFY = 175,
    NOP1 = 176,
    NOP2 = 177,
    NOP3 = 178,
    NOP4 = 179,
    NOP5 = 180,
    NOP6 = 181,
    NOP7 = 182,
    NOP8 = 183,
    NOP9 = 184,
    NOP10 = 185,
    PUBKEYHASH = 253,
    PUBKEY = 254,
    INVALIDOPCODE = 255,
    False = ZERO,  // FALSE has a pre-existing macro on some platforms.
    True = ONE,
    CHECKLOCKTIMEVERIFY = NOP2,
    CHECKSEQUENCEVERIFY = NOP3,
};

struct ScriptElement {
    using ScriptData = Space;
    using ScriptPushBytes = Space;
    using InvalidOpcode = std::byte;

    OP opcode_{};
    std::optional<InvalidOpcode> invalid_{};
    std::optional<ScriptPushBytes> bytes_{};
    std::optional<ScriptData> data_{};
};
using ScriptElements = std::vector<ScriptElement>;
}  // namespace bitcoin

using Height = std::int64_t;
using Hash = blockchain::Hash;
using pHash = blockchain::pHash;
using Position = std::pair<Height, pHash>;
using Txid = blockchain::Hash;
using pTxid = blockchain::pHash;

/// Use this to test for a chain reorg. lhs is the current position and rhs is
/// the new position.
auto operator>(const Position& lhs, const Position& rhs) noexcept -> bool;

OPENTXS_EXPORT auto BlankHash() noexcept -> pHash;
}  // namespace block

namespace filter
{
using Hash = blockchain::Hash;
using pHash = blockchain::pHash;
using Header = Hash;
using pHeader = pHash;
}  // namespace filter

using SendOutcome = std::pair<SendResult, block::pTxid>;
}  // namespace blockchain
}  // namespace opentxs
#endif
