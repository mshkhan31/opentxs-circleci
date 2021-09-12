// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "0_stdafx.hpp"    // IWYU pragma: associated
#include "1_Internal.hpp"  // IWYU pragma: associated

#include <zmq.h>
#include <atomic>
#include <functional>
#include <limits>
#include <mutex>
#include <thread>

#include "Proto.hpp"
#include "Proto.tpp"
#include "blockchain/DownloadManager.hpp"
#include "blockchain/node/FilterOracle.hpp"
#include "internal/blockchain/Blockchain.hpp"
#include "network/zeromq/socket/Socket.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/blockchain/block/bitcoin/Block.hpp"
#include "opentxs/blockchain/block/bitcoin/Header.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/network/blockchain/sync/Block.hpp"
#include "opentxs/network/blockchain/sync/Data.hpp"
#include "opentxs/network/blockchain/sync/MessageType.hpp"
#include "opentxs/network/blockchain/sync/Request.hpp"
#include "opentxs/network/blockchain/sync/State.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/Frame.hpp"
#include "opentxs/network/zeromq/FrameSection.hpp"
#include "opentxs/network/zeromq/Message.hpp"
#include "opentxs/network/zeromq/Pipeline.hpp"
#include "opentxs/network/zeromq/socket/Publish.hpp"
#include "opentxs/network/zeromq/socket/Socket.hpp"
#include "opentxs/util/WorkType.hpp"

#define SYNC_SERVER "opentxs::blockchain::node::base::SyncServer::"

namespace opentxs::blockchain::node::base
{
using SyncDM = download::
    Manager<SyncServer, std::unique_ptr<const node::GCS>, int, filter::Type>;
using SyncWorker = Worker<SyncServer, api::Core>;

class SyncServer : public SyncDM, public SyncWorker
{
public:
    auto Tip() const noexcept { return db_.SyncTip(); }

    auto NextBatch() noexcept { return allocate_batch(type_); }
    auto Shutdown() noexcept -> std::shared_future<void>
    {
        {
            auto lock = Lock{zmq_lock_};
            zmq_running_ = false;
        }

        if (zmq_thread_.joinable()) { zmq_thread_.join(); }

        return stop_worker();
    }

    SyncServer(
        const api::Core& api,
        const node::internal::SyncDatabase& db,
        const node::internal::HeaderOracle& header,
        const node::internal::FilterOracle& filter,
        const node::internal::Network& node,
        const blockchain::Type chain,
        const filter::Type type,
        const std::string& shutdown,
        const std::string& publishEndpoint) noexcept
        : SyncDM(
              [&] { return db.SyncTip(); }(),
              [&] {
                  auto promise = std::promise<int>{};
                  promise.set_value(0);

                  return Finished{promise.get_future()};
              }(),
              "sync server",
              2000,
              1000)
        , SyncWorker(api, std::chrono::milliseconds{20})
        , db_(db)
        , header_(header)
        , filter_(filter)
        , node_(node)
        , chain_(chain)
        , type_(type)
        , linger_(0)
        , endpoint_(publishEndpoint)
        , socket_(::zmq_socket(api_.Network().ZeroMQ(), ZMQ_PAIR), ::zmq_close)
        , zmq_lock_()
        , zmq_running_(true)
        , zmq_thread_(&SyncServer::zmq_thread, this)
    {
        init_executor(
            {shutdown,
             api_.Endpoints().InternalBlockchainFilterUpdated(chain_)});
        ::zmq_setsockopt(socket_.get(), ZMQ_LINGER, &linger_, sizeof(linger_));
        ::zmq_connect(socket_.get(), endpoint_.c_str());
    }

    ~SyncServer() { Shutdown().get(); }

private:
    friend SyncDM;
    friend SyncWorker;

    using Socket = std::unique_ptr<void, decltype(&::zmq_close)>;
    using OTSocket = opentxs::network::zeromq::socket::implementation::Socket;
    using Work = node::implementation::Base::Work;

    const node::internal::SyncDatabase& db_;
    const node::internal::HeaderOracle& header_;
    const node::internal::FilterOracle& filter_;
    const node::internal::Network& node_;
    const blockchain::Type chain_;
    const filter::Type type_;
    const int linger_;
    const std::string endpoint_;
    Socket socket_;
    mutable std::mutex zmq_lock_;
    std::atomic_bool zmq_running_;
    std::thread zmq_thread_;

    auto batch_ready() const noexcept -> void { trigger(); }
    auto batch_size(const std::size_t in) const noexcept -> std::size_t
    {
        if (in < 10) {

            return 1;
        } else if (in < 100) {

            return 10;
        } else if (in < 1000) {

            return 100;
        } else {

            return 1000;
        }
    }
    auto check_task(TaskType&) const noexcept -> void {}
    auto hello(const Lock&, const block::Position& incoming) const noexcept
    {
        // TODO use known() and Ancestors() instead
        auto [parent, best] = header_.CommonParent(incoming);
        if ((0 == parent.first) && (1000 < incoming.first)) {
            const auto height = std::min(incoming.first - 1000, best.first);
            parent = {height, header_.BestHash(height)};
        }
        const auto needSync = incoming != best;
        auto state = network::blockchain::sync::State{chain_, std::move(best)};

        return std::make_tuple(needSync, parent, std::move(state));
    }
    auto trigger_state_machine() const noexcept -> void { trigger(); }
    auto update_tip(const Position& position, const int&) const noexcept -> void
    {
        auto saved = db_.SetSyncTip(position);

        OT_ASSERT(saved);

        LogDetail(DisplayString(chain_))(" sync data updated to height ")(
            position.first)
            .Flush();
    }

    auto download() noexcept -> void
    {
        auto work = NextBatch();

        for (const auto& task : work.data_) {
            task->download(filter_.LoadFilter(type_, task->position_.second));
        }
    }
    auto pipeline(const zmq::Message& in) noexcept -> void
    {
        if (false == running_.get()) { return; }

        const auto body = in.Body();

        OT_ASSERT(1 <= body.size());

        using Work = Work;
        const auto work = [&] {
            try {

                return body.at(0).as<Work>();
            } catch (...) {

                OT_FAIL;
            }
        }();
        auto lock = Lock{zmq_lock_};

        switch (work) {
            case Work::shutdown: {
                shutdown(shutdown_promise_);
            } break;
            case Work::heartbeat: {
                if (dm_enabled()) { process_position(filter_.Tip(type_)); }

                run_if_enabled();
            } break;
            case Work::filter: {
                process_position(in);
                run_if_enabled();
            } break;
            case Work::statemachine: {
                download();
                run_if_enabled();
            } break;
            default: {
                OT_FAIL;
            }
        }
    }
    auto process_position(const zmq::Message& in) noexcept -> void
    {
        const auto body = in.Body();

        OT_ASSERT(body.size() > 3);

        const auto type = body.at(1).as<filter::Type>();

        if (type != type_) { return; }

        process_position(Position{
            body.at(2).as<block::Height>(), api_.Factory().Data(body.at(3))});
    }
    auto process_position(const Position& pos) noexcept -> void
    {
        LogTrace(SYNC_SERVER)(__func__)(": processing block ")(
            pos.second->asHex())(" at height ")(pos.first)
            .Flush();

        try {
            auto current = known();
            auto hashes = header_.Ancestors(current, pos, 2000);
            LogTrace(SYNC_SERVER)(__func__)(
                ": current position best known position is block ")(
                current.second->asHex())(" at height ")(current.first)
                .Flush();

            OT_ASSERT(0 < hashes.size());

            if (1 == hashes.size()) {
                LogTrace(SYNC_SERVER)(__func__)(
                    ": current position matches incoming block ")(
                    pos.second->asHex())(" at height ")(pos.first)
                    .Flush();

                return;
            }

            auto prior = Previous{std::nullopt};
            {
                auto& first = hashes.front();
                auto promise = std::promise<int>{};
                promise.set_value(0);
                prior.emplace(std::move(first), promise.get_future());
            }
            hashes.erase(hashes.begin());
            {
                const auto& first = hashes.front();
                const auto& last = hashes.back();

                if (first.first <= current.first) {
                    LogTrace(SYNC_SERVER)(__func__)(": reorg detected").Flush();
                }

                LogTrace(SYNC_SERVER)(__func__)(
                    ": scheduling download starting from block ")(
                    first.second->asHex())(" at height ")(first.first)(
                    " until block ")(last.second->asHex())(" at height ")(
                    last.first)
                    .Flush();
            }
            update_position(std::move(hashes), type_, std::move(prior));
        } catch (...) {
        }
    }
    auto process_zmq(const Lock& lock) noexcept -> void
    {
        const auto incoming = [&] {
            auto output = api_.Network().ZeroMQ().Message();
            OTSocket::receive_message(lock, socket_.get(), output);

            return output;
        }();
        namespace sync = network::blockchain::sync;
        const auto base = sync::Factory(api_, incoming);

        if (auto type = base->Type(); type != sync::MessageType::sync_request) {
            LogOutput(SYNC_SERVER)(__func__)(
                ": Invalid or unsupported message type ")(opentxs::print(type))
                .Flush();

            return;
        }

        try {
            const auto& request = base->asRequest();
            const auto& state = [&]() -> const sync::State& {
                for (const auto& state : request.State()) {
                    if (state.Chain() == chain_) { return state; }
                }

                throw std::runtime_error{"No matching chains"};
            }();
            const auto& position = state.Position();
            auto [needSync, parent, data] = hello(lock, position);
            const auto& [height, hash] = parent;
            auto reply =
                sync::Data{WorkType::SyncReply, std::move(data), {}, {}};
            auto send{true};

            if (needSync) { send = db_.LoadSync(height, reply); }

            auto out = api_.Network().ZeroMQ().ReplyMessage(incoming);

            if (send && reply.Serialize(out)) {
                OTSocket::send_message(lock, socket_.get(), out);
            }
        } catch (const std::exception& e) {
            LogOutput(SYNC_SERVER)(__func__)(": ")(e.what()).Flush();
        }
    }
    auto queue_processing(DownloadedData&& data) noexcept -> void
    {
        if (0 == data.size()) { return; }

        const auto& tip = data.back();
        auto items = std::vector<network::blockchain::sync::Block>{};
        auto previousFilterHeader = api_.Factory().Data();

        for (const auto& task : data) {
            try {
                const auto pHeader =
                    header_.LoadBitcoinHeader(task->position_.second);

                if (false == bool(pHeader)) {
                    throw std::runtime_error(
                        std::string{"failed to load block header "} +
                        task->position_.second->asHex());
                }

                const auto& header = *pHeader;

                if (previousFilterHeader->empty()) {
                    previousFilterHeader =
                        filter_.LoadFilterHeader(type_, header.ParentHash());

                    if (previousFilterHeader->empty()) {
                        throw std::runtime_error(
                            std::string{"failed to previous filter header for "
                                        "block  "} +
                            task->position_.second->asHex());
                    }
                }

                const auto& pGCS = task->data_.get();

                if (false == bool(pGCS)) {
                    throw std::runtime_error(
                        std::string{"failed to load gcs for block "} +
                        task->position_.second->asHex());
                }

                const auto& gcs = *pGCS;
                const auto headerBytes = header.Encode();
                const auto filterBytes = gcs.Compressed();
                items.emplace_back(
                    chain_,
                    task->position_.first,
                    type_,
                    gcs.ElementCount(),
                    headerBytes->Bytes(),
                    reader(filterBytes));
                task->process(1);
            } catch (const std::exception& e) {
                LogOutput(SYNC_SERVER)(__func__)(": ")(e.what()).Flush();
                task->redownload();
                break;
            }
        }

        if (previousFilterHeader->empty() || (0 == items.size())) {
            LogOutput(SYNC_SERVER)(__func__)(": missing data").Flush();

            return;
        }

        const auto& pos = tip->position_;
        const auto stored = db_.StoreSync(pos, items);

        if (false == stored) { OT_FAIL; }

        using Data = network::blockchain::sync::Data;
        const auto msg = Data{
            WorkType::NewBlock,
            {chain_, pos},
            std::move(items),
            previousFilterHeader->Bytes()};
        auto work = api_.Network().ZeroMQ().Message();

        if (msg.Serialize(work) && zmq_running_) {
            // NOTE the appropriate lock is already being held in the pipeline
            // function
            auto dummy = std::mutex{};
            auto lock = Lock{dummy};
            OTSocket::send_message(lock, socket_.get(), work);
        }
    }
    auto shutdown(std::promise<void>& promise) noexcept -> void
    {
        if (running_->Off()) {
            try {
                promise.set_value();
            } catch (...) {
            }
        }
    }
    auto zmq_thread() noexcept -> void
    {
        auto poll = [&] {
            auto output = std::vector<::zmq_pollitem_t>{};

            {
                auto& item = output.emplace_back();
                item.socket = socket_.get();
                item.events = ZMQ_POLLIN;
            }

            return output;
        }();

        OT_ASSERT(std::numeric_limits<int>::max() >= poll.size());

        while (zmq_running_) {
            constexpr auto timeout = std::chrono::milliseconds{250};
            const auto events = ::zmq_poll(
                poll.data(), static_cast<int>(poll.size()), timeout.count());

            if (0 > events) {
                const auto error = ::zmq_errno();
                LogOutput(SYNC_SERVER)(__func__)(": ")(::zmq_strerror(error))
                    .Flush();

                continue;
            } else if (0 == events) {

                continue;
            }

            auto lock = Lock{zmq_lock_};

            for (const auto& item : poll) {
                if (ZMQ_POLLIN != item.revents) { continue; }

                process_zmq(lock);
            }
        }

        ::zmq_disconnect(socket_.get(), endpoint_.c_str());
    }
};
}  // namespace opentxs::blockchain::node::base
