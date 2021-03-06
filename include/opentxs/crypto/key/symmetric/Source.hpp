// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CRYPTO_KEY_SYMMETRIC_SOURCE_HPP
#define OPENTXS_CRYPTO_KEY_SYMMETRIC_SOURCE_HPP

#include "opentxs/crypto/key/Types.hpp"  // IWYU pragma: associated

#include <cstdint>

namespace opentxs
{
namespace crypto
{
namespace key
{
namespace symmetric
{

enum class Source : std::uint8_t {
    Error = 0,
    Raw = 1,
    ECDH = 2,
    Argon2i = 3,
    Argon2id = 4,
};

constexpr auto value(const Source in) noexcept
{
    return static_cast<std::uint8_t>(in);
}
}  // namespace symmetric
}  // namespace key
}  // namespace crypto
}  // namespace opentxs
#endif
