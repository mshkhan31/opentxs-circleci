// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"              // IWYU pragma: associated
#include "1_Internal.hpp"            // IWYU pragma: associated
#include "otx/client/Operation.hpp"  // IWYU pragma: associated

#include <atomic>
#include <chrono>
#include <cstdint>
#include <functional>
#include <future>
#include <iterator>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>

#include "2_Factory.hpp"
#include "Proto.tpp"
#include "core/OTStorage.hpp"
#include "core/StateMachine.hpp"
#include "internal/api/Api.hpp"
#include "internal/otx/client/Client.hpp"
#include "opentxs/Bytes.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/Shared.hpp"
#include "opentxs/SharedPimpl.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/api/Context.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/api/Legacy.hpp"
#include "opentxs/api/Wallet.hpp"
#include "opentxs/api/client/Activity.hpp"
#include "opentxs/api/client/Manager.hpp"
#include "opentxs/api/client/PaymentWorkflowState.hpp"
#include "opentxs/api/client/Workflow.hpp"
#if OT_CASH
#include "opentxs/blind/Mint.hpp"
#include "opentxs/blind/Purse.hpp"
#endif  // OT_CASH
#include "opentxs/client/OT_API.hpp"
#include "opentxs/contact/ContactSectionName.hpp"
#include "opentxs/core/Armored.hpp"
#include "opentxs/core/Cheque.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Item.hpp"
#include "opentxs/core/Ledger.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"
#include "opentxs/core/Message.hpp"
#include "opentxs/core/NymFile.hpp"
#include "opentxs/core/OTTransaction.hpp"
#include "opentxs/core/PasswordPrompt.hpp"
#include "opentxs/core/contract/ServerContract.hpp"
#include "opentxs/core/contract/UnitDefinition.hpp"
#include "opentxs/core/contract/peer/PeerObject.hpp"
#include "opentxs/core/contract/peer/PeerObjectType.hpp"
#include "opentxs/core/contract/peer/PeerReply.hpp"
#include "opentxs/core/contract/peer/PeerRequest.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/core/identifier/Server.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"
#include "opentxs/core/transaction/Helpers.hpp"
#include "opentxs/crypto/Envelope.hpp"
#include "opentxs/ext/OTPayment.hpp"
#include "opentxs/identity/Nym.hpp"
#include "opentxs/otx/LastReplyStatus.hpp"
#include "opentxs/otx/OperationType.hpp"
#include "opentxs/otx/Types.hpp"
#include "opentxs/otx/consensus/Base.hpp"
#include "opentxs/otx/consensus/ManagedNumber.hpp"
#include "opentxs/otx/consensus/Server.hpp"
#include "opentxs/protobuf/Check.hpp"
#include "opentxs/protobuf/Nym.pb.h"
#include "opentxs/protobuf/PaymentWorkflow.pb.h"
#include "opentxs/protobuf/PeerObject.pb.h"
#include "opentxs/protobuf/PeerReply.pb.h"
#include "opentxs/protobuf/PeerRequest.pb.h"
#include "opentxs/protobuf/Purse.pb.h"
#include "opentxs/protobuf/ServerContract.pb.h"
#include "opentxs/protobuf/UnitDefinition.pb.h"  // IWYU pragma: keep
#include "opentxs/protobuf/verify/UnitDefinition.hpp"

#define START()                                                                \
    Lock lock(decision_lock_);                                                 \
                                                                               \
    if (running().load()) {                                                    \
        LogDebug(OT_METHOD)(__func__)(": State machine is already running.")   \
            .Flush();                                                          \
                                                                               \
        return {};                                                             \
    }                                                                          \
                                                                               \
    reset();

#define OPERATION_POLL_MILLISECONDS 100
#define OPERATION_JOIN_MILLISECONDS OPERATION_POLL_MILLISECONDS
#define MAX_ERROR_COUNT 3

#define OT_METHOD "opentxs::otx::client::implementation::Operation::"

#define PREPARE_CONTEXT()                                                      \
    auto contextEditor = context();                                            \
    auto& context = contextEditor.get();                                       \
    [[maybe_unused]] auto& nym = *context.Nym();                               \
    [[maybe_unused]] auto& nymID = nym.ID();                                   \
    [[maybe_unused]] auto& serverID = context.Notary();                        \
    [[maybe_unused]] auto& serverNym = context.RemoteNym();                    \
    context.SetPush(enable_otx_push_.load());

#define CREATE_MESSAGE(a, ...)                                                 \
    [[maybe_unused]] auto [nextNumber, pMessage] =                             \
        context.InitializeServerCommand(MessageType::a, __VA_ARGS__);          \
                                                                               \
    if (false == bool(pMessage)) {                                             \
        LogOutput(OT_METHOD)(__func__)(": Failed to construct ")(#a).Flush();  \
                                                                               \
        return {};                                                             \
    }                                                                          \
                                                                               \
    auto& message = *pMessage;

#define FINISH_MESSAGE(a, b)                                                   \
    const auto finalized = context.FinalizeServerCommand(message, reason_);    \
                                                                               \
    if (false == bool(finalized)) {                                            \
        LogOutput(OT_METHOD)(#a)(": Failed to sign ")(#b);                     \
                                                                               \
        return {};                                                             \
    }                                                                          \
                                                                               \
    return std::move(pMessage);

#define PREPARE_TRANSACTION_WITHOUT_BALANCE_ITEM(                              \
    TRANSACTION_TYPE, ORIGIN_TYPE, ITEM_TYPE, DESTINATION_ACCOUNT)             \
                                                                               \
    PREPARE_CONTEXT();                                                         \
                                                                               \
    auto account = api_.Wallet().Account(account_id_);                         \
                                                                               \
    if (false == bool(account)) {                                              \
        LogOutput(OT_METHOD)(__func__)(": Failed to load account.").Flush();   \
                                                                               \
        return {};                                                             \
    }                                                                          \
                                                                               \
    numbers_.insert(                                                           \
        context.NextTransactionNumber(MessageType::notarizeTransaction));      \
    auto& managedNumber = *numbers_.rbegin();                                  \
    LogVerbose(OT_METHOD)(__func__)(": Allocating transaction number ")(       \
        managedNumber->Value())                                                \
        .Flush();                                                              \
                                                                               \
    if (false == managedNumber->Valid()) {                                     \
        LogOutput(OT_METHOD)(__func__)(                                        \
            ": No transaction numbers were available. Suggest requesting the " \
            "server for one.")                                                 \
            .Flush();                                                          \
                                                                               \
        return {};                                                             \
    }                                                                          \
                                                                               \
    LogVerbose(OT_METHOD)(__func__)(": Allocated transaction number ")(        \
        managedNumber->Value())                                                \
        .Flush();                                                              \
    const auto transactionNum = managedNumber->Value();                        \
    auto pLedger{api_.Factory().Ledger(nymID, account_id_, serverID)};         \
                                                                               \
    if (false == bool(pLedger)) {                                              \
        LogOutput(OT_METHOD)(__func__)(                                        \
            ": Failed to construct transaction ledger.")                       \
            .Flush();                                                          \
                                                                               \
        return {};                                                             \
    }                                                                          \
                                                                               \
    auto& ledger = *pLedger;                                                   \
    const bool generated =                                                     \
        ledger.GenerateLedger(account_id_, serverID, ledgerType::message);     \
                                                                               \
    if (false == generated) {                                                  \
        LogOutput(OT_METHOD)(__func__)(                                        \
            ": Failed to generate transaction ledger")                         \
            .Flush();                                                          \
                                                                               \
        return {};                                                             \
    }                                                                          \
                                                                               \
    std::shared_ptr<OTTransaction> pTransaction{api_.Factory()                 \
                                                    .Transaction(              \
                                                        nymID,                 \
                                                        account_id_,           \
                                                        serverID,              \
                                                        TRANSACTION_TYPE,      \
                                                        ORIGIN_TYPE,           \
                                                        transactionNum)        \
                                                    .release()};               \
                                                                               \
    if (false == bool(pTransaction)) {                                         \
        LogOutput(OT_METHOD)(__func__)(": Failed to construct transaction.")   \
            .Flush();                                                          \
                                                                               \
        return {};                                                             \
    }                                                                          \
                                                                               \
    ledger.AddTransaction(pTransaction);                                       \
    auto& transaction = *pTransaction;                                         \
    std::shared_ptr<Item> pItem{                                               \
        api_.Factory().Item(transaction, ITEM_TYPE, DESTINATION_ACCOUNT)};     \
                                                                               \
    if (false == bool(pItem)) {                                                \
        LogOutput(OT_METHOD)(__func__)(": Failed to construct item.").Flush(); \
                                                                               \
        return {};                                                             \
    }                                                                          \
                                                                               \
    transaction.AddItem(pItem);                                                \
    auto& item = *pItem;                                                       \
    inbox_.reset(account.get().LoadInbox(nym).release());                      \
    outbox_.reset(account.get().LoadOutbox(nym).release());                    \
                                                                               \
    if (false == bool(inbox_)) {                                               \
        LogOutput(OT_METHOD)(__func__)(": Failed loading inbox for "           \
                                       "account: ")(account_id_)               \
            .Flush();                                                          \
                                                                               \
        return {};                                                             \
    }                                                                          \
                                                                               \
    if (false == bool(outbox_)) {                                              \
        LogOutput(OT_METHOD)(__func__)(": Failed loading outbox for "          \
                                       "account: ")(account_id_)               \
            .Flush();                                                          \
                                                                               \
        return {};                                                             \
    }                                                                          \
                                                                               \
    auto& inbox = *inbox_;                                                     \
    auto& outbox = *outbox_;                                                   \
    set_consensus_hash(transaction, context, account.get(), reason_);

#define ADD_BALANCE_ITEM(AMOUNT)                                               \
    std::shared_ptr<Item> pBalanceItem{inbox.GenerateBalanceStatement(         \
        AMOUNT, transaction, context, account.get(), outbox, reason_)};        \
                                                                               \
    if (false == bool(pBalanceItem)) {                                         \
        LogOutput(OT_METHOD)(__func__)(": Failed to construct balance item.")  \
            .Flush();                                                          \
                                                                               \
        return {};                                                             \
    }                                                                          \
                                                                               \
    transaction.AddItem(pBalanceItem);

#define PREPARE_TRANSACTION(                                                   \
    TRANSACTION_TYPE, ORIGIN_TYPE, ITEM_TYPE, DESTINATION_ACCOUNT, AMOUNT)     \
    PREPARE_TRANSACTION_WITHOUT_BALANCE_ITEM(                                  \
        TRANSACTION_TYPE, ORIGIN_TYPE, ITEM_TYPE, DESTINATION_ACCOUNT);        \
    ADD_BALANCE_ITEM(AMOUNT);

#define SIGN_ITEM()                                                            \
    item.SignContract(nym, reason_);                                           \
    item.SaveContract();

#define SIGN_TRANSACTION_AND_LEDGER()                                          \
    transaction.SignContract(nym, reason_);                                    \
    transaction.SaveContract();                                                \
    ledger.SignContract(nym, reason_);                                         \
    ledger.SaveContract();

#define FINISH_TRANSACTION()                                                   \
    SIGN_ITEM();                                                               \
    SIGN_TRANSACTION_AND_LEDGER();                                             \
    CREATE_MESSAGE(                                                            \
        notarizeTransaction,                                                   \
        Armored::Factory(String::Factory(ledger)),                             \
        account_id_,                                                           \
        -1);                                                                   \
    FINISH_MESSAGE(__func__, notarizeTransaction);

namespace zmq = opentxs::network::zeromq;

namespace opentxs
{
auto Factory::Operation(
    const api::client::Manager& api,
    const identifier::Nym& nym,
    const identifier::Server& server,
    const opentxs::PasswordPrompt& reason) -> otx::client::internal::Operation*
{
    return new otx::client::implementation::Operation(api, nym, server, reason);
}
}  // namespace opentxs

namespace opentxs::otx::client::implementation
{
const std::map<otx::OperationType, Operation::Category> Operation::category_{
    {otx::OperationType::AddClaim, Category::Basic},
    {otx::OperationType::CheckNym, Category::Basic},
    {otx::OperationType::ConveyPayment, Category::Basic},
    {otx::OperationType::DepositCash, Category::Transaction},
    {otx::OperationType::DepositCheque, Category::Transaction},
    {otx::OperationType::DownloadContract, Category::Basic},
    {otx::OperationType::DownloadMint, Category::Basic},
    {otx::OperationType::GetTransactionNumbers, Category::NymboxPre},
    {otx::OperationType::IssueUnitDefinition, Category::CreateAccount},
    {otx::OperationType::PublishNym, Category::Basic},
    {otx::OperationType::PublishServer, Category::Basic},
    {otx::OperationType::PublishUnit, Category::Basic},
    {otx::OperationType::RefreshAccount, Category::UpdateAccount},
    {otx::OperationType::RegisterAccount, Category::CreateAccount},
    {otx::OperationType::RegisterNym, Category::NymboxPost},
    {otx::OperationType::RequestAdmin, Category::Basic},
    {otx::OperationType::SendCash, Category::Basic},
    {otx::OperationType::SendMessage, Category::Basic},
    {otx::OperationType::SendPeerReply, Category::Basic},
    {otx::OperationType::SendPeerRequest, Category::Basic},
    {otx::OperationType::SendTransfer, Category::Transaction},
    {otx::OperationType::WithdrawCash, Category::Transaction},
};

const std::map<otx::OperationType, std::size_t> Operation::transaction_numbers_{
    {otx::OperationType::AddClaim, 0},
    {otx::OperationType::CheckNym, 0},
    {otx::OperationType::ConveyPayment, 0},
    {otx::OperationType::DepositCash, 2},
    {otx::OperationType::DepositCheque, 2},
    {otx::OperationType::DownloadContract, 0},
    {otx::OperationType::DownloadMint, 0},
    {otx::OperationType::GetTransactionNumbers, 0},
    {otx::OperationType::IssueUnitDefinition, 0},
    {otx::OperationType::PublishNym, 0},
    {otx::OperationType::PublishServer, 0},
    {otx::OperationType::PublishUnit, 0},
    {otx::OperationType::RefreshAccount, 1},
    {otx::OperationType::RegisterAccount, 0},
    {otx::OperationType::RegisterNym, 0},
    {otx::OperationType::RequestAdmin, 0},
    {otx::OperationType::SendCash, 0},
    {otx::OperationType::SendMessage, 0},
    {otx::OperationType::SendPeerReply, 0},
    {otx::OperationType::SendPeerRequest, 0},
    {otx::OperationType::SendTransfer, 2},
    {otx::OperationType::WithdrawCash, 2},
};

Operation::Operation(
    const api::client::Manager& api,
    const identifier::Nym& nym,
    const identifier::Server& server,
    const opentxs::PasswordPrompt& reason)
    : StateMachine(std::bind(&Operation::state_machine, this))
    , api_(api)
    , reason_(reason)
    , nym_id_(nym)
    , server_id_(server)
    , type_(otx::OperationType::Invalid)
    , state_(State::Idle)
    , refresh_account_(false)
    , args_()
    , message_()
    , outmail_message_()
    , result_set_(false)
    , enable_otx_push_(true)
    , result_()
    , target_nym_id_(identifier::Nym::Factory())
    , target_server_id_(identifier::Server::Factory())
    , target_unit_id_(identifier::UnitDefinition::Factory())
    , contract_type_(ContractType::invalid)
    , unit_definition_()
    , account_id_(Identifier::Factory())
    , generic_id_(Identifier::Factory())
    , amount_(0)
    , memo_(String::Factory())
    , bool_(false)
    , claim_section_(contact::ContactSectionName::Error)
    , claim_type_(contact::ContactItemType::Error)
    , cheque_()
    , payment_()
    , inbox_()
    , outbox_()
#if OT_CASH
    , purse_()
#endif
    , affected_accounts_()
    , redownload_accounts_()
    , numbers_()
    , error_count_(0)
    , peer_reply_(api_.Factory().PeerReply())
    , peer_request_(api_.Factory().PeerRequest())
    , set_id_()
{
}

void Operation::account_pre()
{
    otx::context::Server::DeliveryResult lastResult{};

    switch (category_.at(type_)) {
        case Category::Transaction: {
            download_accounts(State::Execute, State::NymboxPre, lastResult);
        } break;
        default: {
            state_.store(State::Execute);
        }
    }
}

void Operation::account_post()
{
    otx::context::Server::DeliveryResult lastResult{};

    if (download_accounts(State::NymboxPost, State::NymboxPre, lastResult)) {
        if (false == result_set_.load()) { set_result(std::move(lastResult)); }

        affected_accounts_ = redownload_accounts_;
        redownload_accounts_.clear();

        if (0 < affected_accounts_.size()) { state_.store(State::AccountPost); }
    }
}

auto Operation::AddClaim(
    const contact::ContactSectionName section,
    const contact::ContactItemType type,
    const String& value,
    const bool primary) -> bool
{
    START()

    memo_ = value;
    bool_ = primary;
    claim_section_ = section;
    claim_type_ = type;

    return start(lock, otx::OperationType::AddClaim, {});
}

auto Operation::check_future(otx::context::Server::SendFuture& future) -> bool
{
    return std::future_status::ready !=
           future.wait_for(
               std::chrono::milliseconds(OPERATION_POLL_MILLISECONDS));
}

auto Operation::construct() -> std::shared_ptr<Message>
{
    switch (type_.load()) {
        case otx::OperationType::AddClaim: {

            return construct_add_claim();
        }
        case otx::OperationType::CheckNym: {

            return construct_check_nym();
        }
        case otx::OperationType::ConveyPayment: {

            return construct_convey_payment();
        }
#if OT_CASH
        case otx::OperationType::DepositCash: {

            return construct_deposit_cash();
        }
#endif
        case otx::OperationType::DepositCheque: {

            return construct_deposit_cheque();
        }
        case otx::OperationType::DownloadContract: {

            return construct_download_contract();
        }
#if OT_CASH
        case otx::OperationType::DownloadMint: {

            return construct_download_mint();
        }
#endif
        case otx::OperationType::GetTransactionNumbers: {

            return construct_get_transaction_numbers();
        }
        case otx::OperationType::IssueUnitDefinition: {

            return construct_issue_unit_definition();
        }
        case otx::OperationType::PublishNym: {

            return construct_publish_nym();
        }
        case otx::OperationType::PublishServer: {

            return construct_publish_server();
        }
        case otx::OperationType::PublishUnit: {

            return construct_publish_unit();
        }
        case otx::OperationType::RegisterAccount: {

            return construct_register_account();
        }
        case otx::OperationType::RegisterNym: {

            return construct_register_nym();
        }
        case otx::OperationType::RequestAdmin: {

            return construct_request_admin();
        }
#if OT_CASH
        case otx::OperationType::SendCash: {

            return construct_send_cash();
        }
#endif
        case otx::OperationType::SendMessage: {

            return construct_send_message();
        }
        case otx::OperationType::SendPeerReply: {

            return construct_send_peer_reply();
        }
        case otx::OperationType::SendPeerRequest: {

            return construct_send_peer_request();
        }
        case otx::OperationType::SendTransfer: {

            return construct_send_transfer();
        }
#if OT_CASH
        case otx::OperationType::WithdrawCash: {

            return construct_withdraw_cash();
        }
#endif
        default: {
            LogOutput(OT_METHOD)(__func__)(": Unknown message type").Flush();
        }
    }

    return {};
}

auto Operation::construct_add_claim() -> std::shared_ptr<Message>
{
    PREPARE_CONTEXT();
    CREATE_MESSAGE(addClaim, -1, true, true);

    message.m_strNymID2 = String::Factory(
        std::to_string(static_cast<std::uint32_t>(claim_section_)));
    message.m_strInstrumentDefinitionID = String::Factory(
        std::to_string(static_cast<std::uint32_t>(claim_type_)));
    message.m_strAcctID = memo_;
    message.m_bBool = bool_;

    FINISH_MESSAGE(__func__, addClaim);
}

auto Operation::construct_check_nym() -> std::shared_ptr<Message>
{
    PREPARE_CONTEXT();
    CREATE_MESSAGE(checkNym, target_nym_id_, -1, true, true);
    FINISH_MESSAGE(__func__, checkNym);
}

auto Operation::construct_convey_payment() -> std::shared_ptr<Message>
{
    if (false == bool(payment_)) {
        LogOutput(OT_METHOD)(__func__)(": No payment to convey").Flush();

        return {};
    }

    auto& payment = *payment_;
    const auto recipientNym = api_.Wallet().Nym(target_nym_id_);

    if (false == bool(recipientNym)) {
        LogOutput(OT_METHOD)(__func__)(": Recipient nym credentials not found")
            .Flush();

        return {};
    }

    PREPARE_CONTEXT();
    CREATE_MESSAGE(sendNymInstrument, target_nym_id_, -1, true, true);

    const RequestNumber requestNumber{message.m_strRequestNum->ToLong()};
    auto serialized = String::Factory();
    const bool havePayment = payment.GetPaymentContents(serialized);

    if (false == havePayment) {
        LogOutput(OT_METHOD)(__func__)(
            ": Failed attempt to send a blank payment.")
            .Flush();

        return {};
    }

    auto envelope = api_.Factory().Envelope();
    auto sealed = envelope->Seal(
        {recipientNym, context.Nym()}, serialized->Bytes(), reason_);

    if (sealed) { sealed &= envelope->Armored(message.m_ascPayload); }

    if (false == sealed) {
        LogOutput(OT_METHOD)(__func__)(": Failed encrypt payment.").Flush();

        return {};
    }

    const auto finalized = context.FinalizeServerCommand(message, reason_);

    if (false == bool(finalized)) {
        LogOutput(OT_METHOD)(__func__)(": Failed to sign message");

        return {};
    }

    const auto pObject = api_.Factory().PeerObject(
        context.Nym(), String::Factory(message)->Get(), true);

    if (false == bool(pObject)) {
        LogOutput(OT_METHOD)(__func__)(": Failed to create peer object");

        return {};
    }

    auto& object = *pObject;

    return construct_send_nym_object(
        object, recipientNym, context, requestNumber);
}

#if OT_CASH
auto Operation::construct_deposit_cash() -> std::shared_ptr<Message>
{
    if (false == bool(purse_)) {
        LogOutput(OT_METHOD)(__func__)(": Missing purse.").Flush();

        return {};
    }

    auto& purse = *purse_;
    const Amount amount{purse.Value()};

    PREPARE_TRANSACTION(
        transactionType::deposit,
        originType::not_applicable,
        itemType::deposit,
        Identifier::Factory(),
        amount);

    const auto& unitID = account.get().GetInstrumentDefinitionID();

    if (unitID != purse.Unit()) {
        LogOutput(OT_METHOD)(__func__)(": Incorrect account type").Flush();

        return {};
    }

    item.SetAttachment([&] {
        auto proto = proto::Purse{};
        purse.Serialize(proto);

        return api_.Factory().Data(proto);
    }());

    FINISH_TRANSACTION();
}
#endif

auto Operation::construct_deposit_cheque() -> std::shared_ptr<Message>
{
    if (false == bool(cheque_)) {
        LogOutput(OT_METHOD)(__func__)(": No cheque to deposit").Flush();

        return {};
    }

    auto& cheque = *cheque_;
    const Amount amount{cheque.GetAmount()};

    PREPARE_TRANSACTION(
        transactionType::deposit,
        originType::not_applicable,
        itemType::depositCheque,
        Identifier::Factory(),
        amount);

    if (cheque.GetNotaryID() != serverID) {
        LogOutput(OT_METHOD)(__func__)(": NotaryID on cheque (")(
            cheque.GetNotaryID())(") doesn't match "
                                  "notaryID where it's "
                                  "being deposited to "
                                  "(")(serverID)(").")
            .Flush();

        return {};
    }

    // If cancellingCheque==true, we're actually cancelling the cheque by
    // "depositing" it back into the same account it's drawn on.
    bool cancellingCheque{false};
    auto copy{api_.Factory().Cheque(
        serverID, account.get().GetInstrumentDefinitionID())};

    if (cheque.HasRemitter()) {
        cancellingCheque =
            ((cheque.GetRemitterAcctID() == account_id_) &&
             (cheque.GetRemitterNymID() == nymID));
    } else {
        cancellingCheque =
            ((cheque.GetSenderAcctID() == account_id_) &&
             (cheque.GetSenderNymID() == nymID));
        if (cancellingCheque) cancellingCheque = cheque.VerifySignature(nym);
    }

    if (cancellingCheque) {
        cancellingCheque =
            context.VerifyIssuedNumber(cheque.GetTransactionNum());

        // If we TRIED to cancel the cheque (being in this block...) yet the
        // signature fails to verify, or the transaction number isn't even
        // issued, then our attempt to cancel the cheque is going to fail.
        if (false == cancellingCheque) {
            // This is the "tried and failed" block.
            LogOutput(OT_METHOD)(__func__)(
                ": Cannot cancel this cheque. Either the signature fails to "
                "verify, or the transaction number is already closed out.")
                .Flush();

            return {};
        }

        // Else we succeeded in verifying signature and issued num. Let's just
        // make sure there isn't a chequeReceipt or voucherReceipt already
        // sitting in the inbox, for this same cheque.
        auto pChequeReceipt =
            inbox_->GetChequeReceipt(cheque.GetTransactionNum());

        if (pChequeReceipt) {
            LogOutput(OT_METHOD)(__func__)(
                ": Cannot cancel this cheque. There is already "
                "a ")(
                cheque.HasRemitter() ? "voucherReceipt"
                                     : "chequeReceipt")(" for it in the inbox.")
                .Flush();

            return {};
        }

        if (false == copy->LoadContractFromString(String::Factory(cheque))) {
            LogOutput(OT_METHOD)(__func__)(
                ": Unable to load cheque from string.")
                .Flush();

            return {};
        }
    }

    // By this point, we're either NOT cancelling the cheque, or if we are,
    // we've already verified the signature and transaction number on the
    // cheque. (AND we've already verified that there aren't any
    // chequeReceipts for this cheque, in the inbox.)
    const bool cancel = (cancellingCheque && !cheque.HasRemitter());

    if (cancel) {
        copy->CancelCheque();
        copy->ReleaseSignatures();
        copy->SignContract(nym, reason_);
        copy->SaveContract();
        cheque_.reset(copy.release());

        OT_ASSERT(cheque_);
    }

    const auto strNote = String::Factory(
        cancellingCheque ? "Cancel this cheque, please!"
                         : "Deposit this cheque, please!");
    item.SetNote(strNote);
    item.SetAttachment(String::Factory(cheque));

    FINISH_TRANSACTION();
}

auto Operation::construct_download_contract() -> std::shared_ptr<Message>
{
    PREPARE_CONTEXT();
    CREATE_MESSAGE(getInstrumentDefinition, -1, true, true);

    message.m_strInstrumentDefinitionID = String::Factory(generic_id_);
    message.enum_ = static_cast<std::uint8_t>(contract_type_);

    FINISH_MESSAGE(__func__, getInstrumentDefinition);
}

#if OT_CASH
auto Operation::construct_download_mint() -> std::shared_ptr<Message>
{
    try {
        api_.Wallet().UnitDefinition(target_unit_id_);
    } catch (...) {
        LogOutput(OT_METHOD)(__func__)(": Invalid unit definition id");

        return {};
    }

    PREPARE_CONTEXT();
    CREATE_MESSAGE(getMint, -1, true, true);

    message.m_strInstrumentDefinitionID = String::Factory(target_unit_id_);

    FINISH_MESSAGE(__func__, getMint);
}
#endif

auto Operation::construct_get_account_data(const Identifier& accountID)
    -> std::shared_ptr<Message>
{
    PREPARE_CONTEXT();
    CREATE_MESSAGE(getAccountData, -1, true, true);

    message.m_strAcctID = String::Factory(accountID);

    FINISH_MESSAGE(__func__, getAccountData);
}

auto Operation::construct_get_transaction_numbers() -> std::shared_ptr<Message>
{
    return api_.OTAPI().getTransactionNumbers(context().get());
}

auto Operation::construct_issue_unit_definition() -> std::shared_ptr<Message>
{
    if (false == bool(unit_definition_)) {
        LogOutput(OT_METHOD)(__func__)(": Missing unit definition");

        return {};
    }

    try {
        auto contract = api_.Wallet().UnitDefinition(*unit_definition_);

        PREPARE_CONTEXT();
        CREATE_MESSAGE(registerInstrumentDefinition, -1, true, true);

        auto id = contract->ID();
        id->GetString(message.m_strInstrumentDefinitionID);
        auto serialized = proto::UnitDefinition{};
        if (false == contract->Serialize(serialized, true)) {
            LogOutput(OT_METHOD)(__func__)(
                ": Failed to serialize unit definition")
                .Flush();

            return {};
        }
        message.m_ascPayload = api_.Factory().Armored(serialized);

        FINISH_MESSAGE(__func__, registerInstrumentDefinition);
    } catch (...) {
        LogOutput(OT_METHOD)(__func__)(": Invalid unit definition").Flush();

        return {};
    }
}

auto Operation::construct_publish_nym() -> std::shared_ptr<Message>
{
    const auto contract = api_.Wallet().Nym(target_nym_id_);

    if (false == bool(contract)) {
        LogOutput(OT_METHOD)(__func__)(": Nym not found: ")(target_nym_id_)
            .Flush();

        return {};
    }

    PREPARE_CONTEXT();
    CREATE_MESSAGE(registerContract, -1, true, true);

    message.enum_ = static_cast<std::uint8_t>(ContractType::nym);
    auto publicNym = proto::Nym{};
    if (false == contract->Serialize(publicNym)) {
        LogOutput(OT_METHOD)(__func__)(": Failed to serialize nym: ")(
            target_nym_id_)
            .Flush();
        return {};
    }
    message.m_ascPayload = api_.Factory().Armored(publicNym);

    FINISH_MESSAGE(__func__, registerContract);
}

auto Operation::construct_publish_server() -> std::shared_ptr<Message>
{
    try {
        const auto contract = api_.Wallet().Server(target_server_id_);

        PREPARE_CONTEXT();
        CREATE_MESSAGE(registerContract, -1, true, true);

        message.enum_ = static_cast<std::uint8_t>(ContractType::server);
        auto serialized = proto::ServerContract{};
        if (false == contract->Serialize(serialized, true)) {
            LogOutput(OT_METHOD)(__func__)(
                ": Failed to serialize server contract: ")(target_server_id_)
                .Flush();

            return {};
        }
        message.m_ascPayload = api_.Factory().Armored(serialized);

        FINISH_MESSAGE(__func__, registerContract);
    } catch (...) {
        LogOutput(OT_METHOD)(__func__)(": Server not found: ")(
            target_server_id_)
            .Flush();

        return {};
    }
}

auto Operation::construct_publish_unit() -> std::shared_ptr<Message>
{
    try {
        const auto contract = api_.Wallet().UnitDefinition(target_unit_id_);

        PREPARE_CONTEXT();
        CREATE_MESSAGE(registerContract, -1, true, true);

        message.enum_ = static_cast<std::uint8_t>(ContractType::unit);
        auto serialized = proto::UnitDefinition{};
        if (false == contract->Serialize(serialized)) {
            LogOutput(OT_METHOD)(__func__)(
                ": Failed to serialize unit definition: ")(target_unit_id_)
                .Flush();

            return {};
        }
        message.m_ascPayload = api_.Factory().Armored(serialized);

        FINISH_MESSAGE(__func__, registerContract);
    } catch (...) {
        LogOutput(OT_METHOD)(__func__)(": Unit definition not found: ")(
            target_unit_id_)
            .Flush();

        return {};
    }
}

auto Operation::construct_process_inbox(
    const Identifier& accountID,
    const Ledger& payload,
    otx::context::Server& context) -> std::shared_ptr<Message>
{
    CREATE_MESSAGE(
        processInbox,
        Armored::Factory(String::Factory(payload)),
        accountID,
        -1);
    FINISH_MESSAGE(__func__, processInbox);
}

auto Operation::construct_register_account() -> std::shared_ptr<Message>
{
    try {
        api_.Wallet().UnitDefinition(target_unit_id_);

        PREPARE_CONTEXT();
        CREATE_MESSAGE(registerAccount, -1, true, true);

        message.m_strInstrumentDefinitionID = String::Factory(target_unit_id_);

        FINISH_MESSAGE(__func__, registerAccount);
    } catch (...) {
        LogOutput(OT_METHOD)(__func__)(": Invalid unit definition id");

        return {};
    }
}

auto Operation::construct_register_nym() -> std::shared_ptr<Message>
{
    PREPARE_CONTEXT();
    CREATE_MESSAGE(registerNym, -1, true, true);

    auto publicNym = proto::Nym{};
    if (false == nym.Serialize(publicNym)) {
        LogOutput(OT_METHOD)(__func__)(": Failed to serialize nym.");
        return {};
    }
    message.m_ascPayload = api_.Factory().Armored(publicNym);

    FINISH_MESSAGE(__func__, registerNym);
}

auto Operation::construct_request_admin() -> std::shared_ptr<Message>
{
    PREPARE_CONTEXT();
    CREATE_MESSAGE(requestAdmin, -1, true, true);

    message.m_strAcctID = memo_;

    FINISH_MESSAGE(__func__, requestAdmin);
}

auto Operation::construct_send_nym_object(
    const PeerObject& object,
    const Nym_p recipient,
    otx::context::Server& context,
    const RequestNumber number) -> std::shared_ptr<Message>
{
    auto envelope = api_.Factory().Armored();

    return construct_send_nym_object(
        object, recipient, context, envelope, number);
}

auto Operation::construct_send_nym_object(
    const PeerObject& object,
    const Nym_p recipient,
    otx::context::Server& context,
    Armored& senderCopy,
    const RequestNumber number) -> std::shared_ptr<Message>
{
    CREATE_MESSAGE(sendNymMessage, recipient->ID(), number, true, true);

    auto envelope = api_.Factory().Envelope();
    auto output = proto::PeerObject{};
    if (false == object.Serialize(output)) {
        LogOutput(OT_METHOD)(__func__)(": Failed to serialize object.").Flush();

        return {};
    }
    auto plaintext = api_.Factory().Armored(output, "PEER OBJECT");
    auto sealed =
        envelope->Seal({recipient, context.Nym()}, plaintext->Bytes(), reason_);

    if (sealed) { sealed &= envelope->Armored(message.m_ascPayload); }

    if (false == sealed) {
        LogOutput(OT_METHOD)(__func__)(": Failed encrypt object.").Flush();

        return {};
    }

    senderCopy.Set(message.m_ascPayload);

    {
        auto copy = api_.Factory().Envelope(senderCopy);
        auto plaintext = std::string{};

        // FIXME removing this line causes the sender to be unable to decrypt
        // this message later on. WTF is happening?
        OT_ASSERT(copy->Open(*context.Nym(), writer(plaintext), reason_));
    }

    FINISH_MESSAGE(__func__, sendNymMessage);
}

#if OT_CASH
auto Operation::construct_send_cash() -> std::shared_ptr<Message>
{
    const auto pRecipient = api_.Wallet().Nym(target_nym_id_);

    if (false == bool(pRecipient)) {
        LogOutput(OT_METHOD)(__func__)(": Recipient nym credentials not found")
            .Flush();

        return {};
    }

    if (false == bool(purse_)) {
        LogOutput(OT_METHOD)(__func__)(": Invalid purse").Flush();

        return {};
    }

    PREPARE_CONTEXT();

    const auto pObject = api_.Factory().PeerObject(context.Nym(), purse_);

    if (false == bool(pObject)) {
        LogOutput(OT_METHOD)(__func__)(": Failed to create peer object");

        return {};
    }

    auto& object = *pObject;

    return construct_send_nym_object(object, pRecipient, context, -1);
}
#endif

auto Operation::construct_send_message() -> std::shared_ptr<Message>
{
    const auto recipientNym = api_.Wallet().Nym(target_nym_id_);

    if (false == bool(recipientNym)) {
        LogOutput(OT_METHOD)(__func__)(": Recipient nym credentials not found")
            .Flush();

        return {};
    }

    if (memo_->empty()) {
        LogOutput(OT_METHOD)(__func__)(": Message is empty").Flush();

        return {};
    }

    auto contextEditor = context();
    auto& context = contextEditor.get();
    const auto& nym = *context.Nym();
    context.SetPush(enable_otx_push_.load());
    auto envelope = api_.Factory().Armored();
    const auto pObject =
        api_.Factory().PeerObject(context.Nym(), memo_->Get(), false);

    if (false == bool(pObject)) {
        LogOutput(OT_METHOD)(__func__)(": Failed to create peer object")
            .Flush();

        return {};
    }

    auto& object = *pObject;

    OT_ASSERT(contract::peer::PeerObjectType::Message == object.Type());

    auto pOutput =
        construct_send_nym_object(object, recipientNym, context, envelope, -1);

    if (false == bool(pOutput)) { return {}; }

    auto& output = *pOutput;
    const TransactionNumber number{output.m_strRequestNum->ToLong()};
    [[maybe_unused]] auto [notUsed, pOutmail] = context.InitializeServerCommand(
        MessageType::outmail, target_nym_id_, number, false, false);

    if (false == bool(pOutmail)) {
        LogOutput(OT_METHOD)(__func__)(": Failed to construct outmail").Flush();

        return {};
    }

    auto& outmail = *pOutmail;
    outmail.m_ascPayload->Set(envelope);
    outmail.SignContract(nym, reason_);
    outmail.SaveContract();
    outmail_message_ = std::move(pOutmail);

    return pOutput;
}

auto Operation::construct_send_peer_reply() -> std::shared_ptr<Message>
{
    const auto recipientNym = api_.Wallet().Nym(target_nym_id_);
    if (false == bool(recipientNym)) {
        LogOutput(OT_METHOD)(__func__)(": Recipient nym credentials not found")
            .Flush();

        return {};
    }

    if (0 == peer_reply_->Version()) {
        LogOutput(OT_METHOD)(__func__)(": Invalid reply.").Flush();

        return {};
    }

    if (0 == peer_request_->Version()) {
        LogOutput(OT_METHOD)(__func__)(": Invalid request.").Flush();

        return {};
    }

    auto contextEditor = context();
    auto& context = contextEditor.get();
    const auto& nym = *context.Nym();
    context.SetPush(enable_otx_push_.load());
    auto reply = proto::PeerReply{};
    if (false == peer_reply_->Serialize(reply)) {
        LogOutput(OT_METHOD)(__func__)(": Failed to serialize reply.").Flush();

        return {};
    }
    auto request = proto::PeerRequest{};
    if (false == peer_request_->Serialize(request)) {
        LogOutput(OT_METHOD)(__func__)(": Failed to serialize request.")
            .Flush();

        return {};
    }
    const bool saved = api_.Wallet().PeerReplyCreate(nym.ID(), request, reply);

    if (false == saved) {
        LogOutput(OT_METHOD)(__func__)(": Failed to save reply in wallet.")
            .Flush();

        return {};
    }

    const auto pObject = api_.Factory().PeerObject(
        peer_request_, peer_reply_, PEER_OBJECT_PEER_REPLY);

    if (false == bool(pObject)) {
        LogOutput(OT_METHOD)(__func__)(": Failed to create peer object")
            .Flush();

        return {};
    }

    auto& object = *pObject;

    OT_ASSERT(contract::peer::PeerObjectType::Response == object.Type());

    auto pOutput = construct_send_nym_object(object, recipientNym, context, -1);

    if (false == bool(pOutput)) { return {}; }

    return pOutput;
}

auto Operation::construct_send_peer_request() -> std::shared_ptr<Message>
{
    const auto recipientNym = api_.Wallet().Nym(target_nym_id_);

    if (false == bool(recipientNym)) {
        LogOutput(OT_METHOD)(__func__)(": Recipient nym credentials not found")
            .Flush();

        return {};
    }

    if (0 == peer_request_->Version()) {
        LogOutput(OT_METHOD)(__func__)(": Invalid request.").Flush();

        return {};
    }

    auto contextEditor = context();
    auto& context = contextEditor.get();
    const auto& nym = *context.Nym();
    context.SetPush(enable_otx_push_.load());
    auto serialized = proto::PeerRequest{};
    if (false == peer_request_->Serialize(serialized)) {
        LogOutput(OT_METHOD)(__func__)(": Failed to serialize request.")
            .Flush();

        return {};
    }
    const bool saved = api_.Wallet().PeerRequestCreate(nym.ID(), serialized);

    if (false == saved) {
        LogOutput(OT_METHOD)(__func__)(": Failed to save request in wallet.")
            .Flush();

        return {};
    }

    const auto itemID = peer_request_->ID();
    const auto pObject =
        api_.Factory().PeerObject(peer_request_, PEER_OBJECT_PEER_REQUEST);

    if (false == bool(pObject)) {
        LogOutput(OT_METHOD)(__func__)(": Failed to create peer object")
            .Flush();
        api_.Wallet().PeerRequestCreateRollback(nym.ID(), itemID);

        return {};
    }

    auto& object = *pObject;

    OT_ASSERT(contract::peer::PeerObjectType::Request == object.Type());

    auto pOutput = construct_send_nym_object(object, recipientNym, context, -1);

    if (false == bool(pOutput)) { return {}; }

    return pOutput;
}

auto Operation::construct_send_transfer() -> std::shared_ptr<Message>
{
    PREPARE_TRANSACTION_WITHOUT_BALANCE_ITEM(
        transactionType::transfer,
        originType::not_applicable,
        itemType::transfer,
        generic_id_);

    item.SetAmount(amount_);

    if (memo_->Exists()) { item.SetNote(memo_); }

    SIGN_ITEM();

    // Need to setup a dummy outbox transaction (to mimic the one that will be
    // on the server side when this pending transaction is actually put into the
    // real outbox.) When the server adds its own, and then compares the two,
    // they should both show the same pending transaction, in order for this
    // balance agreement to be valid. Otherwise the server would have to refuse
    // it for being inaccurate (server can't sign something inaccurate!) So I
    // throw a dummy on there before generating balance statement.
    std::shared_ptr<OTTransaction> outboxTransaction{
        api_.Factory()
            .Transaction(
                *outbox_,
                transactionType::pending,
                originType::not_applicable,
                1)
            .release()};

    OT_ASSERT(outboxTransaction);

    outboxTransaction->SetReferenceString(String::Factory(item));
    outboxTransaction->SetReferenceToNum(item.GetTransactionNum());
    outbox.AddTransaction(outboxTransaction);

    ADD_BALANCE_ITEM(amount_ * (-1));
    SIGN_TRANSACTION_AND_LEDGER();
    CREATE_MESSAGE(
        notarizeTransaction,
        Armored::Factory(String::Factory(ledger)),
        account_id_,
        -1);

    // Reset the temporary changes made above
    inbox_.reset(account.get().LoadInbox(nym).release());
    outbox_.reset(account.get().LoadOutbox(nym).release());
    account.Release();
    const auto workflowID = api_.Workflow().CreateTransfer(item, message);

    if (workflowID->empty()) {
        LogOutput(OT_METHOD)(__func__)(": Failed to create transfer workflow")
            .Flush();
    } else {
        LogDetail(OT_METHOD)(__func__)(": Created transfer ")(
            item.GetTransactionNum())(" workflow"
                                      " ")(workflowID)
            .Flush();
    }

    FINISH_MESSAGE(__func__, notarizeTransaction);
}

#if OT_CASH
auto Operation::construct_withdraw_cash() -> std::shared_ptr<Message>
{
    const Amount totalAmount(amount_);

    PREPARE_TRANSACTION(
        transactionType::withdrawal,
        originType::not_applicable,
        itemType::withdrawal,
        Identifier::Factory(),
        totalAmount * (-1));

    const auto& unitID = account.get().GetInstrumentDefinitionID();
    const bool exists = OTDB::Exists(
        api_,
        api_.DataFolder(),
        api_.Internal().Legacy().Mint(),
        serverID.str(),
        unitID.str(),
        "");

    if (false == exists) {
        LogOutput(OT_METHOD)(__func__)(": File does not exist: ")(
            api_.Internal().Legacy().Mint())(PathSeparator())(
            serverID)(PathSeparator())(unitID)
            .Flush();

        return {};
    }

    auto pMint{api_.Factory().Mint(
        String::Factory(serverID), String::Factory(unitID))};

    if (false == bool(pMint)) {
        LogOutput(OT_METHOD)(__func__)(": Missing mint").Flush();

        return {};
    }

    auto& mint = *pMint;
    const bool validMint = mint.LoadMint() && mint.VerifyMint(serverNym);

    if (false == validMint) {
        LogOutput(OT_METHOD)(__func__)(": Invalid mint").Flush();

        return {};
    }

    auto pPurse{
        api_.Factory().Purse(context, unitID, mint, totalAmount, reason_)};

    if (false == bool(pPurse)) {
        LogOutput(OT_METHOD)(__func__)(": Failed to construct purse").Flush();

        return {};
    }

    auto& purse = *pPurse;
    item.SetNote(String::Factory("Gimme cash!"));
    item.SetAttachment([&] {
        auto proto = proto::Purse{};
        purse.Serialize(proto);

        return api_.Factory().Data(proto);
    }());

    FINISH_TRANSACTION();
}
#endif

auto Operation::context() const -> Editor<otx::context::Server>
{
    return api_.Wallet().mutable_ServerContext(nym_id_, server_id_, reason_);
}

auto Operation::ConveyPayment(
    const identifier::Nym& recipient,
    const std::shared_ptr<const OTPayment> payment) -> bool
{
    START()

    target_nym_id_ = recipient;
    payment_ = payment;

    return start(lock, otx::OperationType::ConveyPayment, {});
}

#if OT_CASH
auto Operation::DepositCash(
    const Identifier& depositAccountID,
    const std::shared_ptr<blind::Purse> purse) -> bool
{
    if (false == bool(purse)) {
        LogOutput(OT_METHOD)(__func__)(": Invalid purse").Flush();

        return false;
    }

    auto pContext = api_.Wallet().ServerContext(nym_id_, server_id_);

    if (false == bool(pContext)) {
        LogOutput(OT_METHOD)(__func__)(": Failed to load context").Flush();

        return false;
    }

    auto& context = *pContext;
    const auto& nym = *context.Nym();
    const auto& serverNym = context.RemoteNym();

    if (false == purse->Unlock(nym, reason_)) {
        LogOutput(OT_METHOD)(__func__)(": Failed to unlock pursed").Flush();

        return false;
    }

    if (false == purse->AddNym(serverNym, reason_)) {
        LogOutput(OT_METHOD)(__func__)(": Failed to encrypt purse to notary")
            .Flush();

        return false;
    }

    START()

    account_id_ = depositAccountID;
    affected_accounts_.insert(depositAccountID);
    purse_ = purse;

    return start(lock, otx::OperationType::DepositCash, {});
}
#endif

auto Operation::DepositCheque(
    const Identifier& depositAccountID,
    const std::shared_ptr<Cheque> cheque) -> bool
{
    START()

    account_id_ = depositAccountID;
    affected_accounts_.insert(depositAccountID);
    cheque_ = cheque;

    return start(lock, otx::OperationType::DepositCheque, {});
}

auto Operation::download_accounts(
    const State successState,
    const State failState,
    otx::context::Server::DeliveryResult& lastResult) -> bool
{
    if (affected_accounts_.empty()) {
        LogOutput(OT_METHOD)(__func__)(": Warning: no accounts to update")
            .Flush();
        state_.store(successState);

        return true;
    }

    std::size_t ready{0};

    for (const auto& accountID : affected_accounts_) {
        if (shutdown().load()) { return false; }

        ready += download_account(accountID, lastResult);
    }

    if (affected_accounts_.size() == ready) {
        LogDetail(OT_METHOD)(__func__)(": All accounts synchronized").Flush();
        state_.store(successState);

        return true;
    }

    LogOutput(OT_METHOD)(__func__)(": Retrying account synchronization")
        .Flush();
    state_.store(failState);

    return false;
}

auto Operation::download_account(
    const Identifier& accountID,
    otx::context::Server::DeliveryResult& lastResult) -> std::size_t
{
    std::shared_ptr<Ledger> inbox{api_.Factory().Ledger(
        nym_id_, accountID, server_id_, ledgerType::inbox)};
    std::shared_ptr<Ledger> outbox{api_.Factory().Ledger(
        nym_id_, accountID, server_id_, ledgerType::outbox)};

    OT_ASSERT(inbox);
    OT_ASSERT(outbox);
    OT_ASSERT(ledgerType::inbox == inbox->GetType());
    OT_ASSERT(ledgerType::outbox == outbox->GetType());

    if (get_account_data(accountID, inbox, outbox, lastResult)) {
        LogDetail(OT_METHOD)(__func__)(": Success downloading account ")(
            accountID)
            .Flush();
    } else {
        LogOutput(OT_METHOD)(__func__)(": Failed downloading account ")(
            accountID)
            .Flush();

        return 0;
    }

    if (shutdown().load()) { return 0; }

    if (get_receipts(accountID, inbox, outbox)) {
        LogDetail(OT_METHOD)(__func__)(": Success synchronizing account ")(
            accountID)
            .Flush();
    } else {
        LogOutput(OT_METHOD)(__func__)(": Failed synchronizing account ")(
            accountID)
            .Flush();

        return 0;
    }

    if (shutdown().load()) { return 0; }

    if (process_inbox(accountID, inbox, outbox, lastResult)) {
        LogDetail(OT_METHOD)(__func__)(": Success processing inbox ")(accountID)
            .Flush();

        return 1;
    } else {
        LogOutput(OT_METHOD)(__func__)(": Failed processing inbox ")(accountID)
            .Flush();

        return 0;
    }
}

auto Operation::download_box_receipt(
    const Identifier& accountID,
    const BoxType box,
    const TransactionNumber number) -> bool
{
    PREPARE_CONTEXT();

    [[maybe_unused]] auto [requestNumber, message] =
        context.InitializeServerCommand(
            MessageType::getBoxReceipt, -1, false, false);

    if (false == bool(message)) {
        LogOutput(OT_METHOD)(__func__)(": Failed to construct message").Flush();

        return false;
    }

    std::shared_ptr<Message> command{message.release()};

    OT_ASSERT(command);

    command->m_strAcctID = String::Factory(accountID);
    command->m_lDepth = static_cast<std::int32_t>(box);
    command->m_lTransactionNum = number;
    const auto finalized = context.FinalizeServerCommand(*command, reason_);

    if (false == finalized) {
        LogOutput(OT_METHOD)(__func__)(": Failed to sign message").Flush();

        return false;
    }

    context.SetPush(enable_otx_push_.load());
    auto result = context.Queue(api_, command, reason_, {});

    while (false == bool(result)) {
        LogTrace(OT_METHOD)(__func__)(": Context is busy").Flush();
        Sleep(std::chrono::milliseconds(OPERATION_POLL_MILLISECONDS));
        result = context.Queue(api_, command, reason_, {});
    }

    OT_ASSERT(result);

    while (check_future(*result)) {
        if (shutdown().load()) { return false; }
    }

    return otx::LastReplyStatus::MessageSuccess == std::get<0>(result->get());
}

auto Operation::DownloadContract(const Identifier& ID, const ContractType type)
    -> bool
{
    START()

    generic_id_ = ID;
    contract_type_ = type;

    return start(lock, otx::OperationType::DownloadContract, {});
}

void Operation::evaluate_transaction_reply(
    otx::context::Server::DeliveryResult&& result)
{
    auto& pMessage = result.second;

    OT_ASSERT(pMessage);

    auto& message = *pMessage;
    auto accountID = Identifier::Factory(message.m_strAcctID);
    const auto success = evaluate_transaction_reply(accountID, message);

    if (false == success) {
        // TODO determine if failure was caused by a consensus hash failure.
        // In this case, set the state to NymboxPre
    }

    set_result(std::move(result));
    state_.store(State::AccountPost);
}

auto Operation::evaluate_transaction_reply(
    const Identifier& accountID,
    const Message& reply) const -> bool
{
    if (false == reply.m_bSuccess) {
        LogOutput(OT_METHOD)(__func__)(": Message failure").Flush();

        return false;
    }

    switch (Message::Type(reply.m_strCommand->Get())) {
        case MessageType::notarizeTransactionResponse:
        case MessageType::processInboxResponse:
        case MessageType::processNymboxResponse:
            break;
        default: {
            LogOutput(OT_METHOD)(__func__)(": ")(reply.m_strCommand)(
                " is not a transaction")
                .Flush();

            return false;
        }
    }

    const auto serialized = String::Factory(reply.m_ascPayload);

    if (false == serialized->Exists()) {
        LogNormal(OT_METHOD)(__func__)(": No response ledger found on message.")
            .Flush();

        return false;
    }

    auto response{api_.Factory().Ledger(nym_id_, accountID, server_id_)};

    OT_ASSERT(response);

    if (false == response->LoadContractFromString(serialized)) {
        LogOutput(OT_METHOD)(__func__)(
            ": Unable to deserialize response ledger")
            .Flush();

        return false;
    }

    const std::size_t count = (response->GetTransactionCount() >= 0)
                                  ? response->GetTransactionCount()
                                  : 0;
    std::size_t good{0};

    if (0 == count) {
        LogOutput(OT_METHOD)(__func__)(
            ": Response ledger does not contain a transaction")
            .Flush();

        return false;
    }

    for (auto& [number, pTransaction] : response->GetTransactionMap()) {
        if (1 > number) {
            LogOutput(OT_METHOD)(__func__)(": Invalid transaction number ")(
                number)
                .Flush();
            continue;
        }

        if (false == bool(pTransaction)) {
            LogOutput(OT_METHOD)(__func__)(": Invalid transaction ")(number)
                .Flush();
            continue;
        }

        auto& transaction = *pTransaction;

        if (transaction.GetSuccess()) {
            LogVerbose(OT_METHOD)(__func__)(": Successful transaction ")(number)
                .Flush();
            ++good;
        } else {
            LogVerbose(OT_METHOD)(__func__)(": Failed transaction ")(number)
                .Flush();
        }
    }

    return count == good;
}

void Operation::execute()
{
    if (refresh_account_.load()) {
        state_.store(State::AccountPost);

        return;
    }

    if (result_set_.load()) {
        state_.store(State::AccountPost);

        return;
    }

    if (message_) {
        refresh();
    } else {
        message_ = construct();
    }

    if (false == bool(message_)) {
        LogOutput(OT_METHOD)(__func__)(": Failed to construct command").Flush();
        ++error_count_;

        return;
    }

    const auto& category = category_.at(type_.load());

    PREPARE_CONTEXT();

    otx::context::Server::QueueResult result{};

    if (Category::Transaction == category) {
        result = context.Queue(
            api_, message_, inbox_, outbox_, &numbers_, reason_, args_);
    } else {
        result = context.Queue(api_, message_, reason_, args_);
    }

    if (false == bool(result)) {
        LogTrace(OT_METHOD)(__func__)(": Context is busy").Flush();
        Sleep(std::chrono::milliseconds(OPERATION_POLL_MILLISECONDS));

        return;
    }

    while (check_future(*result)) {
        if (shutdown().load()) { return; }
    }

    auto finished = result->get();
    update_workflow(*message_, finished);

    switch (std::get<0>(finished)) {
        case otx::LastReplyStatus::MessageSuccess: {
            if (Category::Transaction == category) {
                evaluate_transaction_reply(std::move(finished));
            } else if (Category::CreateAccount == category) {
                OT_ASSERT(finished.second);

                auto& reply = finished.second;
                const auto accountID = Identifier::Factory(reply->m_strAcctID);
                affected_accounts_.emplace(std::move(accountID));
                set_result(std::move(finished));
                state_.store(State::AccountPost);
            } else {
                if (otx::OperationType::SendMessage == type_.load()) {
                    OT_ASSERT(outmail_message_);

                    const auto messageID = api_.Activity().Mail(
                        nym_id_,
                        *outmail_message_,
                        StorageBox::MAILOUTBOX,
                        memo_->Get());

                    if (set_id_) { set_id_(Identifier::Factory(messageID)); }

                    set_result(std::move(finished));
                } else {
                    set_result(std::move(finished));
                }

                state_.store(State::NymboxPost);
            }
        } break;
        case otx::LastReplyStatus::MessageFailed: {
            ++error_count_;
            state_.store(State::NymboxPre);
        } break;
        default: {
            return;
        }
    }
}

auto Operation::get_account_data(
    const Identifier& accountID,
    std::shared_ptr<Ledger> inbox,
    std::shared_ptr<Ledger> outbox,
    otx::context::Server::DeliveryResult& lastResult) -> bool
{
    auto message = construct_get_account_data(accountID);

    if (false == bool(message)) {
        LogOutput(OT_METHOD)(__func__)(": Failed to construct command").Flush();

        return false;
    }

    PREPARE_CONTEXT();

    auto result = context.Queue(api_, message, inbox, outbox, {}, reason_);

    if (false == bool(result)) {
        LogTrace(OT_METHOD)(__func__)(": Context is busy").Flush();
        Sleep(std::chrono::milliseconds(OPERATION_POLL_MILLISECONDS));

        return false;
    }

    while (check_future(*result)) {
        if (shutdown().load()) { return false; }
    }

    lastResult = result->get();

    return otx::LastReplyStatus::MessageSuccess == std::get<0>(lastResult);
}

auto Operation::get_receipts(
    const Identifier& accountID,
    std::shared_ptr<Ledger> inbox,
    std::shared_ptr<Ledger> outbox) -> bool
{
    OT_ASSERT(inbox);
    OT_ASSERT(outbox);

    bool output{true};

    output &= get_receipts(accountID, BoxType::Inbox, *inbox);
    output &= get_receipts(accountID, BoxType::Outbox, *outbox);

    return output;
}

auto Operation::get_receipts(
    const Identifier& accountID,
    const BoxType type,
    Ledger& box) -> bool
{
    const auto count = box.GetTransactionMap().size();
    std::size_t good{0};

    if (0 == count) {
        LogDetail(OT_METHOD)(__func__)(": Box is empty").Flush();
    }

    for (auto [number, pItem] : box.GetTransactionMap()) {
        if (1 > number) {
            LogOutput(OT_METHOD)(__func__)(": Invalid transaction number ")(
                number)
                .Flush();

            continue;
        }

        if (false == bool(pItem)) {
            LogOutput(OT_METHOD)(__func__)(": Warning: Invalid item ")(number)
                .Flush();
        }

        const auto exists = VerifyBoxReceiptExists(
            api_,
            api_.DataFolder(),
            server_id_,
            nym_id_,
            accountID,
            static_cast<std::int32_t>(type),
            number);

        if (exists) {
            LogDetail(OT_METHOD)(__func__)(": Receipt ")(
                number)(" already exists.")
                .Flush();
            ++good;

            continue;
        }

        if (download_box_receipt(accountID, type, number)) {
            LogDetail(OT_METHOD)(__func__)(": Downloaded receipt ")(number)
                .Flush();
            ++good;
        } else {
            LogOutput(OT_METHOD)(__func__)(": Failed to download receipt ")(
                number)
                .Flush();
        }
    }

    return count == good;
}

auto Operation::hasContext() const -> bool
{
    const auto context = api_.Wallet().ServerContext(nym_id_, server_id_);

    return bool(context);
}

auto Operation::GetFuture() -> Operation::Future
{
    return result_.get_future();
}

auto Operation::IssueUnitDefinition(
    const std::shared_ptr<const proto::UnitDefinition> unitDefinition,
    const otx::context::Server::ExtraArgs& args) -> bool
{
    if (false == bool(unitDefinition)) {
        LogOutput(OT_METHOD)(__func__)(": Missing unit definition").Flush();

        return false;
    }

    if (false == proto::Validate(*unitDefinition, VERBOSE)) {
        LogOutput(OT_METHOD)(__func__)(": Invalid unit definition").Flush();

        return false;
    }

    START()

    unit_definition_ = unitDefinition;

    return start(lock, otx::OperationType::IssueUnitDefinition, args);
}

auto Operation::IssueUnitDefinition(
    const ReadView& view,
    const otx::context::Server::ExtraArgs& args) -> bool
{
    auto unitdefinition = std::make_shared<const proto::UnitDefinition>(
        proto::Factory<proto::UnitDefinition>(view));
    return IssueUnitDefinition(unitdefinition, args);
}

void Operation::join()
{
    while (State::Idle != state_.load()) {
        Sleep(std::chrono::milliseconds(OPERATION_JOIN_MILLISECONDS));
    }
}

void Operation::nymbox_post()
{
    auto contextEditor = context();
    auto& context = contextEditor.get();
    context.SetPush(enable_otx_push_.load());
    auto mismatch = !context.NymboxHashMatch();
    bool post{false};

    switch (category_.at(type_)) {
        case Category::Transaction:
        case Category::UpdateAccount:
        case Category::CreateAccount:
        case Category::NymboxPre:
        case Category::NymboxPost: {
            post = true;
        } break;
        case Category::Basic:
        default: {
            state_.store(State::Idle);
        }
    }

    if (post || mismatch) {
        auto result = context.RefreshNymbox(api_, reason_);

        if (false == bool(result)) {
            LogTrace(OT_METHOD)(__func__)(": Context is busy").Flush();

            return;
        }

        while (check_future(*result)) {
            if (shutdown().load()) { return; }
        }

        switch (std::get<0>(result->get())) {
            case otx::LastReplyStatus::MessageSuccess: {
                if (context.NymboxHashMatch()) { state_.store(State::Idle); }
            } break;
            default: {
            }
        }
    }
}

void Operation::nymbox_pre()
{
    bool needInbox{false};

    switch (category_.at(type_)) {
        case Category::UpdateAccount:
        case Category::Transaction: {
            needInbox = true;
            [[fallthrough]];
        }
        case Category::CreateAccount:
        case Category::NymboxPre: {
            auto contextEditor = context();
            auto& context = contextEditor.get();
            context.SetPush(enable_otx_push_.load());

            if (context.NymboxHashMatch()) {
                if (needInbox) {
                    state_.store(State::TransactionNumbers);
                } else {
                    state_.store(State::Execute);
                }

                return;
            }

            auto result = context.RefreshNymbox(api_, reason_);

            if (false == bool(result)) {
                LogTrace(OT_METHOD)(__func__)(": Context is busy").Flush();

                break;
            }

            while (check_future(*result)) {
                if (shutdown().load()) { return; }
            }

            switch (std::get<0>(result->get())) {
                case otx::LastReplyStatus::MessageSuccess: {
                    if (needInbox) {
                        state_.store(State::TransactionNumbers);
                    } else {
                        state_.store(State::Execute);
                    }
                } break;
                default: {
                }
            }
        } break;
        case Category::NymboxPost:
        case Category::Basic:
        default: {
            state_.store(State::Execute);
        }
    }
}

auto Operation::process_inbox(
    const Identifier& accountID,
    std::shared_ptr<Ledger> inbox,
    std::shared_ptr<Ledger> outbox,
    otx::context::Server::DeliveryResult& lastResult) -> bool
{
    class Cleanup
    {
    public:
        void SetSuccess() { recover_ = false; }

        Cleanup(const TransactionNumber number, otx::context::Server& context)
            : context_(context)
            , number_(number)
        {
        }
        Cleanup() = delete;
        ~Cleanup()
        {
            if (recover_ && (0 != number_)) {
                LogOutput(OT_METHOD)(__func__)(": Recovering unused number ")(
                    number_)(".")
                    .Flush();
                const bool recovered = context_.RecoverAvailableNumber(number_);

                if (false == recovered) {
                    LogOutput(OT_METHOD)(__func__)("Failed.").Flush();
                }
            }
        }

    private:
        otx::context::Server& context_;
        const TransactionNumber number_{0};
        bool recover_{true};
    };

    OT_ASSERT(inbox);
    OT_ASSERT(outbox);
    OT_ASSERT(ledgerType::inbox == inbox->GetType());
    OT_ASSERT(ledgerType::outbox == outbox->GetType());

    const auto count =
        (inbox->GetTransactionCount() > 0) ? inbox->GetTransactionCount() : 0;

    if (1 > count) {
        LogDetail(OT_METHOD)(__func__)(": No items to accept in account ")(
            accountID)
            .Flush();

        return true;
    } else {
        LogDetail(OT_METHOD)(__func__)(": ")(
            count)(" items to accept in account ")(accountID)
            .Flush();
        redownload_accounts_.insert(accountID);
    }

    PREPARE_CONTEXT();

    auto [response, recoverNumber] =
        api_.OTAPI().CreateProcessInbox(accountID, context, *inbox);

    if (false == bool(response)) {
        LogOutput(OT_METHOD)(__func__)(
            ": Error instantiating processInbox for account: ")(accountID)
            .Flush();

        return false;
    }

    Cleanup cleanup(recoverNumber, context);

    for (auto i = std::int32_t{0}; i < count; ++i) {
        auto transaction = inbox->GetTransactionByIndex(i);

        if (false == bool(transaction)) {
            LogOutput(OT_METHOD)(__func__)(": Invalid transaction").Flush();

            return false;
        }

        const auto number = transaction->GetTransactionNum();

        if (transaction->IsAbbreviated()) {
            inbox->LoadBoxReceipt(number);
            transaction = inbox->GetTransaction(number);

            if (false == bool(transaction)) {
                LogOutput(OT_METHOD)(__func__)(": Unable to load item: ")(
                    number)(".")
                    .Flush();

                continue;
            }
        }

        // TODO This should happen when the box receipt is downloaded
        if (transactionType::chequeReceipt == transaction->GetType()) {
            const auto workflowUpdated =
                api_.Workflow().ClearCheque(context.Nym()->ID(), *transaction);

            if (workflowUpdated) {
                LogVerbose(OT_METHOD)(__func__)(": Updated workflow.").Flush();
            } else {
                LogOutput(OT_METHOD)(__func__)(": Failed to update workflow.")
                    .Flush();
            }
        }

        const bool accepted = api_.OTAPI().IncludeResponse(
            accountID, true, context, *transaction, *response);

        if (false == accepted) {
            LogOutput(OT_METHOD)(__func__)(": Failed to accept item: ")(number)
                .Flush();

            return false;
        }
    }

    const bool finalized = api_.OTAPI().FinalizeProcessInbox(
        accountID, context, *response, *inbox, *outbox, reason_);

    if (false == finalized) {
        LogOutput(OT_METHOD)(__func__)(": Unable to finalize response.")
            .Flush();

        return false;
    }

    auto message = construct_process_inbox(accountID, *response, context);

    if (false == bool(message)) {
        LogOutput(OT_METHOD)(__func__)(": Failed to construct command").Flush();

        return false;
    }

    auto result = context.Queue(api_, message, reason_, {});

    while (false == bool(result)) {
        LogTrace(OT_METHOD)(__func__)(": Context is busy").Flush();
        Sleep(std::chrono::milliseconds(OPERATION_POLL_MILLISECONDS));
        result = context.Queue(api_, message, reason_, {});
    }

    while (check_future(*result)) {
        if (shutdown().load()) { return false; }
    }

    lastResult = result->get();
    const auto [status, reply] = lastResult;

    if (otx::LastReplyStatus::MessageSuccess != status) {
        LogOutput(OT_METHOD)(__func__)(": Failed to deliver processInbox ")(
            value(status))
            .Flush();

        return false;
    }

    cleanup.SetSuccess();

    OT_ASSERT(reply);

    const auto success = evaluate_transaction_reply(accountID, *reply);

    if (success) {
        LogDetail(OT_METHOD)(__func__)(": Success processing inbox ")(accountID)
            .Flush();
    } else {
        LogOutput(OT_METHOD)(__func__)(": Failure processing inbox ")(accountID)
            .Flush();
    }

    return success;
}

auto Operation::PublishContract(const identifier::Nym& id) -> bool
{
    START()

    target_nym_id_ = id;

    return start(lock, otx::OperationType::PublishNym, {});
}

auto Operation::PublishContract(const identifier::Server& id) -> bool
{
    START()

    target_server_id_ = id;

    return start(lock, otx::OperationType::PublishServer, {});
}

auto Operation::PublishContract(const identifier::UnitDefinition& id) -> bool
{
    START()

    target_unit_id_ = id;

    return start(lock, otx::OperationType::PublishUnit, {});
}

void Operation::refresh()
{
    OT_ASSERT(message_);

    context().get().UpdateRequestNumber(*message_, reason_);
}

auto Operation::RequestAdmin(const String& password) -> bool
{
    START()

    memo_ = password;

    return start(lock, otx::OperationType::RequestAdmin, {});
}

void Operation::reset()
{
    state_.store(State::NymboxPre);
    refresh_account_.store(false);
    message_.reset();
    outmail_message_.reset();
    result_set_.store(false);
    result_ = Promise{};
    target_nym_id_ = identifier::Nym::Factory();
    target_server_id_ = identifier::Server::Factory();
    target_unit_id_ = identifier::UnitDefinition::Factory();
    contract_type_ = ContractType::invalid;
    unit_definition_.reset();
    account_id_ = Identifier::Factory();
    generic_id_ = Identifier::Factory();
    amount_ = 0;
    memo_ = String::Factory();
    bool_ = false;
    claim_section_ = contact::ContactSectionName::Error;
    claim_type_ = contact::ContactItemType::Error;
    cheque_.reset();
    payment_.reset();
    inbox_.reset();
    outbox_.reset();
#if OT_CASH
    purse_.reset();
#endif
    affected_accounts_.clear();
    redownload_accounts_.clear();
    numbers_.clear();
    error_count_ = 0;
    peer_reply_ = api_.Factory().PeerReply();
    peer_request_ = api_.Factory().PeerRequest();
    set_id_ = {};
}

#if OT_CASH
auto Operation::SendCash(
    const identifier::Nym& recipientID,
    const Identifier& workflowID) -> bool
{
    const auto pSender = api_.Wallet().Nym(nym_id_);
    const auto pRecipient = api_.Wallet().Nym(recipientID);

    if (false == bool(pRecipient)) {
        LogOutput(OT_METHOD)(__func__)(": Failed to load recipient nym")
            .Flush();

        return false;
    }

    if (false == bool(pSender)) {
        LogOutput(OT_METHOD)(__func__)(": Failed to load sender nym").Flush();

        return false;
    }

    const auto& recipient = *pRecipient;
    const auto& sender = *pSender;

    try {
        const auto workflow = [&] {
            auto out = proto::PaymentWorkflow{};

            if (!api_.Workflow().LoadWorkflow(nym_id_, workflowID, out)) {
                throw std::runtime_error{"Failed to load workflow"};
            }

            return out;
        }();

        auto [state, pPurse] =
            api::client::Workflow::InstantiatePurse(api_, workflow);

        if (api::client::PaymentWorkflowState::Unsent != state) {
            LogOutput(OT_METHOD)(__func__)(": Incorrect workflow state")
                .Flush();

            return false;
        }

        if (false == bool(pPurse)) {
            LogOutput(OT_METHOD)(__func__)(": Purse not found").Flush();

            return false;
        }

        auto& purse = *pPurse;

        if (false == purse.Unlock(sender, reason_)) {
            LogOutput(OT_METHOD)(__func__)(": Failed to unlock pursed").Flush();

            return false;
        }

        if (false == purse.AddNym(recipient, reason_)) {
            LogOutput(OT_METHOD)(__func__)(
                ": Failed to encrypt purse to recipient")
                .Flush();

            return false;
        }

        START()

        target_nym_id_ = recipientID;
        generic_id_ = workflowID;
        purse_ = std::move(pPurse);

        return start(lock, otx::OperationType::SendCash, {});
    } catch (const std::exception& e) {
        LogOutput(OT_METHOD)(__func__)(": ")(e.what()).Flush();

        return false;
    }
}
#endif

auto Operation::SendMessage(
    const identifier::Nym& recipient,
    const String& message,
    const SetID setID) -> bool
{
    START()

    target_nym_id_ = recipient;
    memo_ = message;
    set_id_ = setID;

    return start(lock, otx::OperationType::SendMessage, {});
}

auto Operation::SendPeerReply(
    const identifier::Nym& targetNymID,
    const OTPeerReply peerreply,
    const OTPeerRequest peerrequest) -> bool
{
    START()

    target_nym_id_ = targetNymID;
    peer_reply_ = peerreply;
    peer_request_ = peerrequest;

    return start(lock, otx::OperationType::SendPeerReply, {});
}

auto Operation::SendPeerRequest(
    const identifier::Nym& targetNymID,
    const OTPeerRequest peerrequest) -> bool
{
    START()

    target_nym_id_ = targetNymID;
    peer_request_ = peerrequest;

    return start(lock, otx::OperationType::SendPeerRequest, {});
}

auto Operation::SendTransfer(
    const Identifier& sourceAccountID,
    const Identifier& destinationAccountID,
    const Amount amount,
    const String& memo) -> bool
{
    START()

    account_id_ = sourceAccountID;
    generic_id_ = destinationAccountID;
    amount_ = amount;
    memo_ = memo;
    affected_accounts_.insert(sourceAccountID);

    return start(lock, otx::OperationType::SendTransfer, {});
}

void Operation::set_consensus_hash(
    OTTransaction& transaction,
    const otx::context::Base& context,
    const Account& account,
    const opentxs::PasswordPrompt& reason)
{
    auto accountHash{Identifier::Factory()};
    auto accountid{Identifier::Factory()};
    auto inboxHash{Identifier::Factory()};
    auto outboxHash{Identifier::Factory()};
    account.GetIdentifier(accountid);
    account.ConsensusHash(context, accountHash, reason);
    auto nymfile = context.Nymfile(reason);
    nymfile->GetInboxHash(accountid->str(), inboxHash);
    nymfile->GetOutboxHash(accountid->str(), outboxHash);
    transaction.SetAccountHash(accountHash);
    transaction.SetInboxHash(inboxHash);
    transaction.SetOutboxHash(outboxHash);
}

void Operation::set_result(otx::context::Server::DeliveryResult&& result)
{
    result_set_.store(true);
    result_.set_value(std::move(result));
}

void Operation::Shutdown() { Stop(); }

auto Operation::Start(
    const otx::OperationType type,
    const otx::context::Server::ExtraArgs& args) -> bool
{
    START()

    switch (type) {
        case otx::OperationType::GetTransactionNumbers:
        case otx::OperationType::RegisterNym: {
            break;
        }
        default: {
            LogOutput(OT_METHOD)(__func__)(": Incorrect arguments").Flush();

            return false;
        }
    }

    return start(lock, type, args);
}

auto Operation::Start(
    const otx::OperationType type,
    const identifier::UnitDefinition& targetUnitID,
    const otx::context::Server::ExtraArgs& args) -> bool
{
    START()

    switch (type) {
        case otx::OperationType::DownloadMint:
        case otx::OperationType::RegisterAccount: {
            break;
        }
        default: {
            LogOutput(OT_METHOD)(__func__)(": Incorrect arguments").Flush();

            return false;
        }
    }

    target_unit_id_ = targetUnitID;

    return start(lock, type, args);
}

auto Operation::Start(
    const otx::OperationType type,
    const identifier::Nym& targetNymID,
    const otx::context::Server::ExtraArgs& args) -> bool
{
    START()

    switch (type) {
        case otx::OperationType::CheckNym: {
            break;
        }
        default: {
            LogOutput(OT_METHOD)(__func__)(": Incorrect arguments").Flush();

            return false;
        }
    }

    target_nym_id_ = targetNymID;

    return start(lock, type, args);
}

auto Operation::start(
    const Lock& decisionLock,
    const otx::OperationType type,
    const otx::context::Server::ExtraArgs& args) -> bool
{
    type_.store(type);
    args_ = args;

    if (otx::OperationType::RefreshAccount == type) {
        refresh_account_.store(true);
    }

    return trigger(decisionLock);
}

auto Operation::state_machine() -> bool
{
    switch (state_.load()) {
        case State::NymboxPre: {
            nymbox_pre();
        } break;
        case State::AccountPre: {
            account_pre();
        } break;
        case State::TransactionNumbers: {
            transaction_numbers();
        } break;
        case State::Execute: {
            execute();
        } break;
        case State::AccountPost: {
            account_post();
        } break;
        case State::NymboxPost: {
            nymbox_post();
        } break;
        default: {
            LogOutput(OT_METHOD)(__func__)(": Unexpected state").Flush();
        }
    }

    if (State::Idle == state_.load()) {
        LogDetail(OT_METHOD)(__func__)(": Success").Flush();

        return false;
    }

    if (error_count_ > MAX_ERROR_COUNT) {
        LogOutput(OT_METHOD)(__func__)(": Error count exceeded").Flush();
        set_result({otx::LastReplyStatus::Unknown, nullptr});
        state_.store(State::Idle);

        return false;
    }

    return true;
}

void Operation::transaction_numbers()
{
    switch (category_.at(type_.load())) {
        case Category::UpdateAccount:
        case Category::Transaction: {
            break;
        }
        case Category::Invalid:
        case Category::Basic:
        case Category::NymboxPost:
        case Category::NymboxPre:
        case Category::CreateAccount:
        default: {
            state_.store(State::Execute);

            return;
        }
    }

    PREPARE_CONTEXT();

    const auto need = transaction_numbers_.at(type_.load());

    if (context.AvailableNumbers() >= need) {
        state_.store(State::AccountPre);

        return;
    }

    std::shared_ptr<Message> message{
        api_.OTAPI().getTransactionNumbers(context)};

    if (false == bool(message)) { return; }

    auto result = context.Queue(api_, message, reason_, {});

    if (false == bool(result)) {
        LogTrace(OT_METHOD)(__func__)(": Context is busy").Flush();
        Sleep(std::chrono::milliseconds(OPERATION_POLL_MILLISECONDS));

        return;
    }

    while (check_future(*result)) {
        if (shutdown().load()) { return; }
    }

    if (otx::LastReplyStatus::MessageSuccess == std::get<0>(result->get())) {
        auto nymbox = context.RefreshNymbox(api_, reason_);

        while (false == bool(nymbox)) {
            if (shutdown().load()) { return; }
            LogTrace(OT_METHOD)(__func__)(": Context is busy").Flush();
            Sleep(std::chrono::milliseconds(OPERATION_POLL_MILLISECONDS));
            nymbox = context.RefreshNymbox(api_, reason_);
        }

        while (check_future(*nymbox)) {
            if (shutdown().load()) { return; }
        }

        [[maybe_unused]] auto done = nymbox->get();
        state_.store(State::NymboxPre);
    }
}

void Operation::update_workflow(
    const Message& request,
    const otx::context::Server::DeliveryResult& result) const
{
    switch (type_.load()) {
        case otx::OperationType::ConveyPayment: {
            update_workflow_convey_payment(request, result);
        } break;
#if OT_CASH
        case otx::OperationType::SendCash: {
            update_workflow_send_cash(request, result);
        } break;
#endif
        default: {
        }
    }
}

void Operation::update_workflow_convey_payment(
    const Message& request,
    const otx::context::Server::DeliveryResult& result) const
{
    const auto& [status, reply] = result;

    OT_ASSERT(payment_);

    if (false == payment_->IsCheque()) {
        // TODO Handle other payment types once the workflow api handles them

        return;
    }

    bool workflowUpdated{false};
    auto pCheque{api_.Factory().Cheque()};

    OT_ASSERT(pCheque);

    auto& cheque = *pCheque;
    const auto loaded = cheque.LoadContractFromString(payment_->Payment());

    if (false == loaded) {
        LogOutput(OT_METHOD)(__func__)(": Failed to load cheque.").Flush();

        return;
    }

    if (const_cast<OTPayment&>(*payment_).IsCancelledCheque(reason_)) {
        workflowUpdated =
            api_.Workflow().CancelCheque(cheque, request, reply.get());
    } else {
        workflowUpdated =
            api_.Workflow().SendCheque(cheque, request, reply.get());
    }

    if (workflowUpdated) {
        LogDetail(OT_METHOD)(__func__)(": Successfully updated workflow.")
            .Flush();
    } else {
        LogOutput(OT_METHOD)(__func__)(": Failed to update workflow.").Flush();
    }
}

#if OT_CASH
void Operation::update_workflow_send_cash(
    const Message& request,
    const otx::context::Server::DeliveryResult& result) const
{
    api_.Workflow().SendCash(
        nym_id_, target_nym_id_, generic_id_, request, result.second.get());
}
#endif

auto Operation::UpdateAccount(const Identifier& accountID) -> bool
{
    START()

    affected_accounts_.insert(accountID);
    redownload_accounts_.clear();

    return start(lock, otx::OperationType::RefreshAccount, {});
}

#if OT_CASH
auto Operation::WithdrawCash(const Identifier& accountID, const Amount amount)
    -> bool
{
    START()

    account_id_ = accountID;
    amount_ = amount;
    affected_accounts_.insert(accountID);

    return start(lock, otx::OperationType::WithdrawCash, {});
}
#endif

Operation::~Operation()
{
    Stop().get();

    if (hasContext()) { context().get().Join(); }

    const bool needPromise =
        (false == result_set_.load()) && (State::Idle != state_.load());

    if (needPromise) { set_result({otx::LastReplyStatus::Unknown, nullptr}); }
}
}  // namespace opentxs::otx::client::implementation
