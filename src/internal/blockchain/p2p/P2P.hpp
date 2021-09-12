// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <boost/asio.hpp>
#include <cstdint>
#include <future>
#include <set>

#include "core/StateMachine.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/p2p/Address.hpp"
#include "opentxs/blockchain/p2p/Peer.hpp"
#include "opentxs/core/Identifier.hpp"

namespace ba = boost::asio;
namespace ip = ba::ip;
using tcp = ip::tcp;

namespace opentxs::blockchain::p2p::internal
{
struct Address : virtual public p2p::Address {
    virtual auto clone_internal() const noexcept
        -> std::unique_ptr<Address> = 0;
    virtual auto Incoming() const noexcept -> bool = 0;
    virtual auto PreviousLastConnected() const noexcept -> Time = 0;
    virtual auto PreviousServices() const noexcept -> std::set<Service> = 0;

    ~Address() override = default;
};

struct Peer : virtual public p2p::Peer {
    virtual auto AddressID() const noexcept -> OTIdentifier = 0;
    virtual auto Shutdown() noexcept -> std::shared_future<void> = 0;

    virtual ~Peer() override = default;
};
}  // namespace opentxs::blockchain::p2p::internal

namespace opentxs::factory
{
auto BlockchainAddress(
    const api::Core& api,
    const blockchain::p2p::Protocol protocol,
    const blockchain::p2p::Network network,
    const Data& bytes,
    const std::uint16_t port,
    const blockchain::Type chain,
    const Time lastConnected,
    const std::set<blockchain::p2p::Service>& services,
    const bool incoming) noexcept
    -> std::unique_ptr<blockchain::p2p::internal::Address>;
auto BlockchainAddress(
    const api::Core& api,
    const proto::BlockchainPeerAddress serialized) noexcept
    -> std::unique_ptr<blockchain::p2p::internal::Address>;
}  // namespace opentxs::factory
