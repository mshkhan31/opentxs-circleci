// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <functional>
#include <string>
#include <vector>

#include "opentxs/Bytes.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/network/asio/Endpoint.hpp"
#include "opentxs/network/asio/Socket.hpp"
#include "opentxs/network/blockchain/sync/State.hpp"
#include "util/Work.hpp"

namespace boost
{
namespace asio
{
class io_context;
}  // namespace asio
}  // namespace boost

namespace opentxs
{
namespace api
{
namespace client
{
namespace internal
{
struct Blockchain;
}  // namespace internal
}  // namespace client

class Legacy;
}  // namespace api

namespace blockchain
{
namespace database
{
namespace common
{
class Database;
}  // namespace common
}  // namespace database
}  // namespace blockchain

namespace identifier
{
class Nym;
}  // namespace identifier

namespace network
{
namespace zeromq
{
namespace socket
{
class Publish;
}  // namespace socket
}  // namespace zeromq
}  // namespace network

class Identifier;
class Options;
}  // namespace opentxs

namespace opentxs::api::network::internal
{
struct Asio {
    using Endpoint = opentxs::network::asio::Endpoint::Imp;
    using Socket = opentxs::network::asio::Socket::Imp;
    using Callback = std::function<void()>;

    virtual auto Connect(const ReadView id, Socket& socket) noexcept
        -> bool = 0;
    virtual auto IOContext() noexcept -> boost::asio::io_context& = 0;
    virtual auto PostIO(Callback cb) noexcept -> bool = 0;
    virtual auto PostCPU(Callback cb) noexcept -> bool = 0;
    virtual auto Receive(
        const ReadView id,
        const OTZMQWorkType type,
        const std::size_t bytes,
        Socket& socket) noexcept -> bool = 0;

    virtual ~Asio() = default;

protected:
    Asio() = default;

private:
    Asio(const Asio&) = delete;
    Asio(Asio&&) = delete;
    Asio& operator=(const Asio&) = delete;
    Asio& operator=(Asio&&) = delete;
};

struct Blockchain {
    virtual auto BlockQueueUpdate() const noexcept
        -> const opentxs::network::zeromq::socket::Publish& = 0;
    virtual auto Database() const noexcept
        -> const opentxs::blockchain::database::common::Database& = 0;
    virtual auto FilterUpdate() const noexcept
        -> const opentxs::network::zeromq::socket::Publish& = 0;
    using SyncData = std::vector<opentxs::network::blockchain::sync::State>;
    virtual auto Hello() const noexcept -> SyncData = 0;
    virtual auto IsEnabled(const opentxs::blockchain::Type chain) const noexcept
        -> bool = 0;
    virtual auto Mempool() const noexcept
        -> const opentxs::network::zeromq::socket::Publish& = 0;
    virtual auto PeerUpdate() const noexcept
        -> const opentxs::network::zeromq::socket::Publish& = 0;
    virtual auto Reorg() const noexcept
        -> const opentxs::network::zeromq::socket::Publish& = 0;
    virtual auto ReportProgress(
        const opentxs::blockchain::Type chain,
        const opentxs::blockchain::block::Height current,
        const opentxs::blockchain::block::Height target) const noexcept
        -> void = 0;
    virtual auto RestoreNetworks() const noexcept -> void = 0;
    virtual auto SyncEndpoint() const noexcept -> const std::string& = 0;
    virtual auto UpdatePeer(
        const opentxs::blockchain::Type chain,
        const std::string& address) const noexcept -> void = 0;

    virtual auto Init(
        const api::client::internal::Blockchain& crypto,
        const api::Legacy& legacy,
        const std::string& dataFolder,
        const Options& args) noexcept -> void = 0;
    virtual auto Shutdown() noexcept -> void = 0;

    virtual ~Blockchain() = default;

protected:
    Blockchain() = default;

private:
    Blockchain(const Blockchain&) = delete;
    Blockchain(Blockchain&&) = delete;
    Blockchain& operator=(const Blockchain&) = delete;
    Blockchain& operator=(Blockchain&&) = delete;
};
}  // namespace opentxs::api::network::internal
