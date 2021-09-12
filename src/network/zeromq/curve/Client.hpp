// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>
#include <iosfwd>
#include <mutex>
#include <string>

#include "network/zeromq/socket/Socket.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/network/zeromq/curve/Client.hpp"

namespace opentxs
{
namespace contract
{
class Server;
}  // namespace contract

namespace network
{
namespace zeromq
{
namespace socket
{
namespace implementation
{
class Socket;
}  // namespace implementation
}  // namespace socket
}  // namespace zeromq
}  // namespace network

class Data;
}  // namespace opentxs

namespace opentxs::network::zeromq::curve::implementation
{
class Client : virtual public zeromq::curve::Client
{
public:
    auto SetKeysZ85(
        const std::string& serverPublic,
        const std::string& clientPrivate,
        const std::string& clientPublic) const noexcept -> bool final;
    auto SetServerPubkey(const contract::Server& contract) const noexcept
        -> bool final;
    auto SetServerPubkey(const Data& key) const noexcept -> bool final;

protected:
    auto set_public_key(const contract::Server& contract) const noexcept
        -> bool;
    auto set_public_key(const Data& key) const noexcept -> bool;

    Client(zeromq::socket::implementation::Socket& socket) noexcept;

    ~Client() override = default;

private:
    zeromq::socket::implementation::Socket& parent_;

    auto set_local_keys() const noexcept -> bool;
    auto set_local_keys(
        const std::string& privateKey,
        const std::string& publicKey) const noexcept -> bool;
    auto set_local_keys(
        const void* privateKey,
        const std::size_t privateKeySize,
        const void* publicKey,
        const std::size_t publicKeySize) const noexcept -> bool;
    auto set_remote_key(const void* key, const std::size_t size) const noexcept
        -> bool;

    Client() = delete;
    Client(const Client&) = delete;
    Client(Client&&) = delete;
    auto operator=(const Client&) -> Client& = delete;
    auto operator=(Client&&) -> Client& = delete;
};
}  // namespace opentxs::network::zeromq::curve::implementation
