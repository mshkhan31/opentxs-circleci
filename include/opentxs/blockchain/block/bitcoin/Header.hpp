// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_BLOCKCHAIN_BLOCK_BITCOIN_HEADER_HPP
#define OPENTXS_BLOCKCHAIN_BLOCK_BITCOIN_HEADER_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include "opentxs/Types.hpp"
#include "opentxs/blockchain/block/Header.hpp"

namespace opentxs
{
namespace blockchain
{
namespace block
{
namespace bitcoin
{
class OPENTXS_EXPORT Header : virtual public block::Header
{
public:
    virtual auto MerkleRoot() const noexcept -> const block::Hash& = 0;
    virtual auto Encode() const noexcept -> OTData = 0;
    virtual auto Nonce() const noexcept -> std::uint32_t = 0;
    virtual auto nBits() const noexcept -> std::uint32_t = 0;
    virtual auto Timestamp() const noexcept -> Time = 0;
    virtual auto Version() const noexcept -> std::uint32_t = 0;

    ~Header() override = default;

protected:
    Header() noexcept = default;

private:
    Header(const Header&) = delete;
    Header(Header&&) = delete;
    auto operator=(const Header&) -> Header& = delete;
    auto operator=(Header&&) -> Header& = delete;
};
}  // namespace bitcoin
}  // namespace block
}  // namespace blockchain
}  // namespace opentxs
#endif
