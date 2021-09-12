// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_UI_MESSAGABLELIST_HPP
#define OPENTXS_UI_MESSAGABLELIST_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include "opentxs/SharedPimpl.hpp"
#include "opentxs/ui/List.hpp"

namespace opentxs
{
namespace ui
{
class ContactListItem;
class MessagableList;
}  // namespace ui
}  // namespace opentxs

namespace opentxs
{
namespace ui
{
class OPENTXS_EXPORT MessagableList : virtual public List
{
public:
    virtual auto First() const noexcept
        -> opentxs::SharedPimpl<opentxs::ui::ContactListItem> = 0;
    virtual auto Next() const noexcept
        -> opentxs::SharedPimpl<opentxs::ui::ContactListItem> = 0;

    ~MessagableList() override = default;

protected:
    MessagableList() noexcept = default;

private:
    MessagableList(const MessagableList&) = delete;
    MessagableList(MessagableList&&) = delete;
    auto operator=(const MessagableList&) -> MessagableList& = delete;
    auto operator=(MessagableList&&) -> MessagableList& = delete;
};
}  // namespace ui
}  // namespace opentxs
#endif
