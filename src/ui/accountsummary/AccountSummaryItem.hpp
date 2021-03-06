// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <atomic>
#include <string>

#include "1_Internal.hpp"
#include "Proto.hpp"
#include "internal/ui/UI.hpp"
#include "opentxs/SharedPimpl.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/contact/ContactItemType.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/contract/UnitDefinition.hpp"
#include "opentxs/ui/AccountSummaryItem.hpp"
#include "ui/base/Row.hpp"

class QVariant;

namespace opentxs
{
namespace api
{
namespace client
{
class Manager;
}  // namespace client

class Core;
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
class AccountSummaryItem;
}  // namespace ui
}  // namespace opentxs

namespace opentxs::ui::implementation
{
using AccountSummaryItemRow =
    Row<IssuerItemRowInternal, IssuerItemInternalInterface, IssuerItemRowID>;

class AccountSummaryItem final : public AccountSummaryItemRow
{
public:
    auto AccountID() const noexcept -> std::string final
    {
        return account_id_.str();
    }
    auto Balance() const noexcept -> Amount final { return balance_.load(); }
    auto DisplayBalance() const noexcept -> std::string final;
    auto Name() const noexcept -> std::string final;

    AccountSummaryItem(
        const IssuerItemInternalInterface& parent,
        const api::client::Manager& api,
        const IssuerItemRowID& rowID,
        const IssuerItemSortKey& sortKey,
        CustomData& custom) noexcept;

    ~AccountSummaryItem() final = default;

private:
    const Identifier& account_id_;
    const contact::ContactItemType& currency_;
    mutable std::atomic<Amount> balance_;
    IssuerItemSortKey name_;
    mutable OTUnitDefinition contract_;

    static auto load_unit(const api::Core& api, const Identifier& id)
        -> OTUnitDefinition;

    auto qt_data(const int column, const int role, QVariant& out) const noexcept
        -> void final;

    auto reindex(const IssuerItemSortKey& key, CustomData& custom) noexcept
        -> bool final;

    AccountSummaryItem() = delete;
    AccountSummaryItem(const AccountSummaryItem&) = delete;
    AccountSummaryItem(AccountSummaryItem&&) = delete;
    auto operator=(const AccountSummaryItem&) -> AccountSummaryItem& = delete;
    auto operator=(AccountSummaryItem&&) -> AccountSummaryItem& = delete;
};
}  // namespace opentxs::ui::implementation

template class opentxs::SharedPimpl<opentxs::ui::AccountSummaryItem>;
