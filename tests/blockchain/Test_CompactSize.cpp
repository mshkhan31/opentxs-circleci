// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <cstdint>
#include <map>
#include <string>
#include <vector>

#include "opentxs/Pimpl.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/iterator/Bidirectional.hpp"
#include "opentxs/network/blockchain/bitcoin/CompactSize.hpp"

namespace ot = opentxs;

namespace ottest
{
// clang-format off
const std::map<std::uint64_t, std::string> vector_1_{
    {0u,                    "0x00"},
    {252u,                  "0xfc"},
    {253u,                  "0xfdfd00"},
    {65535u,                "0xfdffff"},
    {65536u,                "0xfe00000100"},
    {4294967295u,           "0xfeffffffff"},
    {4294967296u,           "0xff0000000001000000"},
    {18446744073709551615u, "0xffffffffffffffffff"},
};
const std::map<std::string, std::uint64_t> vector_2_{
    {"0x00",                 0u},
    {"0xfc",                 252u},
};
const std::map<std::string, std::uint64_t> vector_3_{
    {"0xfd0000",             0u},
    {"0xfdfc00",             252u},
    {"0xfdfd00",             253u},
    {"0xfdffff",             65535u},
};
const std::map<std::string, std::uint64_t> vector_4_{
    {"0xfe00000000",         0u},
    {"0xfefc000000",         252u},
    {"0xfefd000000",         253u},
    {"0xfeffff0000",         65535u},
    {"0xfe00000100",         65536u},
    {"0xfeffffffff",         4294967295u},
};
const std::map<std::string, std::uint64_t> vector_5_{
    {"0xff0000000000000000", 0u},
    {"0xfffc00000000000000", 252u},
    {"0xfffd00000000000000", 253u},
    {"0xffffff000000000000", 65535u},
    {"0xff0000010000000000", 65536u},
    {"0xffffffffff00000000", 4294967295u},
    {"0xff0000000001000000", 4294967296u},
    {"0xffffffffffffffffff", 18446744073709551615u},
};
// clang-format on

using CompactSize = opentxs::network::blockchain::bitcoin::CompactSize;

CompactSize::Bytes decode_hex(const std::string& hex);
CompactSize::Bytes decode_hex(const std::string& hex)
{
    CompactSize::Bytes output{};
    auto bytes = ot::Data::Factory(hex, ot::Data::Mode::Hex);

    for (const auto& byte : bytes.get()) { output.emplace_back(byte); }

    return output;
}

TEST(Test_CompactSize, encode)
{
    for (const auto& [number, hex] : vector_1_) {
        CompactSize encoded(number);
        const auto expectedRaw = ot::Data::Factory(hex, ot::Data::Mode::Hex);
        const auto calculatedRaw = ot::Data::Factory(encoded.Encode());

        EXPECT_EQ(calculatedRaw, expectedRaw);
    }
}

TEST(Test_CompactSize, decode_one_byte)
{
    for (const auto& [hex, expected] : vector_2_) {
        auto raw = decode_hex(hex);
        const auto first = raw.at(0);

        EXPECT_EQ(CompactSize::CalculateSize(first), 0);

        CompactSize decoded;

        EXPECT_TRUE(decoded.Decode(raw));
        EXPECT_EQ(decoded.Value(), expected);
    }
}

TEST(Test_CompactSize, decode_three_bytes)
{
    for (const auto& [hex, expected] : vector_3_) {
        auto raw = decode_hex(hex);
        const auto first = raw.at(0);

        EXPECT_EQ(2, CompactSize::CalculateSize(first));

        CompactSize decoded;
        raw.erase(raw.begin());

        EXPECT_TRUE(decoded.Decode(raw));
        EXPECT_EQ(decoded.Value(), expected);
    }
}

TEST(Test_CompactSize, decode_five_bytes)
{
    for (const auto& [hex, expected] : vector_4_) {
        auto raw = decode_hex(hex);
        const auto first = raw.at(0);

        EXPECT_EQ(4, CompactSize::CalculateSize(first));

        CompactSize decoded;
        raw.erase(raw.begin());

        EXPECT_TRUE(decoded.Decode(raw));
        EXPECT_EQ(decoded.Value(), expected);
    }
}

TEST(Test_CompactSize, decode_nine_bytes)
{
    for (const auto& [hex, expected] : vector_5_) {
        auto raw = decode_hex(hex);
        const auto first = raw.at(0);

        EXPECT_EQ(8, CompactSize::CalculateSize(first));

        CompactSize decoded;
        raw.erase(raw.begin());

        EXPECT_TRUE(decoded.Decode(raw));
        EXPECT_EQ(decoded.Value(), expected);
    }
}
}  // namespace ottest
