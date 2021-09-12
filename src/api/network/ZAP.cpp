// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"         // IWYU pragma: associated
#include "1_Internal.hpp"       // IWYU pragma: associated
#include "api/network/ZAP.hpp"  // IWYU pragma: associated

#include "2_Factory.hpp"
#include "opentxs/api/network/ZAP.hpp"
#include "opentxs/network/zeromq/zap/Callback.hpp"
#include "opentxs/network/zeromq/zap/Handler.hpp"

//#define OT_METHOD "opentxs::api::network::ZAP::"

namespace opentxs
{
namespace network
{
namespace zeromq
{
class Context;
}  // namespace zeromq
}  // namespace network

auto Factory::ZAP(const network::zeromq::Context& context) -> api::network::ZAP*
{
    return new api::network::implementation::ZAP(context);
}
}  // namespace opentxs

namespace opentxs::api::network::implementation
{
ZAP::ZAP(const opentxs::network::zeromq::Context& context)
    : context_(context)
    , callback_(opentxs::network::zeromq::zap::Callback::Factory())
    , zap_(opentxs::network::zeromq::zap::Handler::Factory(context_, callback_))
{
}

auto ZAP::RegisterDomain(const std::string& domain, const Callback& callback)
    const -> bool
{
    return callback_->SetDomain(domain, callback);
}

auto ZAP::SetDefaultPolicy(const Policy policy) const -> bool
{
    return callback_->SetPolicy(policy);
}
}  // namespace opentxs::api::network::implementation
