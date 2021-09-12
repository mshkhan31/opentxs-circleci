// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CRYPTO_KEY_ED25519_HPP
#define OPENTXS_CRYPTO_KEY_ED25519_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include "opentxs/crypto/key/HD.hpp"

namespace opentxs
{
namespace crypto
{
namespace key
{
class OPENTXS_EXPORT Ed25519 : virtual public HD
{
public:
    ~Ed25519() override = default;

protected:
    Ed25519() = default;

private:
    Ed25519(const Ed25519&) = delete;
    Ed25519(Ed25519&&) = delete;
    auto operator=(const Ed25519&) -> Ed25519& = delete;
    auto operator=(Ed25519&&) -> Ed25519& = delete;
};
}  // namespace key
}  // namespace crypto
}  // namespace opentxs
#endif
