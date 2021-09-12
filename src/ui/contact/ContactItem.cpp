// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                // IWYU pragma: associated
#include "1_Internal.hpp"              // IWYU pragma: associated
#include "ui/contact/ContactItem.hpp"  // IWYU pragma: associated

#include "opentxs/contact/ContactItem.hpp"
#include "opentxs/core/Log.hpp"
#include "ui/base/Widget.hpp"

namespace opentxs::factory
{
auto ContactItemWidget(
    const ui::implementation::ContactSubsectionInternalInterface& parent,
    const api::client::Manager& api,
    const ui::implementation::ContactSubsectionRowID& rowID,
    const ui::implementation::ContactSubsectionSortKey& sortKey,
    ui::implementation::CustomData& custom) noexcept
    -> std::shared_ptr<ui::implementation::ContactSubsectionRowInternal>
{
    using ReturnType = ui::implementation::ContactItem;

    return std::make_shared<ReturnType>(parent, api, rowID, sortKey, custom);
}
}  // namespace opentxs::factory

namespace opentxs::ui::implementation
{
ContactItem::ContactItem(
    const ContactSubsectionInternalInterface& parent,
    const api::client::Manager& api,
    const ContactSubsectionRowID& rowID,
    const ContactSubsectionSortKey& sortKey,
    CustomData& custom) noexcept
    : ContactItemRow(parent, api, rowID, true)
    , item_{new opentxs::ContactItem(
          extract_custom<opentxs::ContactItem>(custom))}
{
    OT_ASSERT(item_);
}

auto ContactItem::reindex(
    const ContactSubsectionSortKey&,
    CustomData& custom) noexcept -> bool
{
    eLock lock(shared_lock_);
    item_.reset(
        new opentxs::ContactItem(extract_custom<opentxs::ContactItem>(custom)));

    OT_ASSERT(item_);

    return true;
}
}  // namespace opentxs::ui::implementation
