// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <atomic>
#include <chrono>
#include <functional>
#include <map>
#include <mutex>
#include <queue>
#include <set>
#include <string>
#include <vector>

#include "opentxs/Types.hpp"
#include "opentxs/core/Flag.hpp"
#include "opentxs/core/Lockable.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/socket/Socket.hpp"

namespace opentxs
{
namespace network
{
namespace zeromq
{
class Message;
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs

#define CURVE_KEY_BYTES 32
#define CURVE_KEY_Z85_BYTES 40
#define SHUTDOWN                                                               \
    {                                                                          \
        running_->Off();                                                       \
        Lock lock(lock_);                                                      \
        shutdown(lock);                                                        \
    }

namespace opentxs::network::zeromq::socket::implementation
{
class Socket : virtual public zeromq::socket::Socket, public Lockable
{
public:
    using SocketCallback = std::function<bool(const Lock& lock)>;

    static auto send_message(
        const Lock& lock,
        void* socket,
        zeromq::Message& message) noexcept -> bool;
    static auto random_inproc_endpoint() noexcept -> std::string;
    static auto receive_message(
        const Lock& lock,
        void* socket,
        zeromq::Message& message) noexcept -> bool;

    auto Type() const noexcept -> SocketType final { return type_; }

    operator void*() const noexcept final;

    virtual auto apply_socket(SocketCallback&& cb) const noexcept -> bool;
    auto Close() const noexcept -> bool override;
    auto Context() const noexcept -> const zeromq::Context& final
    {
        return context_;
    }
    auto SetTimeouts(
        const std::chrono::milliseconds& linger,
        const std::chrono::milliseconds& send,
        const std::chrono::milliseconds& receive) const noexcept -> bool final;
    auto Start(const std::string& endpoint) const noexcept -> bool override;
    auto StartAsync(const std::string& endpoint) const noexcept -> void final;

    auto get() -> Socket& { return *this; }

    ~Socket() override;

protected:
    struct Endpoints {
        std::mutex lock_{};
        std::queue<std::string> queue_{};

        auto pop() noexcept -> std::vector<std::string>
        {
            auto output = std::vector<std::string>{};

            Lock lock{lock_};
            output.reserve(queue_.size());

            while (false == queue_.empty()) {
                output.emplace_back(queue_.front());
                queue_.pop();
            }

            return output;
        }
    };

    const zeromq::Context& context_;
    const Socket::Direction direction_;
    mutable void* socket_;
    mutable std::atomic<int> linger_{0};
    mutable std::atomic<int> send_timeout_{-1};
    mutable std::atomic<int> receive_timeout_{-1};
    mutable std::mutex endpoint_lock_;
    mutable std::set<std::string> endpoints_;
    mutable OTFlag running_;
    mutable Endpoints endpoint_queue_;

    void add_endpoint(const std::string& endpoint) const noexcept;
    auto apply_timeouts(const Lock& lock) const noexcept -> bool;
    auto bind(const Lock& lock, const std::string& endpoint) const noexcept
        -> bool;
    auto connect(const Lock& lock, const std::string& endpoint) const noexcept
        -> bool;
    auto receive_message(const Lock& lock, zeromq::Message& message)
        const noexcept -> bool;
    auto send_message(const Lock& lock, zeromq::Message& message) const noexcept
        -> bool;
    auto set_socks_proxy(const std::string& proxy) const noexcept -> bool;
    auto start(const Lock& lock, const std::string& endpoint) const noexcept
        -> bool;

    virtual void init() noexcept {}
    virtual void shutdown(const Lock& lock) noexcept;

    explicit Socket(
        const zeromq::Context& context,
        const SocketType type,
        const Socket::Direction direction) noexcept;

private:
    static const std::map<SocketType, int> types_;

    const SocketType type_{SocketType::Error};

    Socket() = delete;
    Socket(const Socket&) = delete;
    Socket(Socket&&) = delete;
    auto operator=(const Socket&) -> Socket& = delete;
    auto operator=(Socket&&) -> Socket& = delete;
};
}  // namespace opentxs::network::zeromq::socket::implementation
