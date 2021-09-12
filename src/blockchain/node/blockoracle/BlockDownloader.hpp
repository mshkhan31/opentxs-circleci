// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "0_stdafx.hpp"                     // IWYU pragma: associated
#include "1_Internal.hpp"                   // IWYU pragma: associated
#include "blockchain/node/BlockOracle.hpp"  // IWYU pragma: associated

#include <functional>

#include "blockchain/DownloadManager.hpp"
#include "internal/blockchain/Blockchain.hpp"
#include "internal/blockchain/Params.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/blockchain/block/bitcoin/Block.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/Frame.hpp"
#include "opentxs/network/zeromq/FrameSection.hpp"
#include "opentxs/network/zeromq/Message.hpp"
#include "opentxs/network/zeromq/Pipeline.hpp"
#include "opentxs/network/zeromq/socket/Publish.hpp"
#include "opentxs/network/zeromq/socket/Socket.hpp"

namespace opentxs::blockchain::node::implementation
{
using BlockDM = download::Manager<
    BlockOracle::BlockDownloader,
    std::shared_ptr<const block::bitcoin::Block>,
    int>;
using BlockWorker = Worker<BlockOracle::BlockDownloader, api::Core>;

class BlockOracle::BlockDownloader : public BlockDM, public BlockWorker
{
public:
    auto NextBatch() noexcept { return allocate_batch(0); }

    BlockDownloader(
        const api::Core& api,
        const internal::BlockDatabase& db,
        const internal::HeaderOracle& header,
        const internal::Network& node,
        const blockchain::Type chain,
        const std::string& shutdown) noexcept
        : BlockDM(
              [&] { return db.BlockTip(); }(),
              [&] {
                  auto promise = std::promise<int>{};
                  promise.set_value(0);

                  return Finished{promise.get_future()};
              }(),
              "block",
              2000,
              1000)
        , BlockWorker(api, std::chrono::milliseconds{20})
        , db_(db)
        , header_(header)
        , node_(node)
        , chain_(chain)
        , socket_(api_.Network().ZeroMQ().PublishSocket())
    {
        init_executor({shutdown, api_.Endpoints().BlockchainReorg()});
        auto zmq = socket_->Start(
            api_.Endpoints().InternalBlockchainBlockUpdated(chain_));

        OT_ASSERT(zmq);
    }

    ~BlockDownloader() { stop_worker().get(); }

private:
    friend BlockDM;
    friend BlockWorker;

    const internal::BlockDatabase& db_;
    const internal::HeaderOracle& header_;
    const internal::Network& node_;
    const blockchain::Type chain_;
    OTZMQPublishSocket socket_;

    auto batch_ready() const noexcept -> void
    {
        node_.JobReady(internal::PeerManager::Task::JobAvailableBlock);
    }
    auto batch_size(const std::size_t in) const noexcept -> std::size_t
    {
        const auto limit =
            params::Data::Chains().at(chain_).block_download_batch_;

        if (in < 10) {

            return std::min<std::size_t>(1, limit);
        } else if (in < 100) {

            return std::min<std::size_t>(10, limit);
        } else if (in < 1000) {

            return std::min<std::size_t>(100, limit);
        } else {

            return limit;
        }
    }
    auto check_task(TaskType& task) const noexcept -> void
    {
        const auto& hash = task.position_.second;

        if (auto block = db_.BlockLoadBitcoin(hash); bool(block)) {
            task.download(std::move(block));
        }
    }
    auto trigger_state_machine() const noexcept -> void { trigger(); }
    auto update_tip(const Position& position, const int&) const noexcept -> void
    {
        const auto saved = db_.SetBlockTip(position);

        OT_ASSERT(saved);

        LogDetail(DisplayString(chain_))(" block chain updated to height ")(
            position.first)
            .Flush();
        auto work = MakeWork(OT_ZMQ_NEW_FULL_BLOCK_SIGNAL);
        work->AddFrame(position.first);
        work->AddFrame(position.second);
        socket_->Send(work);
    }

    auto pipeline(const zmq::Message& in) noexcept -> void
    {
        if (false == running_.get()) { return; }

        const auto body = in.Body();

        OT_ASSERT(1 <= body.size());

        const auto work = [&] {
            try {

                return body.at(0).as<BlockOracle::Work>();
            } catch (...) {

                OT_FAIL;
            }
        }();

        switch (work) {
            case BlockOracle::Work::shutdown: {
                shutdown(shutdown_promise_);
            } break;
            case BlockOracle::Work::block:
            case BlockOracle::Work::reorg: {
                if (dm_enabled()) { process_position(in); }

                run_if_enabled();
            } break;
            case BlockOracle::Work::heartbeat: {
                if (dm_enabled()) { process_position(); }

                run_if_enabled();
            } break;
            case BlockOracle::Work::statemachine: {
                run_if_enabled();
            } break;
            default: {
                OT_FAIL;
            }
        }
    }
    auto process_position(const zmq::Message& in) noexcept -> void
    {
        {
            const auto body = in.Body();

            OT_ASSERT(body.size() >= 4);

            const auto chain = body.at(1).as<blockchain::Type>();

            if (chain_ != chain) { return; }
        }

        process_position();
    }
    auto process_position() noexcept -> void
    {
        auto current = known();
        auto hashes = header_.BestChain(current, 2000);

        OT_ASSERT(0 < hashes.size());

        auto prior = Previous{std::nullopt};
        {
            auto& first = hashes.front();

            if (first != current) {
                auto promise = std::promise<int>{};
                promise.set_value(0);
                prior.emplace(std::move(first), promise.get_future());
            }
        }
        hashes.erase(hashes.begin());
        update_position(std::move(hashes), 0, std::move(prior));
    }
    auto queue_processing(DownloadedData&& data) noexcept -> void
    {
        if (0 == data.size()) { return; }

        for (const auto& task : data) {
            const auto& pBlock = task->data_.get();

            OT_ASSERT(pBlock);

            const auto& block = *pBlock;

            // TODO implement batch db updates
            if (db_.BlockStore(block)) {
                task->process(0);
            } else {
                task->redownload();
                break;
            }
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
};
}  // namespace opentxs::blockchain::node::implementation
