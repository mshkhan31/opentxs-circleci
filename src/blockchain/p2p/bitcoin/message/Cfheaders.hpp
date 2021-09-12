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
#include "opentxs/Bytes.hpp"
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

class Factory;
}  // namespace opentxs

namespace opentxs::blockchain::p2p::bitcoin::message::implementation
{
class Cfheaders final : public internal::Cfheaders,
                        public implementation::Message
{
public:
    using BitcoinFormat = FilterPrefixChained;

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
    auto Previous() const noexcept -> const filter::Header& final
    {
        return previous_;
    }
    auto size() const noexcept -> std::size_t final { return payload_.size(); }
    auto Stop() const noexcept -> const block::Hash& final { return stop_; }
    auto Type() const noexcept -> filter::Type final { return type_; }

    Cfheaders(
        const api::Core& api,
        const blockchain::Type network,
        const filter::Type type,
        const block::Hash& stop,
        const ReadView previousHeader,
        const std::vector<filter::pHash>& headers) noexcept;
    Cfheaders(
        const api::Core& api,
        std::unique_ptr<Header> header,
        const filter::Type type,
        const block::Hash& stop,
        const filter::Header& previous,
        const std::vector<filter::pHash>& headers) noexcept;

    ~Cfheaders() final = default;

private:
    const filter::Type type_;
    const block::pHash stop_;
    const filter::pHeader previous_;
    const std::vector<filter::pHash> payload_;

    auto payload() const noexcept -> OTData final;

    Cfheaders(const Cfheaders&) = delete;
    Cfheaders(Cfheaders&&) = delete;
    auto operator=(const Cfheaders&) -> Cfheaders& = delete;
    auto operator=(Cfheaders&&) -> Cfheaders& = delete;
};
}  // namespace opentxs::blockchain::p2p::bitcoin::message::implementation
