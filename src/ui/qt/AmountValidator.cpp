// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                       // IWYU pragma: associated
#include "1_Internal.hpp"                     // IWYU pragma: associated
#include "opentxs/ui/qt/AmountValidator.hpp"  // IWYU pragma: associated

#include "opentxs/core/Log.hpp"
#include "ui/qt/AmountValidator.hpp"

// #define OT_METHOD "opentxs::ui::AmountValidator::"

namespace opentxs::ui
{
AmountValidator::AmountValidator(
    implementation::AccountActivity& parent) noexcept
    : imp_(std::make_unique<Imp>(parent))
{
    OT_ASSERT(imp_);
}

auto AmountValidator::fixup(QString& input) const -> void
{
    imp_->fixup(input);
}

auto AmountValidator::getMaxDecimals() const -> int
{
    return imp_->getMaxDecimals();
}

auto AmountValidator::getMinDecimals() const -> int
{
    return imp_->getMinDecimals();
}

auto AmountValidator::getScale() const -> int { return imp_->getScale(); }

auto AmountValidator::revise(const QString& input, int previous) const
    -> QString
{
    return imp_->revise(input, previous);
}

auto AmountValidator::setMaxDecimals(int value) -> void
{
    if (imp_->setMaxDecimals(value)) { emit scaleChanged(imp_->scale_.load()); }
}

auto AmountValidator::setMinDecimals(int value) -> void
{
    if (imp_->setMinDecimals(value)) { emit scaleChanged(imp_->scale_.load()); }
}

auto AmountValidator::setScale(int value) -> void
{
    auto old = int{};

    if (imp_->setScale(value, old)) { emit scaleChanged(old); }
}

auto AmountValidator::validate(QString& input, int& pos) const -> State
{
    return imp_->validate(input, pos);
}

AmountValidator::~AmountValidator() = default;
}  // namespace opentxs::ui
