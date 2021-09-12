// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <gtest/gtest.h>
#include <string>

#include "blockchain/DownloadManager.hpp"
#include "opentxs/core/Data.hpp"

namespace ot = opentxs;
namespace b = ot::blockchain;
namespace d = b::download;
namespace bb = b::block;

namespace
{
struct DownloadManager;

using DownloadType = int;
using FinishedType = std::string;
using ManagerType = d::Manager<DownloadManager, DownloadType, FinishedType>;

struct DownloadManager : public ManagerType {
    using FinishedMap = std::map<Position, Finished>;

    static const bb::Position genesis_;

    std::atomic_bool batch_ready_;
    std::atomic_int state_machine_triggers_;
    DownloadedData ready_;
    Position best_position_;
    FinishedType best_data_;
    FinishedMap finished_;

    [[maybe_unused]] auto GetFinished(const std::size_t index) const
        -> FinishedMap::value_type
    {
        auto it = finished_.find(generated_positions_.at(index));

        if (finished_.end() == it) { throw std::runtime_error{"bad position"}; }

        return *it;
    }
    [[maybe_unused]] auto GetPosition(std::size_t index) const noexcept
        -> const bb::Position&
    {
        return generated_positions_.at(index);
    }

    [[maybe_unused]] auto GetBatch() noexcept -> BatchType
    {
        auto output = allocate_batch();

        if (output.data_.size() == 0) { batch_ready_ = false; }

        return output;
    }
    [[maybe_unused]] auto MakePositions(
        bb::Height start,
        std::vector<std::string> hashes) noexcept
    {
        auto output = ManagerType::Positions{};
        output.reserve(hashes.size());

        for (const auto& hash : hashes) {
            output.emplace_back(
                start++, ot::Data::Factory(hash, ot::Data::Mode::Raw));
        }

        generated_positions_.insert(
            generated_positions_.end(), output.begin(), output.end());

        return output;
    }
    [[maybe_unused]] auto ProcessData(
        std::optional<std::size_t> items = std::nullopt,
        std::optional<FinishedType> output = std::nullopt) noexcept -> bool
    {
        if (items.has_value()) { EXPECT_EQ(ready_.size(), items.value()); }

        for (const auto& task : ready_) {
            task->process(
                task->previous_.get() + " " +
                std::to_string(task->data_.get()));
            finished_.emplace(task->position_, task->output_);
        }

        if (0 == ready_.size()) { return false; }

        if (output.has_value()) {
            EXPECT_EQ(ready_.back()->output_.get(), output.value());
        }

        ready_.clear();

        return true;
    }
    [[maybe_unused]] auto RunStateMachine() noexcept { return state_machine(); }
    [[maybe_unused]] auto UpdatePosition(
        Positions&& in,
        Previous prior = std::nullopt) noexcept
    {
        update_position(std::move(in), 0, std::move(prior));
    }

    DownloadManager(
        std::size_t batch,
        std::size_t max,
        std::size_t min) noexcept
        : ManagerType(
              genesis_,
              [] {
                  auto promise = std::promise<FinishedType>{};
                  promise.set_value("0");

                  return promise.get_future();
              }(),
              "test",
              max,
              min)
        , batch_ready_(false)
        , state_machine_triggers_(0)
        , ready_()
        , best_position_(genesis_)
        , best_data_("0")
        , finished_()
        , batch_size_(batch)
        , generated_positions_()
    {
    }

private:
    friend ManagerType;

    const std::size_t batch_size_;
    ManagerType::Positions generated_positions_;

    auto batch_size(std::size_t) noexcept -> std::size_t { return batch_size_; }
    auto batch_ready() noexcept -> void { batch_ready_ = true; }
    auto check_task(TaskType&) noexcept -> void {}
    auto queue_processing(DownloadedData&& data) noexcept -> void
    {
        ready_.insert(
            ready_.end(),
            std::make_move_iterator(data.begin()),
            std::make_move_iterator(data.end()));
    }
    auto trigger_state_machine() noexcept -> void { ++state_machine_triggers_; }
    auto update_tip(const Position& pos, const FinishedType& data) noexcept
        -> void
    {
        best_position_ = pos;
        best_data_ = data;
    }
};

const bb::Position DownloadManager::genesis_{
    0,
    ot::Data::Factory("0", ot::Data::Mode::Raw)};
}  // namespace
