// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Types.hpp"
#include "opentxs/core/Flag.hpp"
#include "opentxs/otx/consensus/ManagedNumber.hpp"

namespace opentxs
{
namespace otx
{
namespace context
{
class Server;
}  // namespace context
}  // namespace otx
}  // namespace opentxs

namespace opentxs::otx::context::implementation
{
class ManagedNumber final : virtual public otx::context::ManagedNumber
{
public:
    void SetSuccess(const bool value = true) const final;
    auto Valid() const -> bool final;
    auto Value() const -> TransactionNumber final;

    ManagedNumber(const TransactionNumber number, otx::context::Server&);
    ~ManagedNumber() final;

private:
    otx::context::Server& context_;
    const TransactionNumber number_;
    mutable OTFlag success_;
    bool managed_{true};

    ManagedNumber() = delete;
    ManagedNumber(const ManagedNumber&) = delete;
    ManagedNumber(ManagedNumber&& rhs) = delete;
    auto operator=(const ManagedNumber&) -> ManagedNumber& = delete;
    auto operator=(ManagedNumber&&) -> ManagedNumber& = delete;
};
}  // namespace opentxs::otx::context::implementation
