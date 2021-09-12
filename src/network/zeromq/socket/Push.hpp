// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "network/zeromq/curve/Client.hpp"
#include "network/zeromq/socket/Sender.hpp"
#include "network/zeromq/socket/Socket.hpp"
#include "opentxs/network/zeromq/socket/Push.hpp"
#include "opentxs/network/zeromq/socket/Socket.hpp"

namespace opentxs
{
namespace network
{
namespace zeromq
{
namespace socket
{
class Push;
}  // namespace socket

class Context;
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs

namespace opentxs::network::zeromq::socket::implementation
{
class Push final : public Sender<zeromq::socket::Push>,
                   public zeromq::curve::implementation::Client
{
public:
    Push(
        const zeromq::Context& context,
        const Socket::Direction direction) noexcept;

    ~Push() final;

private:
    auto clone() const noexcept -> Push* final
    {
        return new Push(context_, direction_);
    }

    Push() = delete;
    Push(const Push&) = delete;
    Push(Push&&) = delete;
    auto operator=(const Push&) -> Push& = delete;
    auto operator=(Push&&) -> Push& = delete;
};
}  // namespace opentxs::network::zeromq::socket::implementation
