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
#include "opentxs/network/zeromq/curve/Server.hpp"

namespace opentxs
{
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

class OTPassword;
class Secret;
}  // namespace opentxs

namespace opentxs::network::zeromq::curve::implementation
{
class Server : virtual public zeromq::curve::Server
{
public:
    auto SetDomain(const std::string& domain) const noexcept -> bool final;
    auto SetPrivateKey(const Secret& key) const noexcept -> bool final;
    auto SetPrivateKey(const std::string& z85) const noexcept -> bool final;

protected:
    auto set_private_key(const void* key, const std::size_t keySize)
        const noexcept -> bool;

    Server(zeromq::socket::implementation::Socket& socket) noexcept;

    ~Server() override = default;

private:
    zeromq::socket::implementation::Socket& parent_;

    Server() = delete;
    Server(const Server&) = delete;
    Server(Server&&) = delete;
    auto operator=(const Server&) -> Server& = delete;
    auto operator=(Server&&) -> Server& = delete;
};
}  // namespace opentxs::network::zeromq::curve::implementation
