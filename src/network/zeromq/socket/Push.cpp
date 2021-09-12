// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                    // IWYU pragma: associated
#include "1_Internal.hpp"                  // IWYU pragma: associated
#include "network/zeromq/socket/Push.hpp"  // IWYU pragma: associated

#include <memory>

#include "internal/network/zeromq/socket/Socket.hpp"
#include "network/zeromq/curve/Client.hpp"
#include "network/zeromq/socket/Sender.tpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/network/zeromq/socket/Push.hpp"

template class opentxs::Pimpl<opentxs::network::zeromq::socket::Push>;

//#define OT_METHOD
//"opentxs::network::zeromq::socket::implementation::Push::"

namespace opentxs::factory
{
auto PushSocket(const network::zeromq::Context& context, const bool direction)
    -> std::unique_ptr<network::zeromq::socket::Push>
{
    using ReturnType = network::zeromq::socket::implementation::Push;

    return std::make_unique<ReturnType>(
        context,
        static_cast<network::zeromq::socket::Socket::Direction>(direction));
}
}  // namespace opentxs::factory

namespace opentxs::network::zeromq::socket::implementation
{
Push::Push(
    const zeromq::Context& context,
    const Socket::Direction direction) noexcept
    : Socket(context, SocketType::Push, direction)
    , Sender()
    , Client(this->get())
{
    init();
}

Push::~Push() SHUTDOWN
}  // namespace opentxs::network::zeromq::socket::implementation
