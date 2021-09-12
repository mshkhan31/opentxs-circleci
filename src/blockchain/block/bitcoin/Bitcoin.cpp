// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"    // IWYU pragma: associated
#include "1_Internal.hpp"  // IWYU pragma: associated
#include "internal/blockchain/block/bitcoin/Bitcoin.hpp"  // IWYU pragma: associated

#include <boost/endian/buffers.hpp>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iterator>
#include <limits>

#include "opentxs/core/Log.hpp"

namespace be = boost::endian;

namespace opentxs::blockchain::block::bitcoin::internal
{
auto DecodeBip34(const ReadView coinbase) noexcept -> block::Height
{
    static constexpr auto null = block::Height{-1};

    if (false == valid(coinbase)) { return null; }

    auto* i = reinterpret_cast<const std::byte*>(coinbase.data());
    const auto size = std::to_integer<std::uint8_t>(*i);
    std::advance(i, 1);
    auto buf = be::little_int64_buf_t{0};

    if ((size + 1u) > coinbase.size()) { return null; }
    if (size > sizeof(buf)) { return null; }

    std::memcpy(reinterpret_cast<std::byte*>(&buf), i, size);

    return buf.value();
}

auto EncodeBip34(block::Height height) noexcept -> Space
{
    if (std::numeric_limits<std::int8_t>::max() >= height) {
        auto buf = be::little_int8_buf_t(static_cast<std::int8_t>(height));
        auto out = space(sizeof(buf) + 1u);
        out.at(0) = std::byte{0x1};
        std::memcpy(std::next(out.data()), &buf, sizeof(buf));
        static_assert(sizeof(buf) == 1u);

        return out;
    } else if (std::numeric_limits<std::int16_t>::max() >= height) {
        auto buf = be::little_int16_buf_t(static_cast<std::int16_t>(height));
        auto out = space(sizeof(buf) + 1u);
        out.at(0) = std::byte{0x2};
        std::memcpy(std::next(out.data()), &buf, sizeof(buf));
        static_assert(sizeof(buf) == 2u);

        return out;
    } else if (8388607 >= height) {
        auto buf = be::little_int32_buf_t(static_cast<std::int32_t>(height));
        auto out = space(sizeof(buf) + 1u);
        out.at(0) = std::byte{0x3};
        std::memcpy(std::next(out.data()), &buf, 3u);
        static_assert(sizeof(buf) == 4u);

        return out;
    } else {
        OT_ASSERT(std::numeric_limits<std::int32_t>::max() >= height);

        auto buf = be::little_int32_buf_t(static_cast<std::int32_t>(height));
        auto out = space(sizeof(buf) + 1u);
        out.at(0) = std::byte{0x4};
        std::memcpy(std::next(out.data()), &buf, sizeof(buf));
        static_assert(sizeof(buf) == 4u);

        return out;
    }
}
}  // namespace opentxs::blockchain::block::bitcoin::internal
