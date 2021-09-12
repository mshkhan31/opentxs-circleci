// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"    // IWYU pragma: associated
#include "1_Internal.hpp"  // IWYU pragma: associated
#include "ui/activitythread/BlockchainActivityThreadItem.hpp"  // IWYU pragma: associated

#include <memory>
#include <utility>

#include "opentxs/Pimpl.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/api/client/Manager.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "ui/activitythread/ActivityThreadItem.hpp"
#include "ui/base/Widget.hpp"

// #define OT_METHOD
// "opentxs::ui::implementation::BlockchainActivityThreadItem::"

namespace opentxs::factory
{
auto BlockchainActivityThreadItem(
    const ui::implementation::ActivityThreadInternalInterface& parent,
    const api::client::Manager& api,
    const identifier::Nym& nymID,
    const ui::implementation::ActivityThreadRowID& rowID,
    const ui::implementation::ActivityThreadSortKey& sortKey,
    ui::implementation::CustomData& custom) noexcept
    -> std::shared_ptr<ui::implementation::ActivityThreadRowInternal>
{
    using ReturnType = ui::implementation::BlockchainActivityThreadItem;

    auto [txid, amount, display, memo] =
        ReturnType::extract(api, nymID, custom);

    return std::make_shared<ReturnType>(
        parent,
        api,
        nymID,
        rowID,
        sortKey,
        custom,
        std::move(txid),
        amount,
        std::move(display),
        std::move(memo));
}
}  // namespace opentxs::factory

namespace opentxs::ui::implementation
{
BlockchainActivityThreadItem::BlockchainActivityThreadItem(
    const ActivityThreadInternalInterface& parent,
    const api::client::Manager& api,
    const identifier::Nym& nymID,
    const ActivityThreadRowID& rowID,
    const ActivityThreadSortKey& sortKey,
    CustomData& custom,
    OTData&& txid,
    opentxs::Amount amount,
    std::string&& displayAmount,
    std::string&& memo) noexcept
    : ActivityThreadItem(parent, api, nymID, rowID, sortKey, custom)
    , txid_(std::move(txid))
    , display_amount_(std::move(displayAmount))
    , memo_(std::move(memo))
    , amount_(amount)
{
    OT_ASSERT(false == nym_id_.empty())
    OT_ASSERT(false == item_id_.empty())
    OT_ASSERT(false == txid_->empty())
}

auto BlockchainActivityThreadItem::Amount() const noexcept -> opentxs::Amount
{
    auto lock = sLock{shared_lock_};

    return amount_;
}

auto BlockchainActivityThreadItem::DisplayAmount() const noexcept -> std::string
{
    auto lock = sLock{shared_lock_};

    return display_amount_;
}

auto BlockchainActivityThreadItem::extract(
    const api::client::Manager& api,
    const identifier::Nym& nymID,
    CustomData& custom) noexcept
    -> std::tuple<OTData, opentxs::Amount, std::string, std::string>
{
    return std::tuple<OTData, opentxs::Amount, std::string, std::string>{
        api.Factory().Data(
            ui::implementation::extract_custom<std::string>(custom, 5),
            StringStyle::Raw),
        ui::implementation::extract_custom<opentxs::Amount>(custom, 6),
        ui::implementation::extract_custom<std::string>(custom, 7),
        ui::implementation::extract_custom<std::string>(custom, 8)};
}

auto BlockchainActivityThreadItem::Memo() const noexcept -> std::string
{
    auto lock = sLock{shared_lock_};

    return memo_;
}

auto BlockchainActivityThreadItem::reindex(
    const ActivityThreadSortKey& key,
    CustomData& custom) noexcept -> bool
{
    auto [txid, amount, display, memo] = extract(api_, nym_id_, custom);
    auto output = ActivityThreadItem::reindex(key, custom);

    OT_ASSERT(txid_ == txid);

    auto lock = eLock{shared_lock_};

    if (display_amount_ != display) {
        display_amount_ = std::move(display);
        output = true;
    }

    if (memo_ != memo) {
        memo_ = std::move(memo);
        output = true;
    }

    if (amount_ != amount) {
        amount_ = amount;
        output = true;
    }

    return output;
}
}  // namespace opentxs::ui::implementation
