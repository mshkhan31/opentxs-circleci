// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <atomic>
#include <iosfwd>
#include <memory>
#include <string>

#include "1_Internal.hpp"
#include "Proto.hpp"
#include "internal/ui/UI.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/SharedPimpl.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/contract/ServerContract.hpp"
#include "opentxs/core/contract/UnitDefinition.hpp"
#include "opentxs/protobuf/ContactEnums.pb.h"
#include "opentxs/ui/BlockchainSelectionItem.hpp"
#include "ui/base/Row.hpp"

class QVariant;

namespace opentxs
{
namespace api
{
namespace client
{
namespace internal
{
struct Blockchain;
}  // namespace internal

class Manager;
}  // namespace client
}  // namespace api

namespace network
{
namespace zeromq
{
namespace socket
{
class Publish;
}  // namespace socket
}  // namespace zeromq
}  // namespace network

namespace ui
{
class BlockchainSelectionItem;
}  // namespace ui
}  // namespace opentxs

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wnon-virtual-dtor"
namespace opentxs::ui::implementation
{
using BlockchainSelectionItemRow =
    Row<BlockchainSelectionRowInternal,
        BlockchainSelectionInternalInterface,
        BlockchainSelectionRowID>;

class BlockchainSelectionItem final
    : public BlockchainSelectionItemRow,
      public std::enable_shared_from_this<BlockchainSelectionItem>
{
public:
    auto Name() const noexcept -> std::string final { return name_; }
    auto IsEnabled() const noexcept -> bool final { return enabled_; }
    auto IsTestnet() const noexcept -> bool final { return testnet_; }
    auto Type() const noexcept -> blockchain::Type final { return row_id_; }

    BlockchainSelectionItem(
        const BlockchainSelectionInternalInterface& parent,
        const api::client::Manager& api,
        const BlockchainSelectionRowID& rowID,
        const BlockchainSelectionSortKey& sortKey,
        CustomData& custom) noexcept;

    ~BlockchainSelectionItem() final;

private:
    const bool testnet_;
    const std::string name_;
    std::atomic_bool enabled_;

    auto qt_data(const int column, const int role, QVariant& out) const noexcept
        -> void final;

    auto reindex(const BlockchainSelectionSortKey&, CustomData&) noexcept
        -> bool final;

    BlockchainSelectionItem() = delete;
    BlockchainSelectionItem(const BlockchainSelectionItem&) = delete;
    BlockchainSelectionItem(BlockchainSelectionItem&&) = delete;
    auto operator=(const BlockchainSelectionItem&)
        -> BlockchainSelectionItem& = delete;
    auto operator=(BlockchainSelectionItem&&)
        -> BlockchainSelectionItem& = delete;
};
}  // namespace opentxs::ui::implementation
#pragma GCC diagnostic pop

template class opentxs::SharedPimpl<opentxs::ui::BlockchainSelectionItem>;
