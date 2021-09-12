// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                          // IWYU pragma: associated
#include "1_Internal.hpp"                        // IWYU pragma: associated
#include "network/zeromq/PairEventListener.hpp"  // IWYU pragma: associated

#include <memory>

#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/ListenCallback.hpp"
#include "opentxs/network/zeromq/PairEventCallback.hpp"

#define PAIR_EVENT_ENDPOINT_PATH "pairevent"
#define PAIR_EVENT_ENDPOINT_VERSION 1

#define OT_METHOD                                                              \
    "opentxs::network::zeromq::implementation::PairEventListener::"

namespace opentxs::network::zeromq::implementation
{
PairEventListener::PairEventListener(
    const zeromq::Context& context,
    const zeromq::PairEventCallback& callback,
    const int instance)
    : ot_super(context, callback)
    , instance_(instance)
{
    const auto endpoint = context_.BuildEndpoint(
        PAIR_EVENT_ENDPOINT_PATH, instance, PAIR_EVENT_ENDPOINT_VERSION);
    const bool started = Start(endpoint);

    OT_ASSERT(started)

    LogVerbose(OT_METHOD)(__func__)(": listening on ")(endpoint).Flush();
}

auto PairEventListener::clone() const noexcept -> PairEventListener*
{
    return new PairEventListener(
        context_,
        dynamic_cast<const zeromq::PairEventCallback&>(callback_),
        instance_);
}
}  // namespace opentxs::network::zeromq::implementation
