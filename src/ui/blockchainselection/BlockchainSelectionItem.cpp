// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"    // IWYU pragma: associated
#include "1_Internal.hpp"  // IWYU pragma: associated
#include "ui/blockchainselection/BlockchainSelectionItem.hpp"  // IWYU pragma: associated

#include <memory>

#include "opentxs/Types.hpp"
#include "opentxs/core/Log.hpp"
#include "ui/base/Widget.hpp"

// #define OT_METHOD "opentxs::ui::implementation::BlockchainSelectionItem::"

namespace opentxs::factory
{
auto BlockchainSelectionItem(
    const ui::implementation::BlockchainSelectionInternalInterface& parent,
    const api::client::Manager& api,
    const ui::implementation::BlockchainSelectionRowID& rowID,
    const ui::implementation::BlockchainSelectionSortKey& sortKey,
    ui::implementation::CustomData& custom) noexcept
    -> std::shared_ptr<ui::implementation::BlockchainSelectionRowInternal>
{
    using ReturnType = ui::implementation::BlockchainSelectionItem;

    return std::make_shared<ReturnType>(parent, api, rowID, sortKey, custom);
}
}  // namespace opentxs::factory

namespace opentxs::ui::implementation
{
BlockchainSelectionItem::BlockchainSelectionItem(
    const BlockchainSelectionInternalInterface& parent,
    const api::client::Manager& api,
    const BlockchainSelectionRowID& rowID,
    const BlockchainSelectionSortKey& sortKey,
    CustomData& custom) noexcept
    : BlockchainSelectionItemRow(parent, api, rowID, true)
    , testnet_(sortKey.first)
    , name_(sortKey.second)
    , enabled_(extract_custom<bool>(custom, 0))
{
}

auto BlockchainSelectionItem::reindex(
    const BlockchainSelectionSortKey& key,
    CustomData& custom) noexcept -> bool
{
    OT_ASSERT(testnet_ == key.first);
    OT_ASSERT(name_ == key.second);

    Lock lock{lock_};

    if (auto enabled = extract_custom<bool>(custom, 0); enabled_ != enabled) {
        enabled_ = enabled;

        return true;
    }

    return false;
}

BlockchainSelectionItem::~BlockchainSelectionItem() = default;
}  // namespace opentxs::ui::implementation
