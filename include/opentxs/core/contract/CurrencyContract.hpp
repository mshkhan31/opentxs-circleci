// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CORE_CONTRACT_CURRENCYCONTRACT_HPP
#define OPENTXS_CORE_CONTRACT_CURRENCYCONTRACT_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include "opentxs/SharedPimpl.hpp"
#include "opentxs/core/contract/UnitDefinition.hpp"

namespace opentxs
{
namespace contract
{
namespace unit
{
class Currency;
}  // namespace unit
}  // namespace contract

using OTCurrencyContract = SharedPimpl<contract::unit::Currency>;
}  // namespace opentxs

namespace opentxs
{
namespace contract
{
namespace unit
{
class OPENTXS_EXPORT Currency : virtual public contract::Unit
{
public:
    ~Currency() override = default;

protected:
    Currency() noexcept = default;

private:
    friend OTCurrencyContract;

#ifndef _WIN32
    auto clone() const noexcept -> Currency* override = 0;
#endif

    Currency(const Currency&) = delete;
    Currency(Currency&&) = delete;
    auto operator=(const Currency&) -> Currency& = delete;
    auto operator=(Currency&&) -> Currency& = delete;
};
}  // namespace unit
}  // namespace contract
}  // namespace opentxs
#endif
