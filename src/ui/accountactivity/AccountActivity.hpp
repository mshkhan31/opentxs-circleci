// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <atomic>
#include <functional>
#include <list>
#include <mutex>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "1_Internal.hpp"
#include "Proto.hpp"
#include "core/Worker.hpp"
#include "display/Definition.hpp"
#include "internal/ui/UI.hpp"
#include "opentxs/SharedPimpl.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/api/Core.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/contract/ServerContract.hpp"
#include "opentxs/core/contract/UnitDefinition.hpp"
#include "opentxs/protobuf/PaymentWorkflowEnums.pb.h"
#include "opentxs/ui/AccountActivity.hpp"
#include "opentxs/util/WorkType.hpp"
#include "ui/base/List.hpp"
#include "ui/base/Widget.hpp"
#include "ui/qt/SendMonitor.hpp"
#include "util/Polarity.hpp"
#include "util/Work.hpp"

namespace opentxs
{
namespace api
{
namespace client
{
class Manager;
}  // namespace client
}  // namespace api

namespace identifier
{
class Nym;
}  // namespace identifier

namespace network
{
namespace zeromq
{
namespace socket
{
class Publish;
}  // namespace socket

class Message;
}  // namespace zeromq
}  // namespace network

namespace proto
{
class PaymentEvent;
class PaymentWorkflow;
}  // namespace proto

namespace ui
{
class AmountValidator;
class DestinationValidator;
class DisplayScaleQt;
}  // namespace ui
}  // namespace opentxs

namespace opentxs
{
template <typename T>
struct make_blank;

template <>
struct make_blank<ui::implementation::AccountActivityRowID> {
    static auto value(const api::Core& api)
        -> ui::implementation::AccountActivityRowID
    {
        return {api.Factory().Identifier(), proto::PAYMENTEVENTTYPE_ERROR};
    }
};
}  // namespace opentxs

namespace opentxs::ui::implementation
{
using AccountActivityList = List<
    AccountActivityExternalInterface,
    AccountActivityInternalInterface,
    AccountActivityRowID,
    AccountActivityRowInterface,
    AccountActivityRowInternal,
    AccountActivityRowBlank,
    AccountActivitySortKey,
    AccountActivityPrimaryID>;

/** Show the list of Workflows applicable to this account

    Each row is a BalanceItem which is associated with a Workflow state.

    Some Workflows will only have one entry in the AccountActivity based on
    their type, but others may have multiple entries corresponding to different
    states.
 */
class AccountActivity : public AccountActivityList,
                        protected Worker<AccountActivity>
{
public:
    const display::Definition scales_;

    auto AccountID() const noexcept -> std::string final
    {
        return account_id_->str();
    }
    auto Balance() const noexcept -> Amount final { return balance_.load(); }
    auto BalancePolarity() const noexcept -> int final
    {
        return polarity(balance_.load());
    }
    auto ClearCallbacks() const noexcept -> void final;
    auto Contract() const noexcept -> const contract::Unit& final
    {
        return contract_.get();
    }
    auto DepositAddress() const noexcept -> std::string override
    {
        return DepositAddress(blockchain::Type::Unknown);
    }
    auto DepositAddress(const blockchain::Type) const noexcept
        -> std::string override
    {
        return {};
    }
    auto DepositChains() const noexcept
        -> std::vector<blockchain::Type> override
    {
        return {};
    }
    auto DisplayBalance() const noexcept -> std::string final
    {
        return display_balance(balance_.load());
    }
    auto Notary() const noexcept -> const contract::Server& final
    {
        return notary_.get();
    }
    using ui::AccountActivity::Send;
    auto Send(
        [[maybe_unused]] const Identifier& contact,
        [[maybe_unused]] const Amount amount,
        [[maybe_unused]] const std::string& memo) const noexcept
        -> bool override
    {
        return false;
    }
    auto Send(
        [[maybe_unused]] const Identifier& contact,
        [[maybe_unused]] const std::string& amount,
        [[maybe_unused]] const std::string& memo,
        [[maybe_unused]] Scale scale) const noexcept -> bool override
    {
        return false;
    }
    auto Send(
        [[maybe_unused]] const Identifier& contact,
        [[maybe_unused]] const std::string& amount,
        [[maybe_unused]] const std::string& memo,
        [[maybe_unused]] Scale scale,
        [[maybe_unused]] SendMonitor::Callback cb) const noexcept
        -> int override
    {
        return false;
    }
    auto Send(
        [[maybe_unused]] const std::string& address,
        [[maybe_unused]] const Amount amount,
        [[maybe_unused]] const std::string& memo) const noexcept
        -> bool override
    {
        return false;
    }
    auto Send(
        [[maybe_unused]] const std::string& address,
        [[maybe_unused]] const std::string& amount,
        [[maybe_unused]] const std::string& memo,
        [[maybe_unused]] Scale scale) const noexcept -> bool override
    {
        return false;
    }
    auto Send(
        [[maybe_unused]] const std::string& address,
        [[maybe_unused]] const std::string& amount,
        [[maybe_unused]] const std::string& memo,
        [[maybe_unused]] Scale scale,
        [[maybe_unused]] SendMonitor::Callback cb) const noexcept
        -> int override
    {
        return false;
    }
    auto SendMonitor() const noexcept -> implementation::SendMonitor& final;
    auto SyncPercentage() const noexcept -> double override { return 100; }
    auto SyncProgress() const noexcept -> std::pair<int, int> override
    {
        return {1, 1};
    }
    auto Type() const noexcept -> AccountType final { return type_; }
    auto ValidateAddress([[maybe_unused]] const std::string& text)
        const noexcept -> bool override
    {
        return false;
    }
    auto ValidateAmount([[maybe_unused]] const std::string& text) const noexcept
        -> std::string override
    {
        return {};
    }

    auto AmountValidator() noexcept -> ui::AmountValidator& final;
    auto DestinationValidator() noexcept -> ui::DestinationValidator& final;
    auto DisplayScaleQt() noexcept -> ui::DisplayScaleQt& final;
    auto SendMonitor() noexcept -> implementation::SendMonitor& final;
    auto SetCallbacks(Callbacks&& cb) noexcept -> void final;

    ~AccountActivity() override;

protected:
    struct CallbackHolder {
        mutable std::mutex lock_{};
        Callbacks cb_{};
    };

    mutable CallbackHolder callbacks_;
    mutable std::atomic<Amount> balance_;
    const OTIdentifier account_id_;
    const AccountType type_;
    OTUnitDefinition contract_;
    OTServerContract notary_;

    virtual auto display_balance(opentxs::Amount value) const noexcept
        -> std::string = 0;
    auto notify_balance(opentxs::Amount balance) const noexcept -> void;

    // NOTE only call in final class constructor bodies
    auto init(Endpoints endpoints) noexcept -> void;

    AccountActivity(
        const api::client::Manager& api,
        const identifier::Nym& nymID,
        const Identifier& accountID,
        const AccountType type,
        const SimpleCallback& cb,
        display::Definition&& scales) noexcept;

private:
    friend Worker<AccountActivity>;

    struct QT;

    QT* qt_;

    virtual auto startup() noexcept -> void = 0;

    auto construct_row(
        const AccountActivityRowID& id,
        const AccountActivitySortKey& index,
        CustomData& custom) const noexcept -> RowPointer final;

    auto init_qt() noexcept -> void;
    virtual auto pipeline(const Message& in) noexcept -> void = 0;
    auto shutdown_qt() noexcept -> void;

    AccountActivity() = delete;
    AccountActivity(const AccountActivity&) = delete;
    AccountActivity(AccountActivity&&) = delete;
    auto operator=(const AccountActivity&) -> AccountActivity& = delete;
    auto operator=(AccountActivity&&) -> AccountActivity& = delete;
};
}  // namespace opentxs::ui::implementation
