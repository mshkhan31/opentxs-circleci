// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_BLOCKCHAIN_CRYPTO_HD_HPP
#define OPENTXS_BLOCKCHAIN_CRYPTO_HD_HPP

// IWYU pragma: no_include "opentxs/blockchain/crypto/HDProtocol.hpp"

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include "opentxs/blockchain/crypto/Deterministic.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"

namespace opentxs
{
namespace blockchain
{
namespace crypto
{
namespace internal
{
struct HD;
}  // namespace internal
}  // namespace crypto
}  // namespace blockchain
}  // namespace opentxs

namespace opentxs
{
namespace blockchain
{
namespace crypto
{
class OPENTXS_EXPORT HD : virtual public Deterministic
{
public:
    OPENTXS_NO_EXPORT virtual auto InternalHD() const noexcept
        -> internal::HD& = 0;
    virtual auto Name() const noexcept -> std::string = 0;
    virtual auto Standard() const noexcept -> HDProtocol = 0;

    OPENTXS_NO_EXPORT ~HD() override = default;

protected:
    HD() noexcept = default;

private:
    HD(const HD&) = delete;
    HD(HD&&) = delete;
    auto operator=(const HD&) -> HD& = delete;
    auto operator=(HD&&) -> HD& = delete;
};
}  // namespace crypto
}  // namespace blockchain
}  // namespace opentxs
#endif  // OPENTXS_BLOCKCHAIN_CRYPTO_HDCHAIN_HPP
