// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                  // IWYU pragma: associated
#include "1_Internal.hpp"                // IWYU pragma: associated
#include "blockchain/p2p/peer/Peer.hpp"  // IWYU pragma: associated

#include <chrono>
#include <stdexcept>
#include <string_view>

#include "blockchain/DownloadTask.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/api/Core.hpp"
#include "opentxs/api/Endpoints.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/core/Flag.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"
#include "opentxs/network/asio/Socket.hpp"  // IWYU pragma: keep
#include "opentxs/network/zeromq/Frame.hpp"
#include "opentxs/network/zeromq/FrameSection.hpp"
#include "opentxs/network/zeromq/Message.hpp"
#include "opentxs/network/zeromq/Pipeline.hpp"
#include "util/ScopeGuard.hpp"

#define OT_BLOCKCHAIN_PEER_PING_SECONDS 30
#define OT_BLOCKCHAIN_PEER_DISCONNECT_SECONDS 40
#define OT_BLOCKCHAIN_PEER_DOWNLOAD_ADDRESSES_MINUTES 10

#define OT_METHOD "opentxs::blockchain::p2p::implementation::Peer::"

namespace zmq = opentxs::network::zeromq;

namespace opentxs::blockchain::p2p::implementation
{
Peer::Peer(
    const api::Core& api,
    const node::internal::Config& config,
    const node::internal::Mempool& mempool,
    const node::internal::Network& network,
    const node::internal::FilterOracle& filter,
    const node::internal::BlockOracle& block,
    const node::internal::PeerManager& manager,
    const int id,
    const std::string& shutdown,
    const std::size_t headerSize,
    const std::size_t bodySize,
    std::unique_ptr<internal::Address> address) noexcept
    : Worker(api, std::chrono::milliseconds(10))
    , network_(network)
    , filter_(filter)
    , block_(block)
    , manager_(manager)
    , mempool_(mempool)
    , chain_(address->Chain())
    , header_probe_(false)
    , cfilter_probe_(false)
    , address_(std::move(address))
    , download_peers_()
    , state_()
    , cfheader_job_()
    , cfilter_job_()
    , block_job_()
    , known_transactions_()
    , verify_filter_checkpoint_(config.download_cfilters_)
    , id_(id)
    , shutdown_endpoint_(shutdown)
    , connection_(init_connection_manager(
          api_,
          id_,
          manager_,
          *this,
          running_,
          address_,
          headerSize))
    , send_promises_()
    , activity_()
    , init_promise_()
    , init_(init_promise_.get_future())
{
    OT_ASSERT(connection_);

    if (false == connection_->init(id_)) {
        LogNormal("Disconnecting ")(DisplayString(chain_))(" peer ")(
            address_.Display())(" due to connection timeout.")
            .Flush();
        disconnect();
    }
}

auto Peer::break_promises() noexcept -> void
{
    state_.break_promises();
    send_promises_.Break();
}

auto Peer::check_activity() noexcept -> void
{
    const auto interval = Clock::now() - activity_.get();
    const bool disconnect =
        std::chrono::seconds(OT_BLOCKCHAIN_PEER_DISCONNECT_SECONDS) <= interval;
    const bool ping =
        std::chrono::seconds(OT_BLOCKCHAIN_PEER_PING_SECONDS) <= interval;

    if (disconnect) {
        LogNormal("Disconnecting ")(DisplayString(chain_))(" peer ")(
            address_.Display())(" due to activity timeout.")
            .Flush();
        this->disconnect();
    } else if (ping) {
        this->ping();
    }
}

auto Peer::check_download_peers() noexcept -> void
{
    const auto interval = Clock::now() - download_peers_.get();
    const bool download =
        std::chrono::minutes(OT_BLOCKCHAIN_PEER_DOWNLOAD_ADDRESSES_MINUTES) <=
        interval;

    if (download) { request_addresses(); }
}

auto Peer::check_jobs() noexcept -> void
{
    constexpr auto limit = std::chrono::minutes(1);

    if (auto& job = cfheader_job_; job) {
        if (job.Elapsed() >= limit) { reset_cfheader_job(); }
    } else if (cfilter_probe_) {
        reset_cfheader_job();
    }

    if (auto& job = cfilter_job_; job) {
        if (job.Elapsed() >= limit) { reset_cfilter_job(); }
    } else if (cfilter_probe_) {
        reset_cfilter_job();
    }

    if (auto& job = block_job_; job) {
        if (job.Elapsed() >= limit) { reset_block_job(); }
    } else if (header_probe_) {
        reset_block_job();
    }
}

auto Peer::check_handshake() noexcept -> void
{
    auto& state = state_.handshake_;

    if (state.first_action_ && state.second_action_ &&
        (false == state.done())) {
        LogDetail(
            address_.Incoming() ? "Incoming connection from "
                                : "Connected to ")(
            DisplayString(address_.Chain()))(" peer at ")(address_.Display())
            .Flush();
        LogVerbose("Advertised services: ").Flush();

        for (const auto& service : address_.Services()) {
            LogVerbose(" * ")(p2p::DisplayService(service)).Flush();
        }

        update_address_activity();
        state.promise_.set_value();

        OT_ASSERT(state.done());
    }

    trigger();
}

auto Peer::check_verify() noexcept -> void
{
    auto& state = state_.verify_;

    if (state.first_action_ &&
        (state.second_action_ || (false == verify_filter_checkpoint_)) &&
        (false == state.done())) {
        state.promise_.set_value();
    }

    trigger();
}

auto Peer::connect() noexcept -> void { connection_->connect(); }

auto Peer::disconnect() noexcept -> void
{
    try {
        state_.connect_.promise_.set_value(false);
    } catch (...) {
    }

    LogVerbose(
        address_.Incoming() ? "Dropping incoming connection "
                            : "Disconnecting from ")(connection_->host())(":")(
        connection_->port())
        .Flush();
    manager_.Disconnect(id_);
}

auto Peer::init() noexcept -> void
{
    connect();
    init_promise_.set_value();
}

auto Peer::init_connection_manager(
    const api::Core& api,
    const int id,
    const node::internal::PeerManager& manager,
    Peer& parent,
    const Flag& running,
    const Address& address,
    const std::size_t headerSize) noexcept -> std::unique_ptr<ConnectionManager>
{
    if (Network::zmq == address.Type()) {
        if (address.Incoming()) {

            return ConnectionManager::ZMQIncoming(
                api, parent, running, address, headerSize);
        } else {

            return ConnectionManager::ZMQ(
                api, parent, running, address, headerSize);
        }
    } else {
        if (address.Incoming()) {

            return ConnectionManager::TCPIncoming(
                api,
                parent,
                running,
                address,
                headerSize,
                manager.LookupIncomingSocket(id));
        } else {

            return ConnectionManager::TCP(
                api, parent, running, address, headerSize);
        }
    }
}

auto Peer::on_connect() noexcept -> void
{
    try {
        state_.connect_.promise_.set_value(true);
    } catch (...) {
    }

    state_.value_.store(
        address_.Incoming() ? State::Listening : State::Handshake);
    init_executor({manager_.Endpoint(Task::Heartbeat), shutdown_endpoint_});
    trigger();
}

auto Peer::on_pipeline(
    const Task type,
    const std::vector<ReadView>& frames) noexcept -> void
{
    auto message = MakeWork(type);

    for (const auto& frame : frames) {
        if ((nullptr != frame.data()) && (0 < frame.size())) {
            message->AddFrame(frame.data(), frame.size());
        } else {
            message->AddFrame();
        }
    }

    pipeline_->Push(message);
}

auto Peer::pipeline(zmq::Message& message) noexcept -> void
{
    if (false == running_.get()) { return; }

    const auto header = message.Header();

    if (0 < header.size()) {
        auto* promise = reinterpret_cast<std::promise<void>*>(
            header.at(0).as<std::uintptr_t>());

        OT_ASSERT(nullptr != promise);

        promise->set_value();
    }

    const auto body = message.Body();

    OT_ASSERT(0 < body.size());

    const auto task = [&] {
        try {

            return body.at(0).as<Task>();
        } catch (const std::exception& e) {
            LogOutput(OT_METHOD)(__func__)(": ")(e.what()).Flush();
            // TODO It's impossible for this exception to happen but it does
            // anyway from time to time. Somebody really ought to figure out why
            // someday.

            return Task::Heartbeat;
        }
    }();

    switch (task) {
        case Task::Shutdown: {
            shutdown(shutdown_promise_);
        } break;
        case Task::Mempool: {
            process_mempool(message);
        } break;
        case Task::Disconnect: {
            disconnect();
        } break;
        case Task::Getheaders: {
            if (State::Run == state_.value_.load()) { request_headers(); }
        } break;
        case Task::Getblock: {
            request_block(message);
        } break;
        case Task::BroadcastTransaction: {
            broadcast_transaction(message);
        } break;
        case Task::BroadcastBlock: {
            broadcast_block(message);
        } break;
        case Task::JobAvailableCfheaders: {
            if (State::Run == state_.value_.load()) {
                if (cfheader_job_) { break; }

                reset_cfheader_job();
            }
        } break;
        case Task::JobAvailableCfilters: {
            if (State::Run == state_.value_.load()) {
                if (cfilter_job_) { break; }

                reset_cfilter_job();
            }
        } break;
        case Task::JobAvailableBlock: {
            if (State::Run == state_.value_.load()) {
                if (block_job_) { break; }

                reset_block_job();
            }
        } break;
        case Task::Header: {
            activity_.Bump();
        } break;
        case Task::ReceiveMessage: {
            activity_.Bump();
            process_message(message);
        } break;
        case Task::SendMessage: {
            transmit(message);
        } break;
        case Task::Heartbeat:
        case Task::StateMachine: {
            do_work();
        } break;
        default: {
            OT_FAIL;
        }
    }
}

auto Peer::process_mempool(const zmq::Message& message) noexcept -> void
{
    const auto body = message.Body();

    if (body.at(1).as<Type>() != chain_) { return; }

    const auto hash = std::string{body.at(2).Bytes()};

    if (0 < known_transactions_.count(hash)) { return; }

    broadcast_inv_transaction(hash);
}

auto Peer::process_state_machine() noexcept -> void
{
    switch (state_.value_.load()) {
        case State::Run: {
            check_activity();
            check_jobs();
            check_download_peers();
        } break;
        default: {
        }
    }
}

auto Peer::reset_block_job() noexcept -> void
{
    auto& job = block_job_;
    job = {};

    if (header_probe_) { job = block_.GetBlockJob(); }

    if (job) { request_blocks(); }
}

auto Peer::reset_cfheader_job() noexcept -> void
{
    auto& job = cfheader_job_;
    job = {};

    if (cfilter_probe_) { job = filter_.GetHeaderJob(); }

    if (job) { request_cfheaders(); }
}

auto Peer::reset_cfilter_job() noexcept -> void
{
    auto& job = cfilter_job_;
    job = {};

    if (cfilter_probe_) { job = filter_.GetFilterJob(); }

    if (job) { request_cfilter(); }
}

auto Peer::send(OTData in) noexcept -> SendStatus
{
    try {
        if (false == state_.connect_.future_.get()) {
            LogVerbose(OT_METHOD)(__func__)(
                ": Unable to send to disconnected peer")
                .Flush();

            return {};
        }
    } catch (...) {
        return {};
    }

    if (running_.get()) {
        auto [future, promise] = send_promises_.NewPromise();
        auto message = MakeWork(Task::SendMessage);
        message->AddFrame(in);
        message->AddFrame(promise);
        pipeline_->Push(message);

        return std::move(future);
    } else {
        return {};
    }
}

auto Peer::Shutdown() noexcept -> std::shared_future<void>
{
    connection_->stop_internal();

    return stop_worker();
}

auto Peer::shutdown(std::promise<void>& promise) noexcept -> void
{
    init_.get();

    if (running_->Off()) {
        const auto state = state_.value_.exchange(State::Shutdown);
        connection_->stop_external();
        break_promises();
        connection_->shutdown_external();

        if ((State::Handshake != state) && (State::Listening != state)) {
            update_address_activity();
        }

        LogVerbose("Disconnected from ")(address_.Display()).Flush();

        try {
            promise.set_value();
        } catch (...) {
        }
    }
}

auto Peer::start_verify() noexcept -> void
{
    if (address_.Incoming()) {
        LogVerbose(OT_METHOD)(__func__)(": incoming peer ")(address_.Display())(
            " is not required to validate checkpoints")
            .Flush();
        state_.value_.store(State::Subscribe);
        do_work();
    } else {
        request_checkpoint_block_header();

        if (verify_filter_checkpoint_) {
            LogVerbose(OT_METHOD)(__func__)(": outgoing peer ")(
                address_.Display())(
                " must validate block header and cfheader checkpoints")
                .Flush();
            request_checkpoint_filter_header();
        } else {
            LogVerbose(OT_METHOD)(__func__)(": outgoing peer ")(
                address_.Display())(
                " must validate block header checkpoints only")
                .Flush();
        }
    }
}

auto Peer::state_machine() noexcept -> bool
{
    LogTrace(OT_METHOD)(__func__).Flush();

    if (false == running_.get()) { return false; }

    auto disconnect{false};

    switch (state_.value_.load()) {
        case State::Listening: {
            OT_ASSERT(address_.Incoming());

            LogVerbose(OT_METHOD)(__func__)(
                ": verifying incoming handshake protocol for ")(
                address_.Display())
                .Flush();
            disconnect = state_.handshake_.run(
                std::chrono::seconds(20), [] {}, State::Verify);
        } break;
        case State::Handshake: {
            LogVerbose(OT_METHOD)(__func__)(
                ": verifying outgoing handshake protocol for ")(
                address_.Display())
                .Flush();
            disconnect = state_.handshake_.run(
                std::chrono::seconds(20),
                [this] { start_handshake(); },
                State::Verify);
        } break;
        case State::Verify: {
            LogVerbose(OT_METHOD)(__func__)(": verifying checkpoints for ")(
                address_.Display())
                .Flush();
            disconnect = state_.verify_.run(
                std::chrono::seconds(30),
                [this] { start_verify(); },
                State::Subscribe);
        } break;
        case State::Subscribe: {
            LogVerbose(OT_METHOD)(__func__)(": achieved subscribe state for ")(
                address_.Display())
                .Flush();
            subscribe();
            state_.value_.store(State::Run);
            manager_.VerifyPeer(id_, address_.Display());
            [[fallthrough]];
        }
        case State::Run: {
            LogVerbose(OT_METHOD)(__func__)(": achieved run state for ")(
                address_.Display())
                .Flush();
            process_state_machine();
        } break;
        case State::Shutdown: {
            disconnect = true;
        } break;
        default: {
        }
    }

    if (disconnect) {
        LogVerbose(OT_METHOD)(__func__)(": Disconnecting ")(address_.Display())
            .Flush();
        this->disconnect();
    }

    return false;
}

auto Peer::subscribe() noexcept -> void
{
    const auto network =
        (1 == address_.Services().count(p2p::Service::Network));
    const auto limited =
        (1 == address_.Services().count(p2p::Service::Limited));
    const auto cfilter =
        (1 == address_.Services().count(p2p::Service::CompactFilters));

    if (network || limited || header_probe_) {
        pipeline_->Start(manager_.Endpoint(Task::Getheaders));
        pipeline_->Start(manager_.Endpoint(Task::Getblock));
        pipeline_->Start(manager_.Endpoint(Task::BroadcastTransaction));
        pipeline_->Start(manager_.Endpoint(Task::JobAvailableBlock));
    }

    if (cfilter || cfilter_probe_) {
        pipeline_->Start(manager_.Endpoint(Task::JobAvailableCfheaders));
        pipeline_->Start(manager_.Endpoint(Task::JobAvailableCfilters));
    }

    pipeline_->Start(manager_.Endpoint(Task::BroadcastBlock));
    pipeline_->Start(api_.Endpoints().BlockchainMempool());
    request_headers();
    request_addresses();
    request_mempool();
}

auto Peer::transmit(zmq::Message& message) noexcept -> void
{
    if (false == running_.get()) { return; }

    if (3 < message.Body().size()) {
        LogOutput(OT_METHOD)(__func__)(": Invalid message").Flush();

        return;
    }

    const auto& payload = message.Body_at(1);
    const auto& promiseFrame = message.Body_at(2);
    const auto index = promiseFrame.as<int>();
    auto success = bool{false};
    auto postcondition =
        ScopeGuard{[&] { send_promises_.SetPromise(index, success); }};
    LogTrace(OT_METHOD)(__func__)(": Sending ")(payload.size())(
        " byte message:")
        .Flush();
    LogTrace(Data::Factory(payload)->asHex()).Flush();
    auto promise = std::make_unique<SendPromise>();

    OT_ASSERT(promise);

    auto future = promise->get_future();
    connection_->transmit(payload, std::move(promise));
    auto result{false};
    const auto start = Clock::now();
    static const auto limit = std::chrono::seconds{10};

    try {
        while (running_.get()) {
            const auto status = future.wait_for(std::chrono::milliseconds(5));

            if (std::future_status::ready == status) {
                result = future.get();

                break;
            } else if (const auto time = Clock::now() - start; time >= limit) {
                LogNormal("Disconnecting ")(DisplayString(chain_))(" peer ")(
                    address_.Display())(" due to transmit timeout.")
                    .Flush();
                disconnect();

                return;
            }
        }
    } catch (const std::exception& e) {
        LogNormal("Disconnecting ")(DisplayString(chain_))(" peer ")(
            address_.Display())(" due to transmit error: ")(e.what())
            .Flush();
        disconnect();

        return;
    }

    if (result) {
        LogVerbose(OT_METHOD)(__func__)(": Sent ")(payload.size())(" bytes")
            .Flush();
        success = true;
    } else {
        LogNormal("Disconnecting ")(DisplayString(chain_))(" peer ")(
            address_.Display())(" due to unspecified transmit error.")
            .Flush();
        success = false;
        disconnect();
    }
}

auto Peer::update_address_activity() noexcept -> void
{
    manager_.Database().AddOrUpdate(address_.UpdateTime(activity_.get()));
}

auto Peer::update_address_services(
    const std::set<p2p::Service>& services) noexcept -> void
{
    manager_.Database().AddOrUpdate(address_.UpdateServices(services));
}

Peer::~Peer() { Shutdown().get(); }
}  // namespace opentxs::blockchain::p2p::implementation
