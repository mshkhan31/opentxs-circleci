// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>
#include <iosfwd>
#include <memory>
#include <vector>

#include "blockchain/p2p/bitcoin/Message.hpp"
#include "internal/blockchain/p2p/bitcoin/message/Message.hpp"
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

namespace opentxs::blockchain::p2p::bitcoin::message::implementation
{
class Inv final : public internal::Inv, public implementation::Message
{
public:
    auto at(const std::size_t position) const noexcept(false)
        -> const value_type& final
    {
        return payload_.at(position);
    }
    auto begin() const noexcept -> const_iterator final
    {
        return const_iterator(this, 0);
    }
    auto end() const noexcept -> const_iterator final
    {
        return const_iterator(this, payload_.size());
    }
    auto size() const noexcept -> std::size_t final { return payload_.size(); }

    Inv(const api::Core& api,
        const blockchain::Type network,
        std::vector<value_type>&& payload) noexcept;
    Inv(const api::Core& api,
        std::unique_ptr<Header> header,
        std::vector<value_type>&& payload) noexcept;

    ~Inv() final = default;

private:
    const std::vector<value_type> payload_;

    auto payload() const noexcept -> OTData final;

    Inv(const Inv&) = delete;
    Inv(Inv&&) = delete;
    auto operator=(const Inv&) -> Inv& = delete;
    auto operator=(Inv&&) -> Inv& = delete;
};
}  // namespace opentxs::blockchain::p2p::bitcoin::message::implementation
