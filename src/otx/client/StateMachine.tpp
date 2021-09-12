// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "StateMachine.hpp"  // IWYU pragma: associated

#include "opentxs/api/Wallet.hpp"

namespace opentxs::otx::client::implementation
{
template <>
auto StateMachine::get_param() -> CheckNymTask&
{
    return param_.check_nym_;
}
template <>
auto StateMachine::get_param() -> DepositPaymentTask&
{
    return param_.deposit_payment_;
}
template <>
auto StateMachine::get_param() -> DownloadContractTask&
{
    return param_.download_contract_;
}
#if OT_CASH
template <>
auto StateMachine::get_param() -> DownloadMintTask&
{
    return param_.download_mint_;
}
#endif
template <>
auto StateMachine::get_param() -> DownloadNymboxTask&
{
    return param_.download_nymbox_;
}
template <>
auto StateMachine::get_param() -> DownloadUnitDefinitionTask&
{
    return param_.download_unit_definition_;
}
template <>
auto StateMachine::get_param() -> GetTransactionNumbersTask&
{
    return param_.get_transaction_numbers_;
}
template <>
auto StateMachine::get_param() -> IssueUnitDefinitionTask&
{
    return param_.issue_unit_definition_;
}
template <>
auto StateMachine::get_param() -> MessageTask&
{
    return param_.send_message_;
}
#if OT_CASH
template <>
auto StateMachine::get_param() -> PayCashTask&
{
    return param_.send_cash_;
}
#endif
template <>
auto StateMachine::get_param() -> PaymentTask&
{
    return param_.send_payment_;
}
template <>
auto StateMachine::get_param() -> PeerReplyTask&
{
    return param_.peer_reply_;
}
template <>
auto StateMachine::get_param() -> PeerRequestTask&
{
    return param_.peer_request_;
}
template <>
auto StateMachine::get_param() -> ProcessInboxTask&
{
    return param_.process_inbox_;
}
template <>
auto StateMachine::get_param() -> PublishServerContractTask&
{
    return param_.publish_server_contract_;
}
template <>
auto StateMachine::get_param() -> RegisterAccountTask&
{
    return param_.register_account_;
}
template <>
auto StateMachine::get_param() -> RegisterNymTask&
{
    return param_.register_nym_;
}
template <>
auto StateMachine::get_param() -> SendChequeTask&
{
    return param_.send_cheque_;
}
template <>
auto StateMachine::get_param() -> SendTransferTask&
{
    return param_.send_transfer_;
}
#if OT_CASH
template <>
auto StateMachine::get_param() -> WithdrawCashTask&
{
    return param_.withdraw_cash_;
}
#endif

template <>
auto StateMachine::get_task() const -> const UniqueQueue<CheckNymTask>&
{
    return check_nym_;
}
template <>
auto StateMachine::get_task() const -> const UniqueQueue<DepositPaymentTask>&
{
    return deposit_payment_;
}
template <>
auto StateMachine::get_task() const -> const UniqueQueue<DownloadContractTask>&
{
    return download_contract_;
}
#if OT_CASH
template <>
auto StateMachine::get_task() const -> const UniqueQueue<DownloadMintTask>&
{
    return download_mint_;
}
#endif
template <>
auto StateMachine::get_task() const -> const UniqueQueue<DownloadNymboxTask>&
{
    return download_nymbox_;
}
template <>
auto StateMachine::get_task() const
    -> const UniqueQueue<DownloadUnitDefinitionTask>&
{
    return download_unit_definition_;
}
template <>
auto StateMachine::get_task() const
    -> const UniqueQueue<GetTransactionNumbersTask>&
{
    return get_transaction_numbers_;
}
template <>
auto StateMachine::get_task() const
    -> const UniqueQueue<IssueUnitDefinitionTask>&
{
    return issue_unit_definition_;
}
template <>
auto StateMachine::get_task() const -> const UniqueQueue<MessageTask>&
{
    return send_message_;
}
#if OT_CASH
template <>
auto StateMachine::get_task() const -> const UniqueQueue<PayCashTask>&
{
    return send_cash_;
}
#endif
template <>
auto StateMachine::get_task() const -> const UniqueQueue<PaymentTask>&
{
    return send_payment_;
}
template <>
auto StateMachine::get_task() const -> const UniqueQueue<PeerReplyTask>&
{
    return peer_reply_;
}
template <>
auto StateMachine::get_task() const -> const UniqueQueue<PeerRequestTask>&
{
    return peer_request_;
}
template <>
auto StateMachine::get_task() const -> const UniqueQueue<ProcessInboxTask>&
{
    return process_inbox_;
}
template <>
auto StateMachine::get_task() const
    -> const UniqueQueue<PublishServerContractTask>&
{
    return publish_server_contract_;
}
template <>
auto StateMachine::get_task() const -> const UniqueQueue<RegisterAccountTask>&
{
    return register_account_;
}
template <>
auto StateMachine::get_task() const -> const UniqueQueue<RegisterNymTask>&
{
    return register_nym_;
}
template <>
auto StateMachine::get_task() const -> const UniqueQueue<SendChequeTask>&
{
    return send_cheque_;
}
template <>
auto StateMachine::get_task() const -> const UniqueQueue<SendTransferTask>&
{
    return send_transfer_;
}
#if OT_CASH
template <>
auto StateMachine::get_task() const -> const UniqueQueue<WithdrawCashTask>&
{
    return withdraw_cash_;
}
#endif

template <>
auto StateMachine::load_contract<CheckNymTask>(const identifier::Nym& id) const
    -> bool
{
    return bool(client_.Wallet().Nym(id));
}
template <>
auto StateMachine::load_contract<DownloadContractTask>(
    const identifier::Server& id) const -> bool
{
    try {
        client_.Wallet().Server(id);

        return true;
    } catch (...) {

        return false;
    }
}

template <>
auto StateMachine::load_contract<DownloadUnitDefinitionTask>(
    const identifier::UnitDefinition& id) const -> bool
{
    try {
        client_.Wallet().UnitDefinition(id);

        return true;
    } catch (...) {

        return false;
    }
}

template StateMachine::BackgroundTask StateMachine::StartTask(
    const CheckNymTask& params) const;
template StateMachine::BackgroundTask StateMachine::StartTask(
    const DepositPaymentTask& params) const;
template StateMachine::BackgroundTask StateMachine::StartTask(
    const DownloadContractTask& params) const;
#if OT_CASH
template StateMachine::BackgroundTask StateMachine::StartTask(
    const DownloadMintTask& params) const;
#endif  // OT_CASH
template StateMachine::BackgroundTask StateMachine::StartTask(
    const DownloadNymboxTask& params) const;
template StateMachine::BackgroundTask StateMachine::StartTask(
    const DownloadUnitDefinitionTask& params) const;
template StateMachine::BackgroundTask StateMachine::StartTask(
    const GetTransactionNumbersTask& params) const;
template StateMachine::BackgroundTask StateMachine::StartTask(
    const IssueUnitDefinitionTask& params) const;
template StateMachine::BackgroundTask StateMachine::StartTask(
    const MessageTask& params) const;
#if OT_CASH
template StateMachine::BackgroundTask StateMachine::StartTask(
    const PayCashTask& params) const;
#endif  // OT_CASH
template StateMachine::BackgroundTask StateMachine::StartTask(
    const PaymentTask& params) const;
template StateMachine::BackgroundTask StateMachine::StartTask(
    const PeerReplyTask& params) const;
template StateMachine::BackgroundTask StateMachine::StartTask(
    const PeerRequestTask& params) const;
template StateMachine::BackgroundTask StateMachine::StartTask(
    const ProcessInboxTask& params) const;
template StateMachine::BackgroundTask StateMachine::StartTask(
    const PublishServerContractTask& params) const;
template StateMachine::BackgroundTask StateMachine::StartTask(
    const RegisterAccountTask& params) const;
template StateMachine::BackgroundTask StateMachine::StartTask(
    const RegisterNymTask& params) const;
template StateMachine::BackgroundTask StateMachine::StartTask(
    const SendChequeTask& params) const;
template StateMachine::BackgroundTask StateMachine::StartTask(
    const SendTransferTask& params) const;
#if OT_CASH
template StateMachine::BackgroundTask StateMachine::StartTask(
    const WithdrawCashTask& params) const;
#endif  // OT_CASH
}  // namespace opentxs::otx::client::implementation
