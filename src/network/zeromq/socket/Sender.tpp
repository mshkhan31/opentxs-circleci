// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "network/zeromq/socket/Sender.hpp"  // IWYU pragma: associated

#include "network/zeromq/socket/Socket.hpp"

//#define OT_METHOD "opentxs::network::zeromq::socket::implementation::Sender::"

namespace opentxs::network::zeromq::socket::implementation
{
template <typename Interface, typename ImplementationParent>
Sender<Interface, ImplementationParent>::Sender() noexcept
{
}

template <typename Interface, typename ImplementationParent>
auto Sender<Interface, ImplementationParent>::send(
    zeromq::Message& message) const noexcept -> bool
{
    Lock lock(this->lock_);

    if (false == this->running_.get()) { return false; }

    return this->send_message(lock, message);
}
}  // namespace opentxs::network::zeromq::socket::implementation
