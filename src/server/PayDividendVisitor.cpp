// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                   // IWYU pragma: associated
#include "1_Internal.hpp"                 // IWYU pragma: associated
#include "server/PayDividendVisitor.hpp"  // IWYU pragma: associated

#include <chrono>
#include <cstdint>
#include <memory>

#include "opentxs/Pimpl.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/Editor.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/api/Wallet.hpp"
#include "opentxs/api/server/Manager.hpp"
#include "opentxs/core/Account.hpp"
#include "opentxs/core/AccountVisitor.hpp"
#include "opentxs/core/Cheque.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/core/identifier/Server.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"
#include "opentxs/identity/Nym.hpp"
#include "server/Server.hpp"
#include "server/Transactor.hpp"

#define OT_METHOD "opentxs::PayDividendVisitor::"

namespace opentxs
{
PayDividendVisitor::PayDividendVisitor(
    server::Server& server,
    const identifier::Server& theNotaryID,
    const identifier::Nym& theNymID,
    const identifier::UnitDefinition& thePayoutUnitTypeId,
    const Identifier& theVoucherAcctID,
    const String& strMemo,
    std::int64_t lPayoutPerShare)
    : AccountVisitor(server.API().Wallet(), theNotaryID)
    , server_(server)
    , nymId_(theNymID)
    , payoutUnitTypeId_(thePayoutUnitTypeId)
    , voucherAcctId_(theVoucherAcctID)
    , m_pstrMemo(String::Factory(strMemo.Get()))
    , m_lPayoutPerShare(lPayoutPerShare)
    , m_lAmountPaidOut(0)
    , m_lAmountReturned(0)
{
}

// For each "user" account of a specific instrument definition, this function
// is called in order to pay a dividend to the Nym who owns that account.

// PayDividendVisitor::Trigger() is used in
// OTUnitDefinition::VisitAccountRecords()
// cppcheck-suppress unusedFunction
auto PayDividendVisitor::Trigger(
    const Account& theSharesAccount,
    const PasswordPrompt& reason) -> bool  // theSharesAccount
                                           // is, say, a Pepsi
                                           // shares
// account.  Here, we'll send a dollars voucher
// to its owner.
{
    const std::int64_t lPayoutAmount =
        (theSharesAccount.GetBalance() * GetPayoutPerShare());

    if (lPayoutAmount <= 0) {
        {
            LogNormal(OT_METHOD)(__func__)(
                ": Nothing to pay, "
                "since this account owns no shares. (Returning "
                "true.")
                .Flush();
        }
        return true;  // nothing to pay, since this account owns no shares.
                      // Success!
    }
    OT_ASSERT(false == GetNotaryID().empty());
    const auto& theNotaryID = GetNotaryID();
    OT_ASSERT(!GetPayoutUnitTypeId().empty());
    const auto& payoutUnitTypeId = GetPayoutUnitTypeId();
    OT_ASSERT(!GetVoucherAcctID().empty());
    const auto& theVoucherAcctID = (GetVoucherAcctID());
    const auto& theServerNym = server_.GetServerNym();
    const auto& theServerNymID = theServerNym.ID();
    const auto& RECIPIENT_ID = theSharesAccount.GetNymID();
    OT_ASSERT(!GetNymID().empty());
    const auto& theSenderNymID = (GetNymID());
    OT_ASSERT(!GetMemo()->empty());
    const auto strMemo = GetMemo();
    // Note: theSenderNymID is the originator of the Dividend Payout.
    // However, all the actual vouchers will be from "the server Nym" and
    // not from theSenderNymID. So then why is it even here? Because anytime
    // there's an error, the server will send to theSenderNymID instead of
    // RECIPIENT_ID (so the original sender can have his money back, instead of
    // just having it get lost in the ether.)
    bool bReturnValue = false;

    auto theVoucher{server_.API().Factory().Cheque(
        theNotaryID, server_.API().Factory().UnitID())};

    OT_ASSERT(false != bool(theVoucher));

    // 10 minutes ==    600 Seconds
    // 1 hour    ==     3600 Seconds
    // 1 day    ==    86400 Seconds
    // 30 days    ==  2592000 Seconds
    // 3 months ==  7776000 Seconds
    // 6 months == 15552000 Seconds

    const auto VALID_FROM = Clock::now();
    const auto VALID_TO = VALID_FROM + std::chrono::hours(24 * 30 * 6);
    // 180 days (6 months).
    // Todo hardcoding.
    TransactionNumber lNewTransactionNumber = 0;
    auto context =
        server_.API().Wallet().mutable_ClientContext(theServerNym.ID(), reason);
    bool bGotNextTransNum =
        server_.GetTransactor().issueNextTransactionNumberToNym(
            context.get(), lNewTransactionNumber);  // We save the transaction
    // number on the server Nym (normally we'd discard it) because
    // when the cheque is deposited, the server nym, as the owner of
    // the voucher account, needs to verify the transaction # on the
    // cheque (to prevent double-spending of cheques.)
    if (bGotNextTransNum) {
        const bool bIssueVoucher = theVoucher->IssueCheque(
            lPayoutAmount,          // The amount of the cheque.
            lNewTransactionNumber,  // Requiring a transaction number prevents
                                    // double-spending of cheques.
            VALID_FROM,  // The expiration date (valid from/to dates) of the
                         // cheque
            VALID_TO,  // Vouchers are automatically starting today and lasting
                       // 6 months.
            theVoucherAcctID,  // The asset account the cheque is drawn on.
            theServerNymID,    // Nym ID of the sender (in this case the server
                               // nym.)
            strMemo,  // Optional memo field. Includes item note and request
                      // memo.
            RECIPIENT_ID);

        // All account crediting / debiting happens in the caller, in
        // server::Server.
        //    (AND it happens only ONCE, to cover ALL vouchers.)
        // Then in here, the voucher either gets send to the recipient, or if
        // error, sent back home to
        // the issuer Nym. (ALL the funds are removed, then the vouchers are
        // sent one way or the other.)
        // Any returned vouchers, obviously serve to notify the dividend payer
        // of where the errors were
        // (as well as give him the opportunity to get his money back.)
        //
        bool bSent = false;
        if (bIssueVoucher) {
            // All this does is set the voucher's internal contract string to
            // "VOUCHER" instead of "CHEQUE". We also set the server itself as
            // the remitter, which is unusual for vouchers, but necessary in the
            // case of dividends.
            //
            theVoucher->SetAsVoucher(theServerNymID, theVoucherAcctID);
            theVoucher->SignContract(theServerNym, reason);
            theVoucher->SaveContract();

            // Send the voucher to the payments inbox of the recipient.
            //
            const auto strVoucher = String::Factory(*theVoucher);
            auto thePayment{server_.API().Factory().Payment(strVoucher)};

            OT_ASSERT(false != bool(thePayment));

            // calls DropMessageToNymbox
            bSent = server_.SendInstrumentToNym(
                theNotaryID,
                theServerNymID,  // sender nym
                RECIPIENT_ID,    // recipient nym
                *thePayment,
                "payDividend");    // todo: hardcoding.
            bReturnValue = bSent;  // <======= RETURN VALUE.
            if (bSent)
                m_lAmountPaidOut +=
                    lPayoutAmount;  // At the end of iterating all accounts, if
                                    // m_lAmountPaidOut is less than
            // lTotalPayoutAmount, then we return to rest
            // to the sender.
        } else {
            const auto strPayoutUnitTypeId = String::Factory(payoutUnitTypeId),
                       strRecipientNymID = String::Factory(RECIPIENT_ID);
            LogOutput(OT_METHOD)(__func__)(
                ": ERROR failed issuing "
                "voucher (to send to dividend payout recipient). WAS "
                "TRYING TO PAY ")(lPayoutAmount)(" of instrument "
                                                 "definition"
                                                 " ")(strPayoutUnitTypeId)(" to"
                                                                           " Ny"
                                                                           "m"
                                                                           " ")(
                strRecipientNymID)(".")
                .Flush();
        }
        // If we didn't send it, then we need to return the funds to where they
        // came from.
        //
        if (!bSent) {
            auto theReturnVoucher{server_.API().Factory().Cheque(
                theNotaryID, server_.API().Factory().UnitID())};

            OT_ASSERT(false != bool(theReturnVoucher));

            const bool bIssueReturnVoucher = theReturnVoucher->IssueCheque(
                lPayoutAmount,          // The amount of the cheque.
                lNewTransactionNumber,  // Requiring a transaction number
                                        // prevents double-spending of cheques.
                VALID_FROM,  // The expiration date (valid from/to dates) of the
                             // cheque
                VALID_TO,    // Vouchers are automatically starting today and
                             // lasting 6 months.
                theVoucherAcctID,  // The asset account the cheque is drawn on.
                theServerNymID,    // Nym ID of the sender (in this case the
                                   // server nym.)
                strMemo,  // Optional memo field. Includes item note and request
                          // memo.
                theSenderNymID);  // We're returning the money to its original
                                  // sender.

            if (bIssueReturnVoucher) {
                // All this does is set the voucher's internal contract string
                // to
                // "VOUCHER" instead of "CHEQUE".
                //
                theReturnVoucher->SetAsVoucher(
                    theServerNymID, theVoucherAcctID);
                theReturnVoucher->SignContract(theServerNym, reason);
                theReturnVoucher->SaveContract();

                // Return the voucher back to the payments inbox of the original
                // sender.
                //
                const auto strReturnVoucher =
                    String::Factory(*theReturnVoucher);
                auto theReturnPayment{
                    server_.API().Factory().Payment(strReturnVoucher)};

                OT_ASSERT(false != bool(theReturnPayment));

                // calls DropMessageToNymbox
                bSent = server_.SendInstrumentToNym(
                    theNotaryID,
                    theServerNymID,  // sender nym
                    theSenderNymID,  // recipient nym (original sender.)
                    *theReturnPayment,
                    "payDividend");  // todo: hardcoding.
                if (bSent)
                    m_lAmountReturned +=
                        lPayoutAmount;  // At the end of iterating all accounts,
                                        // if m_lAmountPaidOut+m_lAmountReturned
                                        // is less than lTotalPayoutAmount, then
                                        // we return the rest to the sender.
            } else {
                const auto strPayoutUnitTypeId =
                               String::Factory(payoutUnitTypeId),
                           strSenderNymID = String::Factory(theSenderNymID);
                LogOutput(OT_METHOD)(__func__)(
                    ": ERROR! Failed issuing voucher (to return "
                    "back to "
                    "the dividend payout initiator, after a "
                    "failed "
                    "payment attempt to the originally intended "
                    "recipient). WAS TRYING TO "
                    "PAY ")(lPayoutAmount)(" of instrument "
                                           "definition"
                                           " ")(strPayoutUnitTypeId)(" to "
                                                                     "Nym"
                                                                     " ")(
                    strSenderNymID)(".")
                    .Flush();
            }
        }   // if !bSent
    } else  // !bGotNextTransNum
    {
        const auto strPayoutUnitTypeId = String::Factory(payoutUnitTypeId),
                   strRecipientNymID = String::Factory(RECIPIENT_ID);
        LogOutput(OT_METHOD)(__func__)(
            ": ERROR! Failed issuing next transaction number while "
            "trying to send a voucher (while paying dividends). "
            "WAS TRYING TO PAY ")(lPayoutAmount)(" of instrument "
                                                 "definition"
                                                 " ")(
            strPayoutUnitTypeId->Get())(" to "
                                        "Nym"
                                        " ")(strRecipientNymID->Get())(".")
            .Flush();
    }

    return bReturnValue;
}

PayDividendVisitor::~PayDividendVisitor()
{

    m_pstrMemo = String::Factory();
    m_lPayoutPerShare = 0;
    m_lAmountPaidOut = 0;
    m_lAmountReturned = 0;
}
}  // namespace opentxs
