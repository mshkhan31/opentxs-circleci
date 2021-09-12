// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Types.hpp"

namespace opentxs
{
template <typename I>
struct HDIndex {
    Bip32Index value_{};

    operator Bip32Index() const { return value_; }

    HDIndex(const I in)
        : value_(static_cast<Bip32Index>(in))
    {
    }

    HDIndex(const I lhs, const Bip32Child rhs)
        : value_(static_cast<Bip32Index>(lhs) | static_cast<Bip32Index>(rhs))
    {
    }
};

template <typename Bip43Purpose>
HDIndex(const Bip43Purpose, const Bip32Child) -> HDIndex<Bip43Purpose>;
}  // namespace opentxs
