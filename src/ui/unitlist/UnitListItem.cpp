// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                  // IWYU pragma: associated
#include "1_Internal.hpp"                // IWYU pragma: associated
#include "ui/unitlist/UnitListItem.hpp"  // IWYU pragma: associated

#include <memory>

// #define OT_METHOD "opentxs::ui::implementation::UnitListItem::"

namespace opentxs::factory
{
auto UnitListItem(
    const ui::implementation::UnitListInternalInterface& parent,
    const api::client::Manager& api,
    const ui::implementation::UnitListRowID& rowID,
    const ui::implementation::UnitListSortKey& sortKey,
    ui::implementation::CustomData& custom) noexcept
    -> std::shared_ptr<ui::implementation::UnitListRowInternal>
{
    using ReturnType = ui::implementation::UnitListItem;

    return std::make_shared<ReturnType>(parent, api, rowID, sortKey, custom);
}
}  // namespace opentxs::factory

namespace opentxs::ui::implementation
{
UnitListItem::UnitListItem(
    const UnitListInternalInterface& parent,
    const api::client::Manager& api,
    const UnitListRowID& rowID,
    const UnitListSortKey& sortKey,
    [[maybe_unused]] CustomData& custom) noexcept
    : UnitListItemRow(parent, api, rowID, true)
    , name_(sortKey)
{
}
}  // namespace opentxs::ui::implementation
