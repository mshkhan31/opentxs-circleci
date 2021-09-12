// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CORE_FLAG_HPP
#define OPENTXS_CORE_FLAG_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include "opentxs/Pimpl.hpp"

namespace opentxs
{
class Flag;

using OTFlag = Pimpl<Flag>;
}  // namespace opentxs

namespace opentxs
{
/** Wrapper for a std::atomic<bool> */
class OPENTXS_EXPORT Flag
{
public:
    static auto Factory(const bool state) -> OTFlag;

    virtual operator bool() const = 0;

    /** Returns true if new state differs from previous state */
    virtual auto Off() -> bool = 0;
    /** Returns true if new state differs from previous state */
    virtual auto On() -> bool = 0;
    /** Returns previous state */
    virtual auto Set(const bool value) -> bool = 0;
    /** Returns previous state */
    virtual auto Toggle() -> bool = 0;

    virtual ~Flag() = default;

protected:
    Flag() = default;

private:
    friend OTFlag;

    virtual auto clone() const -> Flag* = 0;

    Flag(const Flag&) = delete;
    Flag(Flag&&) = delete;
    auto operator=(const Flag&) -> Flag& = delete;
    auto operator=(Flag&&) -> Flag& = delete;
};
}  // namespace opentxs
#endif
