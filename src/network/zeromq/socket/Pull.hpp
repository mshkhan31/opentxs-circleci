// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <map>
#include <ostream>
#include <vector>

#include "network/zeromq/curve/Server.hpp"
#include "network/zeromq/socket/Receiver.hpp"
#include "network/zeromq/socket/Socket.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/network/zeromq/socket/Pull.hpp"
#include "opentxs/network/zeromq/socket/Socket.hpp"

namespace opentxs
{
namespace network
{
namespace zeromq
{
namespace socket
{
class Pull;
}  // namespace socket

class Context;
class ListenCallback;
class Message;
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs

namespace opentxs::network::zeromq::socket::implementation
{
class Pull final : public Receiver<zeromq::socket::Pull>,
                   public zeromq::curve::implementation::Server
{
public:
    Pull(
        const zeromq::Context& context,
        const Socket::Direction direction,
        const zeromq::ListenCallback& callback,
        const bool startThread) noexcept;
    Pull(
        const zeromq::Context& context,
        const Socket::Direction direction,
        const zeromq::ListenCallback& callback) noexcept;
    Pull(
        const zeromq::Context& context,
        const Socket::Direction direction) noexcept;

    ~Pull() final;

private:
    const ListenCallback& callback_;

    auto clone() const noexcept -> Pull* final;
    auto have_callback() const noexcept -> bool final;

    void process_incoming(const Lock& lock, Message& message) noexcept final;

    Pull() = delete;
    Pull(const Pull&) = delete;
    Pull(Pull&&) = delete;
    auto operator=(const Pull&) -> Pull& = delete;
    auto operator=(Pull&&) -> Pull& = delete;
};
}  // namespace opentxs::network::zeromq::socket::implementation
