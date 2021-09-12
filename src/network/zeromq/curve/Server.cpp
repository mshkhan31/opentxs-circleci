// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                     // IWYU pragma: associated
#include "1_Internal.hpp"                   // IWYU pragma: associated
#include "network/zeromq/curve/Server.hpp"  // IWYU pragma: associated

#include <zmq.h>
#include <array>
#include <cstdint>
#include <utility>

#include "network/zeromq/socket/Socket.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"
#include "opentxs/core/Secret.hpp"

#define OT_METHOD "opentxs::network::zeromq::curve::implementation::Server::"

namespace opentxs::network::zeromq::curve::implementation
{
Server::Server(zeromq::socket::implementation::Socket& socket) noexcept
    : parent_(socket)
{
}

auto Server::SetDomain(const std::string& domain) const noexcept -> bool
{
    auto set =
        zmq_setsockopt(parent_, ZMQ_ZAP_DOMAIN, domain.data(), domain.size());

    if (0 != set) {
        LogOutput(OT_METHOD)(__func__)(": Failed to set domain.").Flush();

        return false;
    }

    return true;
}

auto Server::SetPrivateKey(const Secret& key) const noexcept -> bool
{
    if (CURVE_KEY_BYTES != key.size()) {
        LogOutput(OT_METHOD)(__func__)(": Invalid private key.").Flush();

        return false;
    }

    return set_private_key(key.data(), key.size());
}

auto Server::SetPrivateKey(const std::string& z85) const noexcept -> bool
{
    if (CURVE_KEY_Z85_BYTES > z85.size()) {
        LogOutput(OT_METHOD)(__func__)(": Invalid private key size (")(
            z85.size())(").")
            .Flush();

        return false;
    }

    std::array<std::uint8_t, CURVE_KEY_BYTES> key;
    ::zmq_z85_decode(key.data(), z85.data());

    return set_private_key(key.data(), key.size());
}

auto Server::set_private_key(const void* key, const std::size_t keySize)
    const noexcept -> bool
{
    OT_ASSERT(nullptr != parent_);

    socket::implementation::Socket::SocketCallback cb{[&](const Lock&) -> bool {
        const int server{1};
        auto set =
            zmq_setsockopt(parent_, ZMQ_CURVE_SERVER, &server, sizeof(server));

        if (0 != set) {
            LogOutput(OT_METHOD)(__func__)(": Failed to set ZMQ_CURVE_SERVER")
                .Flush();

            return false;
        }

        set = zmq_setsockopt(parent_, ZMQ_CURVE_SECRETKEY, key, keySize);

        if (0 != set) {
            LogOutput(OT_METHOD)(__func__)(": Failed to set private key.")
                .Flush();

            return false;
        }

        return true;
    }};

    return parent_.apply_socket(std::move(cb));
}
}  // namespace opentxs::network::zeromq::curve::implementation
