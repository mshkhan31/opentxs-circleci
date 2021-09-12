// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "network/zeromq/socket/Receiver.hpp"  // IWYU pragma: associated

#include <zmq.h>
#include <iostream>
#include <memory>
#include <mutex>
#include <thread>

#include "opentxs/Types.hpp"
#include "opentxs/core/Flag.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/network/zeromq/Frame.hpp"
#include "opentxs/network/zeromq/Message.hpp"

namespace opentxs::network::zeromq::socket::implementation
{
template <typename InterfaceType, typename MessageType>
Receiver<InterfaceType, MessageType>::Receiver(
    const zeromq::Context& context,
    const SocketType type,
    const Socket::Direction direction,
    const bool startThread) noexcept
    : Socket(context, type, direction)
    , start_thread_(startThread)
    , receiver_thread_()
    , next_task_(0)
    , task_lock_()
    , socket_tasks_()
    , task_result_()
{
}

template <typename InterfaceType, typename MessageType>
auto Receiver<InterfaceType, MessageType>::add_task(
    SocketCallback&& cb) const noexcept -> int
{
    Lock lock(task_lock_);
    auto [it, success] = socket_tasks_.emplace(++next_task_, std::move(cb));

    OT_ASSERT(success);

    return it->first;
}

template <typename InterfaceType, typename MessageType>
auto Receiver<InterfaceType, MessageType>::apply_socket(
    SocketCallback&& cb) const noexcept -> bool
{
    const auto id = add_task(std::move(cb));

    while (task_running(id)) {
        Sleep(std::chrono::milliseconds(RECEIVER_POLL_MILLISECONDS));
    }

    return task_result(id);
}

template <typename InterfaceType, typename MessageType>
auto Receiver<InterfaceType, MessageType>::Close() const noexcept -> bool
{
    running_->Off();

    if (receiver_thread_.joinable()) { receiver_thread_.join(); }

    return Socket::Close();
}

template <typename InterfaceType, typename MessageType>
void Receiver<InterfaceType, MessageType>::init() noexcept
{
    Socket::init();

    if (start_thread_) {
        receiver_thread_ = std::thread(&Receiver::thread, this);
    }
}

template <typename InterfaceType, typename MessageType>
void Receiver<InterfaceType, MessageType>::run_tasks(
    const Lock& lock) const noexcept
{
    Lock task_lock(task_lock_);
    auto i = socket_tasks_.begin();

    while (i != socket_tasks_.end()) {
        const auto& [id, cb] = *i;
        task_result_.emplace(id, cb(lock));
        i = socket_tasks_.erase(i);
    }
}

template <typename InterfaceType, typename MessageType>
void Receiver<InterfaceType, MessageType>::shutdown(const Lock& lock) noexcept
{
    if (receiver_thread_.joinable()) { receiver_thread_.join(); }

    Socket::shutdown(lock);
}

template <typename InterfaceType, typename MessageType>
auto Receiver<InterfaceType, MessageType>::task_result(
    const int id) const noexcept -> bool
{
    Lock lock(task_lock_);
    const auto it = task_result_.find(id);

    OT_ASSERT(task_result_.end() != it);

    auto output = it->second;
    task_result_.erase(it);

    return output;
}

template <typename InterfaceType, typename MessageType>
auto Receiver<InterfaceType, MessageType>::task_running(
    const int id) const noexcept -> bool
{
    Lock lock(task_lock_);

    return (1 == socket_tasks_.count(id));
}

template <typename InterfaceType, typename MessageType>
void Receiver<InterfaceType, MessageType>::thread() noexcept
{
    while (running_.get()) {
        if (have_callback()) { break; }

        Sleep(std::chrono::milliseconds(CALLBACK_WAIT_MILLISECONDS));
    }

    zmq_pollitem_t poll[1];
    poll[0].socket = socket_;
    poll[0].events = ZMQ_POLLIN;

    while (running_.get()) {
        auto newEndpoints = endpoint_queue_.pop();
        Lock lock(lock_, std::try_to_lock);

        if (false == lock.owns_lock()) { continue; }

        for (const auto& endpoint : newEndpoints) { start(lock, endpoint); }

        run_tasks(lock);
        const auto events = zmq_poll(poll, 1, RECEIVER_POLL_MILLISECONDS);

        if (0 == events) { continue; }

        if (-1 == events) {
            const auto error = zmq_errno();
            std::cerr << RECEIVER_METHOD << __func__
                      << ": Poll error: " << zmq_strerror(error) << std::endl;

            continue;
        }

        if (false == running_.get()) { return; }

        auto reply = MessageType::Factory();
        const auto received = Socket::receive_message(lock, socket_, reply);

        if (false == received) {
            std::cerr << RECEIVER_METHOD << __func__
                      << ": Failed to receive incoming message." << std::endl;

            continue;
        }

        process_incoming(lock, reply);
        lock.unlock();
        std::this_thread::yield();
    }
}

template <typename InterfaceType, typename MessageType>
Receiver<InterfaceType, MessageType>::~Receiver()
{
    if (receiver_thread_.joinable()) { receiver_thread_.join(); }
}
}  // namespace opentxs::network::zeromq::socket::implementation
