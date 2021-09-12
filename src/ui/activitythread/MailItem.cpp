// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                    // IWYU pragma: associated
#include "1_Internal.hpp"                  // IWYU pragma: associated
#include "ui/activitythread/MailItem.hpp"  // IWYU pragma: associated

#include <memory>

#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "ui/activitythread/ActivityThreadItem.hpp"

namespace opentxs::factory
{
auto MailItem(
    const ui::implementation::ActivityThreadInternalInterface& parent,
    const api::client::Manager& api,
    const identifier::Nym& nymID,
    const ui::implementation::ActivityThreadRowID& rowID,
    const ui::implementation::ActivityThreadSortKey& sortKey,
    ui::implementation::CustomData& custom) noexcept
    -> std::shared_ptr<ui::implementation::ActivityThreadRowInternal>
{
    using ReturnType = ui::implementation::MailItem;

    return std::make_shared<ReturnType>(
        parent, api, nymID, rowID, sortKey, custom);
}
}  // namespace opentxs::factory

namespace opentxs::ui::implementation
{
MailItem::MailItem(
    const ActivityThreadInternalInterface& parent,
    const api::client::Manager& api,
    const identifier::Nym& nymID,
    const ActivityThreadRowID& rowID,
    const ActivityThreadSortKey& sortKey,
    CustomData& custom) noexcept
    : ActivityThreadItem(parent, api, nymID, rowID, sortKey, custom)
{
    OT_ASSERT(false == nym_id_.empty());
    OT_ASSERT(false == item_id_.empty())
}

MailItem::~MailItem() = default;
}  // namespace opentxs::ui::implementation
