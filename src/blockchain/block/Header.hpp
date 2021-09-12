// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <map>
#include <memory>

#include "opentxs/Types.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/NumericHash.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/Work.hpp"
#include "opentxs/blockchain/block/Header.hpp"
#include "opentxs/blockchain/block/bitcoin/Header.hpp"
#include "opentxs/core/Data.hpp"

namespace opentxs
{
namespace api
{
class Core;
}  // namespace api

namespace blockchain
{
namespace block
{
namespace bitcoin
{
class Header;
}  // namespace bitcoin
}  // namespace block
}  // namespace blockchain
}  // namespace opentxs

namespace opentxs::blockchain::block::implementation
{
class Header : virtual public block::Header
{
public:
    auto as_Bitcoin() const noexcept
        -> std::unique_ptr<bitcoin::Header> override
    {
        return {};
    }
    auto Difficulty() const noexcept -> OTWork final { return work_; }
    auto EffectiveState() const noexcept -> Status final;
    auto Hash() const noexcept -> const block::Hash& final;
    auto Height() const noexcept -> block::Height final;
    auto IncrementalWork() const noexcept -> OTWork final { return work_; }
    auto InheritedState() const noexcept -> Status final;
    auto IsBlacklisted() const noexcept -> bool final;
    auto IsDisconnected() const noexcept -> bool final;
    auto LocalState() const noexcept -> Status final;
    auto NumericHash() const noexcept -> OTNumericHash final;
    auto ParentHash() const noexcept -> const block::Hash& final;
    auto ParentWork() const noexcept -> OTWork final { return inherit_work_; }
    auto Position() const noexcept -> block::Position final;
    using block::Header::Serialize;
    auto Serialize(SerializedType& out) const noexcept -> bool override;
    auto Type() const noexcept -> blockchain::Type final { return type_; }
    auto Valid() const noexcept -> bool final;
    auto Work() const noexcept -> OTWork final;

    void CompareToCheckpoint(const block::Position& checkpoint) noexcept final;
    void InheritHeight(const block::Header& parent) final;
    void InheritState(const block::Header& parent) final;
    void InheritWork(const blockchain::Work& work) noexcept final;
    void RemoveBlacklistState() noexcept final;
    void RemoveCheckpointState() noexcept final;
    void SetDisconnectedState() noexcept final;

    ~Header() override = default;

protected:
    static const VersionNumber default_version_{1};

    const api::Core& api_;
    const OTData hash_;
    const OTData pow_;
    const OTData parent_hash_;
    const blockchain::Type type_;

    static auto minimum_work(const blockchain::Type chain) -> OTWork;

    Header(
        const api::Core& api,
        const VersionNumber version,
        const blockchain::Type type,
        block::pHash&& hash,
        block::pHash&& pow,
        block::pHash&& parentHash,
        const block::Height height,
        const Status status,
        const Status inheritStatus,
        const blockchain::Work& work,
        const blockchain::Work& inheritWork) noexcept;
    Header(const Header& rhs) noexcept;

private:
    static const VersionNumber local_data_version_{1};

    const VersionNumber version_;
    const OTWork work_;
    block::Height height_;
    Status status_;
    Status inherit_status_;
    OTWork inherit_work_;

    Header() = delete;
    Header(Header&&) = delete;
    auto operator=(const Header&) -> Header& = delete;
    auto operator=(Header&&) -> Header& = delete;
};
}  // namespace opentxs::blockchain::block::implementation
