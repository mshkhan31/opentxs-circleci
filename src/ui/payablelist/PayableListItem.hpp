// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <string>

#include "1_Internal.hpp"
#include "Proto.hpp"
#include "internal/ui/UI.hpp"
#include "opentxs/SharedPimpl.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/contact/ContactItemType.hpp"
#include "ui/contactlist/ContactListItem.hpp"

class QVariant;

namespace opentxs
{
namespace api
{
namespace client
{
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
class PayableListItem;
}  // namespace ui
}  // namespace opentxs

namespace opentxs::ui::implementation
{
class PayableListItem final : public PayableListRowInternal,
                              public implementation::ContactListItem
{
public:
    auto PaymentCode() const noexcept -> std::string final;

    PayableListItem(
        const PayableInternalInterface& parent,
        const api::client::Manager& api,
        const PayableListRowID& rowID,
        const PayableListSortKey& key,
        const std::string& paymentcode,
        const contact::ContactItemType& currency) noexcept;
    ~PayableListItem() final = default;

private:
    using ot_super = implementation::ContactListItem;

    std::string payment_code_;
    const contact::ContactItemType currency_;

    auto calculate_section(const Lock& lock) const noexcept -> std::string final
    {
        return translate_section(lock);
    }
    auto qt_data(const int column, const int role, QVariant& out) const noexcept
        -> void final;

    auto reindex(const PayableListSortKey&, CustomData&) noexcept -> bool final;
    auto reindex(
        const Lock&,
        const PayableListSortKey& key,
        CustomData& custom) noexcept -> bool final;

    PayableListItem() = delete;
    PayableListItem(const PayableListItem&) = delete;
    PayableListItem(PayableListItem&&) = delete;
    auto operator=(const PayableListItem&) -> PayableListItem& = delete;
    auto operator=(PayableListItem&&) -> PayableListItem& = delete;
};
}  // namespace opentxs::ui::implementation

template class opentxs::SharedPimpl<opentxs::ui::PayableListItem>;
