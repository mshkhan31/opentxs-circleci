// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                       // IWYU pragma: associated
#include "1_Internal.hpp"                     // IWYU pragma: associated
#include "ui/activitythread/PendingSend.hpp"  // IWYU pragma: associated

#include <memory>
#include <utility>

#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "ui/activitythread/ActivityThreadItem.hpp"
#include "ui/base/Widget.hpp"

//#define OT_METHOD "opentxs::ui::implementation::PendingSend::"

namespace opentxs::factory
{
auto PendingSend(
    const ui::implementation::ActivityThreadInternalInterface& parent,
    const api::client::Manager& api,
    const identifier::Nym& nymID,
    const ui::implementation::ActivityThreadRowID& rowID,
    const ui::implementation::ActivityThreadSortKey& sortKey,
    ui::implementation::CustomData& custom) noexcept
    -> std::shared_ptr<ui::implementation::ActivityThreadRowInternal>
{
    using ReturnType = ui::implementation::PendingSend;
    auto [amount, display, memo] = ReturnType::extract(custom);

    return std::make_shared<ReturnType>(
        parent,
        api,
        nymID,
        rowID,
        sortKey,
        custom,
        amount,
        std::move(display),
        std::move(memo));
}
}  // namespace opentxs::factory

namespace opentxs::ui::implementation
{
PendingSend::PendingSend(
    const ActivityThreadInternalInterface& parent,
    const api::client::Manager& api,
    const identifier::Nym& nymID,
    const ActivityThreadRowID& rowID,
    const ActivityThreadSortKey& sortKey,
    CustomData& custom,
    opentxs::Amount amount,
    std::string&& display,
    std::string&& memo) noexcept
    : ActivityThreadItem(parent, api, nymID, rowID, sortKey, custom)
    , amount_(amount)
    , display_amount_(std::move(display))
    , memo_(std::move(memo))
{
    OT_ASSERT(false == nym_id_.empty())
    OT_ASSERT(false == item_id_.empty())
}

auto PendingSend::Amount() const noexcept -> opentxs::Amount
{
    auto lock = sLock{shared_lock_};

    return amount_;
}

auto PendingSend::DisplayAmount() const noexcept -> std::string
{
    auto lock = sLock{shared_lock_};

    return display_amount_;
}

auto PendingSend::extract(CustomData& custom) noexcept
    -> std::tuple<opentxs::Amount, std::string, std::string>
{
    return std::make_tuple(
        extract_custom<opentxs::Amount>(custom, 5),
        extract_custom<std::string>(custom, 6),
        extract_custom<std::string>(custom, 7));
}

auto PendingSend::Memo() const noexcept -> std::string
{
    auto lock = sLock{shared_lock_};

    return memo_;
}

auto PendingSend::reindex(
    const ActivityThreadSortKey& key,
    CustomData& custom) noexcept -> bool
{
    auto output = ActivityThreadItem::reindex(key, custom);
    auto [amount, display, memo] = extract(custom);
    auto lock = eLock{shared_lock_};

    if (amount_ != amount) {
        amount_ = amount;
        output = true;
    }

    if (display_amount_ != display) {
        display_amount_ = std::move(display);
        output = true;
    }

    if (memo_ != memo) {
        memo_ = std::move(memo);
        output = true;
    }

    return output;
}
}  // namespace opentxs::ui::implementation
