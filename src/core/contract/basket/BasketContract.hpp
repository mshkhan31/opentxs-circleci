// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstdint>
#include <string>

#include "Proto.hpp"
#include "core/contract/UnitDefinition.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/contact/ContactItemType.hpp"
#include "opentxs/contact/Types.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/contract/UnitType.hpp"
#include "opentxs/core/contract/basket/BasketContract.hpp"
#include "opentxs/protobuf/UnitDefinition.pb.h"

namespace opentxs
{
namespace api
{
class Core;
}  // namespace api

class PasswordPrompt;
}  // namespace opentxs

namespace opentxs::contract::unit::implementation
{
class Basket final : public unit::Basket, public contract::implementation::Unit
{
public:
    auto BasketID() const -> OTIdentifier final;
    auto Currencies() const -> const Subcontracts& final
    {
        return subcontracts_;
    }
    auto Type() const -> contract::UnitType final
    {
        return contract::UnitType::Basket;
    }
    auto Weight() const -> std::uint64_t final { return weight_; }

    Basket(
        const api::Core& api,
        const Nym_p& nym,
        const std::string& shortname,
        const std::string& name,
        const std::string& symbol,
        const std::string& terms,
        const std::uint64_t weight,
        const contact::ContactItemType unitOfAccount,
        const VersionNumber version);
    Basket(
        const api::Core& api,
        const Nym_p& nym,
        const proto::UnitDefinition serialized);

    ~Basket() final = default;

private:
    friend unit::Basket;

    Subcontracts subcontracts_;
    std::uint64_t weight_;

    auto BasketIDVersion(const Lock& lock) const -> proto::UnitDefinition;
    auto clone() const noexcept -> Basket* final { return new Basket(*this); }
    auto IDVersion(const Lock& lock) const -> proto::UnitDefinition final;

    Basket(const Basket&);
    Basket(Basket&&) = delete;
    auto operator=(const Basket&) -> Basket& = delete;
    auto operator=(Basket&&) -> Basket& = delete;
};
}  // namespace opentxs::contract::unit::implementation
