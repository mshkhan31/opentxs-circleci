// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <memory>

#include "blockchain/p2p/bitcoin/Message.hpp"
#include "internal/blockchain/p2p/bitcoin/message/Message.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/FilterType.hpp"
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
class Getcfheaders final : public internal::Getcfheaders,
                           public implementation::Message
{
public:
    using BitcoinFormat = FilterRequest;

    auto Start() const noexcept -> block::Height final { return start_; }
    auto Stop() const noexcept -> const filter::Hash& final { return stop_; }
    auto Type() const noexcept -> filter::Type final { return type_; }

    Getcfheaders(
        const api::Core& api,
        const blockchain::Type network,
        const filter::Type type,
        const block::Height start,
        const filter::Hash& stop) noexcept;
    Getcfheaders(
        const api::Core& api,
        std::unique_ptr<Header> header,
        const filter::Type type,
        const block::Height start,
        const filter::Hash& stop) noexcept;

    ~Getcfheaders() final = default;

private:
    const filter::Type type_;
    const block::Height start_;
    const filter::pHash stop_;

    auto payload() const noexcept -> OTData final;

    Getcfheaders(const Getcfheaders&) = delete;
    Getcfheaders(Getcfheaders&&) = delete;
    auto operator=(const Getcfheaders&) -> Getcfheaders& = delete;
    auto operator=(Getcfheaders&&) -> Getcfheaders& = delete;
};
}  // namespace opentxs::blockchain::p2p::bitcoin::message::implementation
