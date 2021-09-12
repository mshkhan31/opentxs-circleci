// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <algorithm>
#include <atomic>
#include <cstring>
#include <functional>
#include <future>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "core/StateMachine.hpp"
#include "internal/api/Api.hpp"
#include "internal/api/client/Client.hpp"
#include "internal/otx/client/Client.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/SharedPimpl.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/api/Core.hpp"
#include "opentxs/api/client/Manager.hpp"
#include "opentxs/api/client/OTX.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/PasswordPrompt.hpp"
#include "opentxs/core/UniqueQueue.hpp"
#include "opentxs/core/contract/peer/PeerReply.hpp"
#include "opentxs/core/contract/peer/PeerRequest.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/core/identifier/Server.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"
#include "opentxs/ext/OTPayment.hpp"
#include "otx/client/PaymentTasks.hpp"

namespace opentxs
{
namespace otx
{
namespace context
{
class Server;
}  // namespace context
}  // namespace otx

class Flag;
class OTPassword;
class Secret;
}  // namespace opentxs

namespace std
{
using MESSAGETASK = tuple<opentxs::OTNymID, string, opentxs::SetID>;
using PAYMENTTASK =
    pair<opentxs::OTIdentifier, shared_ptr<const opentxs::OTPayment>>;
using PEERREPLYTASK =
    tuple<opentxs::OTNymID, opentxs::OTPeerReply, opentxs::OTPeerRequest>;
using PEERREQUESTTASK = pair<opentxs::OTNymID, opentxs::OTPeerRequest>;

template <>
struct less<MESSAGETASK> {
    auto operator()(const MESSAGETASK& lhs, const MESSAGETASK& rhs) const
        -> bool
    {
        /* TODO: use structured bindings */
        const auto& lID = std::get<0>(lhs);
        const auto& lMessage = std::get<1>(lhs);
        const auto& lFunction = std::get<2>(lhs);
        const auto& rID = std::get<0>(rhs);
        const auto& rMessage = std::get<1>(rhs);
        const auto& rFunction = std::get<2>(rhs);

        if (lID->str() < rID->str()) { return true; }

        if (rID->str() < lID->str()) { return false; }

        if (lMessage < rMessage) { return true; }

        if (rMessage < lMessage) { return false; }

        if (&lFunction < &rFunction) { return true; }

        return false;
    }
};

template <>
struct less<PAYMENTTASK> {
    auto operator()(const PAYMENTTASK& lhs, const PAYMENTTASK& rhs) const
        -> bool
    {
        /* TODO: use structured bindings */
        const auto& lID = std::get<0>(lhs);
        const auto& lPayment = std::get<1>(lhs);
        const auto& rID = std::get<0>(rhs);
        const auto& rPayment = std::get<1>(rhs);

        if (lID->str() < rID->str()) { return true; }

        if (rID->str() < lID->str()) { return false; }

        auto lPaymentID = opentxs::Identifier::Factory();
        auto rPaymentID = opentxs::Identifier::Factory();

        lPayment->GetIdentifier(lPaymentID);
        rPayment->GetIdentifier(rPaymentID);

        if (lPaymentID->str() < rPaymentID->str()) { return true; }

        return false;
    }
};

template <>
struct less<PEERREPLYTASK> {
    auto operator()(const PEERREPLYTASK& lhs, const PEERREPLYTASK& rhs) const
        -> bool
    {
        /* TODO: use structured bindings */
        const auto& lNym = std::get<0>(lhs);
        const auto& lReply = std::get<1>(lhs);
        const auto& lRequest = std::get<2>(lhs);
        const auto& rNym = std::get<0>(rhs);
        const auto& rReply = std::get<1>(rhs);
        const auto& rRequest = std::get<2>(rhs);

        if (lNym->str() < rNym->str()) { return true; }

        if (rNym->str() < lNym->str()) { return false; }

        if (lReply->ID()->str() < rReply->ID()->str()) { return true; }

        if (rReply->ID()->str() < lReply->ID()->str()) { return false; }

        if (lRequest->ID()->str() < rRequest->ID()->str()) { return true; }

        return false;
    }
};

template <>
struct less<PEERREQUESTTASK> {
    auto operator()(const PEERREQUESTTASK& lhs, const PEERREQUESTTASK& rhs)
        const -> bool
    {
        /* TODO: use structured bindings */
        const auto& lID = std::get<0>(lhs);
        const auto& lRequest = std::get<1>(lhs);
        const auto& rID = std::get<0>(rhs);
        const auto& rRequest = std::get<1>(rhs);

        if (lID->str() < rID->str()) { return true; }

        if (rID->str() < lID->str()) { return false; }

        if (lRequest->ID()->str() < rRequest->ID()->str()) { return true; }

        return false;
    }
};
}  // namespace std

namespace opentxs::otx::client::implementation
{
class StateMachine final : public opentxs::internal::StateMachine,
                           public otx::client::internal::StateMachine
{
public:
    using BackgroundTask = api::client::OTX::BackgroundTask;
    using ContextID = std::pair<OTNymID, OTServerID>;
    using Future = api::client::OTX::Future;
    using RefreshTask = std::pair<int, std::promise<void>>;
    using Result = api::client::OTX::Result;
    using TaskID = api::client::OTX::TaskID;
    using Thread = std::function<void()>;

    union Params {
        CheckNymTask check_nym_;
        DepositPaymentTask deposit_payment_;
        DownloadContractTask download_contract_;
#if OT_CASH
        DownloadMintTask download_mint_;
#endif
        DownloadNymboxTask download_nymbox_;
        DownloadUnitDefinitionTask download_unit_definition_;
        GetTransactionNumbersTask get_transaction_numbers_;
        IssueUnitDefinitionTask issue_unit_definition_;
        MessageTask send_message_;
#if OT_CASH
        PayCashTask send_cash_;
#endif
        PaymentTask send_payment_;
        PeerReplyTask peer_reply_;
        PeerRequestTask peer_request_;
        ProcessInboxTask process_inbox_;
        PublishServerContractTask publish_server_contract_;
        RegisterAccountTask register_account_;
        RegisterNymTask register_nym_;
        SendChequeTask send_cheque_;
        SendTransferTask send_transfer_;
#if OT_CASH
        WithdrawCashTask withdraw_cash_;
#endif

        Params() { memset(static_cast<void*>(this), 0, sizeof(Params)); }
        ~Params() {}
    };

    otx::client::implementation::PaymentTasks payment_tasks_;

    auto api() const -> const api::Core& override { return client_; }
    auto DepositPayment(const DepositPaymentTask& params) const
        -> BackgroundTask override
    {
        return StartTask(params);
    }
    auto DownloadUnitDefinition(const DownloadUnitDefinitionTask& params) const
        -> BackgroundTask override
    {
        return StartTask(params);
    }
    auto error_result() const -> Result override;
    auto RegisterAccount(const RegisterAccountTask& params) const
        -> BackgroundTask override
    {
        return StartTask(params);
    }
    template <typename T>
    auto StartTask(const T& params) const -> BackgroundTask;
    template <typename T>
    auto StartTask(const TaskID taskID, const T& params) const
        -> BackgroundTask;

    void Shutdown() { op_.Shutdown(); }

    StateMachine(
        const api::client::Manager& client,
        const api::client::internal::OTX& parent,
        const Flag& running,
        const api::client::Manager& api,
        const ContextID& id,
        std::atomic<TaskID>& nextTaskID,
        const UniqueQueue<CheckNymTask>& missingNyms,
        const UniqueQueue<CheckNymTask>& outdatedNyms,
        const UniqueQueue<OTServerID>& missingServers,
        const UniqueQueue<OTUnitID>& missingUnitDefinitions,
        const PasswordPrompt& reason);
    ~StateMachine() override = default;

private:
    enum class TaskDone : int { no, yes, retry };
    enum class State : int { needServerContract, needRegistration, ready };

    const api::client::Manager& client_;
    const api::client::internal::OTX& parent_;
    std::atomic<TaskID>& next_task_id_;
    const UniqueQueue<CheckNymTask>& missing_nyms_;
    const UniqueQueue<CheckNymTask>& outdated_nyms_;
    const UniqueQueue<OTServerID>& missing_servers_;
    const UniqueQueue<OTUnitID>& missing_unit_definitions_;
    const OTPasswordPrompt reason_;
    std::unique_ptr<otx::client::internal::Operation> pOp_;
    otx::client::internal::Operation& op_;
    UniqueQueue<CheckNymTask> check_nym_;
    UniqueQueue<DepositPaymentTask> deposit_payment_;
    UniqueQueue<DownloadContractTask> download_contract_;
#if OT_CASH
    UniqueQueue<DownloadMintTask> download_mint_;
#endif  // OT_CASH
    UniqueQueue<DownloadNymboxTask> download_nymbox_;
    UniqueQueue<DownloadUnitDefinitionTask> download_unit_definition_;
    UniqueQueue<GetTransactionNumbersTask> get_transaction_numbers_;
    UniqueQueue<IssueUnitDefinitionTask> issue_unit_definition_;
    UniqueQueue<MessageTask> send_message_;
#if OT_CASH
    UniqueQueue<PayCashTask> send_cash_;
#endif  // OT_CASH
    UniqueQueue<PaymentTask> send_payment_;
    UniqueQueue<PeerReplyTask> peer_reply_;
    UniqueQueue<PeerRequestTask> peer_request_;
    UniqueQueue<ProcessInboxTask> process_inbox_;
    UniqueQueue<PublishServerContractTask> publish_server_contract_;
    UniqueQueue<RegisterAccountTask> register_account_;
    UniqueQueue<RegisterNymTask> register_nym_;
    UniqueQueue<SendChequeTask> send_cheque_;
    UniqueQueue<SendTransferTask> send_transfer_;
#if OT_CASH
    UniqueQueue<WithdrawCashTask> withdraw_cash_;
#endif  // OT_CASH
    Params param_;
    TaskID task_id_{};
    std::atomic<int> counter_;
    mutable std::atomic<int> task_count_;
    mutable std::mutex lock_;
    std::vector<RefreshTask> tasks_;
    mutable State state_;
    mutable std::map<OTNymID, int> unknown_nyms_;
    mutable std::map<OTServerID, int> unknown_servers_;
    mutable std::map<OTUnitID, int> unknown_units_;

    static auto task_done(bool done) -> TaskDone
    {
        return done ? TaskDone::yes : TaskDone::no;
    }

    void associate_message_id(const Identifier& messageID, const TaskID taskID)
        const
    {
        return parent_.associate_message_id(messageID, taskID);
    }
    auto bump_task(const bool bump) const -> bool;
    auto check_admin(const otx::context::Server& context) const -> bool;
    template <typename T, typename C, typename M, typename U>
    auto check_missing_contract(M& missing, U& unknown, bool skip = true) const
        -> bool;
    void check_nym_revision(const otx::context::Server& context) const;
    auto check_registration(
        const identifier::Nym& nymID,
        const identifier::Server& serverID) const -> bool;
    auto check_server_contract(const identifier::Server& serverID) const
        -> bool;
    auto check_server_name(const otx::context::Server& context) const -> bool;
    void check_server_nym(const otx::context::Server& context) const;
    void check_transaction_numbers(const otx::context::Server& context) const;
    auto counter() const -> int { return counter_.load(); }
    auto deposit_cheque(const TaskID taskID, const DepositPaymentTask& task)
        const -> bool;
    auto deposit_cheque_wrapper(
        const TaskID taskID,
        const DepositPaymentTask& task,
        UniqueQueue<DepositPaymentTask>& retry) const -> bool;

#if OT_CASH
    auto download_mint(const TaskID taskID, const DownloadMintTask& task) const
        -> bool;
#endif
    auto download_nym(const TaskID taskID, const CheckNymTask& id) const
        -> bool;
    auto download_nymbox(const TaskID taskID) const -> bool;
    auto download_server(
        const TaskID taskID,
        const DownloadContractTask& contractID) const -> bool;
    auto download_unit_definition(
        const TaskID taskID,
        const DownloadUnitDefinitionTask& id) const -> bool;
    template <typename T, typename C, typename I, typename M, typename U>
    auto find_contract(
        const TaskID taskID,
        const I& targetID,
        M& missing,
        U& unknown,
        const bool skipExisting = true) const -> bool;
    auto finish_task(const TaskID taskID, const bool success, Result&& result)
        const -> bool override
    {
        return parent_.finish_task(taskID, success, std::move(result));
    }
    auto get_admin(const TaskID taskID, const Secret& password) const -> bool;
    auto get_nym_fetch(const identifier::Server& serverID) const
        -> UniqueQueue<OTNymID>&
    {
        return parent_.get_nym_fetch(serverID);
    }
    template <typename T>
    auto get_task() const -> const UniqueQueue<T>&;
    auto get_transaction_numbers(const TaskID taskID) const -> bool;
    auto initiate_peer_reply(const TaskID taskID, const PeerReplyTask& task)
        const -> bool;
    auto initiate_peer_request(const TaskID taskID, const PeerRequestTask& task)
        const -> bool;
    auto issue_unit_definition(
        const TaskID taskID,
        const IssueUnitDefinitionTask& task) const -> bool;
    auto issue_unit_definition_wrapper(
        const TaskID taskID,
        const IssueUnitDefinitionTask& task) const -> bool;
    template <typename T, typename I>
    auto load_contract(const I& id) const -> bool;
    auto message_nym(const TaskID taskID, const MessageTask& task) const
        -> bool;
    auto next_task_id() const -> TaskID override { return ++next_task_id_; }
    auto pay_nym(const TaskID taskID, const PaymentTask& task) const -> bool;
#if OT_CASH
    auto pay_nym_cash(const TaskID taskID, const PayCashTask& Task) const
        -> bool;
#endif  // OT_CASH
    auto process_inbox(const TaskID taskID, const ProcessInboxTask& accountID)
        const -> bool;
    auto publish_server_contract(
        const TaskID taskID,
        const PublishServerContractTask& serverID) const -> bool;
    auto register_account(const TaskID taskID, const RegisterAccountTask& task)
        const -> bool;
    auto register_account_wrapper(
        const TaskID taskID,
        const RegisterAccountTask& task) const -> bool;
    auto register_nym(const TaskID taskID, const RegisterNymTask& resync) const
        -> bool;
    auto register_nym_wrapper(
        const TaskID taskID,
        const RegisterNymTask& resync,
        UniqueQueue<RegisterNymTask>& retry) const -> bool;
    template <typename M, typename I>
    void resolve_unknown(const I& id, const bool found, M& map) const;
    template <typename T, typename M>
    void scan_unknown(const M& map, int& next) const;
    auto send_transfer(const TaskID taskID, const SendTransferTask& task) const
        -> bool;
    auto start_task(const TaskID taskID, bool success) const
        -> BackgroundTask override
    {
        return parent_.start_task(taskID, success);
    }
#if OT_CASH
    auto withdraw_cash(const TaskID taskID, const WithdrawCashTask& task) const
        -> bool;
#endif  // OT_CASH
    auto write_and_send_cheque(const TaskID taskID, const SendChequeTask& task)
        const -> TaskDone;
    auto write_and_send_cheque_wrapper(
        const TaskID taskID,
        const SendChequeTask& task,
        UniqueQueue<SendChequeTask>& retry) const -> bool;

    template <typename T>
    auto get_param() -> T&;
    void increment_counter(const bool run);
    auto main_loop() noexcept -> bool;
    auto queue_contracts(const otx::context::Server& context, int& next)
        -> bool;
    auto queue_nyms() -> bool;
    template <typename T>
    auto run_task(bool (StateMachine::*func)(const TaskID) const) -> bool;
    template <typename T>
    auto run_task(bool (StateMachine::*func)(const TaskID, const T&) const)
        -> bool;
    template <typename T, typename R>
    auto run_task(
        bool (StateMachine::*func)(const TaskID, const T&, R&) const,
        R& retry) -> bool;
    template <typename T>
    auto run_task(std::function<bool(const TaskID, const T&)> func) -> bool;
    auto state_machine() noexcept -> bool;

    StateMachine() = delete;
};
}  // namespace opentxs::otx::client::implementation
