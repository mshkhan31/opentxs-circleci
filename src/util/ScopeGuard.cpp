// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"         // IWYU pragma: associated
#include "1_Internal.hpp"       // IWYU pragma: associated
#include "util/ScopeGuard.hpp"  // IWYU pragma: associated

namespace opentxs
{
ScopeGuard::ScopeGuard(SimpleCallback cb) noexcept
    : cb_(cb)
{
}

ScopeGuard::~ScopeGuard()
{
    if (cb_) { cb_(); }
}
}  // namespace opentxs
