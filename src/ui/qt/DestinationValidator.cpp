// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                            // IWYU pragma: associated
#include "1_Internal.hpp"                          // IWYU pragma: associated
#include "opentxs/ui/qt/DestinationValidator.hpp"  // IWYU pragma: associated

#include <algorithm>
#include <cctype>
#include <string>

#include "opentxs/Types.hpp"
#include "opentxs/core/Log.hpp"
#include "ui/qt/DestinationValidator.hpp"

// #define OT_METHOD "opentxs::ui::DestinationValidator::"

namespace opentxs::ui
{
auto DestinationValidator::Imp::strip_invalid(
    QString& input,
    bool cashaddr) noexcept -> void
{
    const auto test = [&](const char c) {
        return (false == std::isalnum(c)) && (cashaddr ? (c != ':') : true);
    };
    auto raw = input.toStdString();
    raw.erase(std::remove_if(raw.begin(), raw.end(), test), raw.end());
    input = QString{raw.c_str()};
}

DestinationValidator::DestinationValidator(
    const api::client::Manager& api,
    std::int8_t type,
    const Identifier& account,
    implementation::AccountActivity& parent) noexcept
    : imp_(
          (AccountType::Blockchain == static_cast<AccountType>(type))
              ? Imp::Blockchain(api, *this, account, parent)
              : Imp::Custodial(api, parent))
{
    OT_ASSERT(imp_);
}

auto DestinationValidator::fixup(QString& input) const -> void
{
    imp_->fixup(input);
}

auto DestinationValidator::getDetails() const -> QString
{
    return imp_->getDetails();
}

auto DestinationValidator::validate(QString& input, int& pos) const -> State
{
    return imp_->validate(input, pos);
}

DestinationValidator::~DestinationValidator() = default;
}  // namespace opentxs::ui
