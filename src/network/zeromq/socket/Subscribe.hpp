// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <map>
#include <string>
#include <vector>

#include "network/zeromq/curve/Client.hpp"
#include "network/zeromq/socket/Receiver.hpp"
#include "network/zeromq/socket/Receiver.tpp"
#include "opentxs/Types.hpp"
#include "opentxs/network/zeromq/socket/Subscribe.hpp"

namespace opentxs
{
namespace network
{
namespace zeromq
{
namespace socket
{
class Subscribe;
}  // namespace socket

class Context;
class ListenCallback;
class Message;
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs

namespace opentxs::network::zeromq::socket::implementation
{
class Subscribe : public Receiver<zeromq::socket::Subscribe>,
                  public zeromq::curve::implementation::Client
{
public:
    auto SetSocksProxy(const std::string& proxy) const noexcept -> bool final;

    Subscribe(
        const zeromq::Context& context,
        const zeromq::ListenCallback& callback) noexcept;

    ~Subscribe() override;

protected:
    const ListenCallback& callback_;

private:
    auto clone() const noexcept -> Subscribe* override;
    auto have_callback() const noexcept -> bool final;

    void init() noexcept final;
    void process_incoming(const Lock& lock, Message& message) noexcept final;

    Subscribe() = delete;
    Subscribe(const Subscribe&) = delete;
    Subscribe(Subscribe&&) = delete;
    auto operator=(const Subscribe&) -> Subscribe& = delete;
    auto operator=(Subscribe&&) -> Subscribe& = delete;
};
}  // namespace opentxs::network::zeromq::socket::implementation
