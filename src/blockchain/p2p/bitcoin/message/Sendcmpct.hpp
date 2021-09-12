// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <boost/endian/buffers.hpp>
#include <boost/endian/conversion.hpp>
#include <cstdint>
#include <memory>
#include <set>

#include "blockchain/p2p/bitcoin/Message.hpp"
#include "internal/blockchain/p2p/bitcoin/Bitcoin.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/core/Data.hpp"

namespace opentxs
{
namespace api
{
class Core;
}  // namespace api

namespace blockchain
{
namespace p2p
{
namespace bitcoin
{
class Header;
}  // namespace bitcoin
}  // namespace p2p
}  // namespace blockchain
}  // namespace opentxs

namespace be = boost::endian;

namespace opentxs::blockchain::p2p::bitcoin::message
{
class Sendcmpct final : public implementation::Message
{
public:
    struct Raw {
        be::little_int8_buf_t announce_;
        be::little_uint64_buf_t version_;

        Raw(bool announce, std::uint64_t version) noexcept;
        Raw() noexcept;
    };

    auto announce() const noexcept -> bool { return announce_; }
    auto version() const noexcept -> std::uint64_t { return version_; }

    Sendcmpct(
        const api::Core& api,
        const blockchain::Type network,
        const bool announce,
        const std::uint64_t version) noexcept;
    Sendcmpct(
        const api::Core& api,
        std::unique_ptr<Header> header,
        const bool announce,
        const std::uint64_t version) noexcept(false);

    ~Sendcmpct() final = default;

private:
    bool announce_{};
    std::uint64_t version_{};

    auto payload() const noexcept -> OTData final;

    Sendcmpct(const Sendcmpct&) = delete;
    Sendcmpct(Sendcmpct&&) = delete;
    auto operator=(const Sendcmpct&) -> Sendcmpct& = delete;
    auto operator=(Sendcmpct&&) -> Sendcmpct& = delete;
};
}  // namespace opentxs::blockchain::p2p::bitcoin::message
