// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_EXT_OTPAYMENT_HPP
#define OPENTXS_EXT_OTPAYMENT_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <irrxml/irrXML.hpp>
#include <cstdint>
#include <memory>

#include "opentxs/Types.hpp"
#include "opentxs/core/Cheque.hpp"
#include "opentxs/core/Contract.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/OTTransaction.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/core/recurring/OTPaymentPlan.hpp"
#include "opentxs/core/script/OTSmartContract.hpp"

namespace opentxs
{
namespace api
{
namespace implementation
{
class Factory;
}  // namespace implementation

class Core;
}  // namespace api

class NumList;
class OTTrackable;
class PasswordPrompt;

/*
  The PAYMENT can be of types:
    - CHEQUE, INVOICE, VOUCHER (these are all forms of cheque)
    - PAYMENT PLAN, SMART CONTRACT (these are cron items)

 FYI:

 Contract — Most other classes are derived from this one. Contains the actual
 XML contents,
  as well as various data values that were loaded from those contents, including
 public keys.
  Also contains a list of signatures.

 OTScriptable — Derived from Contract, but enables scriptable clauses. Also
 contains a list
  of parties (each with agents and asset accounts) as well as a list of bylaws
 (each with scripted
  clauses, internal state, hooks, callbacks, etc.)

 OTInstrument — Has a date range, a server ID, and an instrument definition id.
 Derived from
 OTScriptable.

 OTTrackable  — Has a transaction number, user ID, and an asset account ID.
 Derived from OTInstrument.

 OTCheque — A financial instrument. Derived from OTTrackable.

 OTCronItem — Derived from OTTrackable. OT has a central “Cron” object which
 runs recurring tasks, known as CronItems.

 OTAgreement — Derived from OTCronItem. It has a recipient and recipient asset
 account.

 OTPaymentPlan — Derived from OTAgreement, derived from OTCronItem. Allows
 merchants and customers
  to set up recurring payments. (Cancel anytime, with a receipt going to both
 inboxes.)

 OTSmartContract — Derived from OTCronItem. All CronItems are actually derived
 from OTScriptable already
  (through OTTrackable/OTInstrument). But OTSmartContract is the first/only Cron
 Item specifically designed
  to take full advantage of both the cron system AND the scriptable system in
 conjunction with each other.
  Currently OTSmartContract is the only actual server-side scripting on OT.
 */

class OPENTXS_EXPORT OTPayment : public Contract
{
public:
    enum paymentType {
        // OTCheque is derived from OTTrackable, which is derived from
        // OTInstrument, which is
        // derived from OTScriptable, which is derived from Contract.
        CHEQUE,   // A cheque drawn on a user's account.
        VOUCHER,  // A cheque drawn on a server account (cashier's cheque aka
                  // banker's cheque)
        INVOICE,  // A cheque with a negative amount. (Depositing this causes a
                  // payment out, instead of a deposit in.)
        PAYMENT_PLAN,    // An OTCronItem-derived OTPaymentPlan, related to a
                         // recurring payment plan.
        SMART_CONTRACT,  // An OTCronItem-derived OTSmartContract, related to a
                         // smart contract.
        NOTICE,  // An OTTransaction containing a notice that a cron item was
                 // activated/canceled.
        // NOTE: Even though a notice isn't a "payment instrument" it can still
        // be found
        // in the Nym's record box, where all his received payments are moved
        // once they
        // are deposited. Interestingly though, I believe those are all
        // RECEIVED, except
        // for the notices, which are SENT. (Well, the notice was actually
        // received from
        // the server, BUT IN REFERENCE TO something that had been sent, and
        // thus the outgoing
        // payment is removed when the notice is received into the record box.
        ERROR_STATE
    };  // If you add any types to this list, update the list of strings at the
    // top of the .CPP file.

    static auto _GetTypeString(paymentType theType) -> const char*;

    static auto GetTypeFromString(const String& strType) -> paymentType;

    auto GetAllTransactionNumbers(
        NumList& numlistOutput,
        const PasswordPrompt& reason) const -> bool;
    // Once you "Instantiate" the first time, then these values are set, if
    // available, and can be queried thereafter from *this. Otherwise, these
    // functions will return false.
    auto GetAmount(Amount& lOutput) const -> bool;
    // Only works for payment plans and smart contracts. Gets the opening
    // transaction number for a given Nym, if applicable. (Or closing number for
    // a given asset account.)
    auto GetClosingNum(
        TransactionNumber& lOutput,
        const Identifier& theAcctID,
        const PasswordPrompt& reason) const -> bool;
    auto GetInstrumentDefinitionID(Identifier& theOutput) const -> bool;
    auto GetMemo(String& strOutput) const -> bool;
    auto GetNotaryID(Identifier& theOutput) const -> bool;
    // Only works for payment plans and smart contracts. Gets the opening
    // transaction number for a given Nym, if applicable. (Or closing number for
    // a given asset account.)
    auto GetOpeningNum(
        TransactionNumber& lOutput,
        const identifier::Nym& theNymID,
        const PasswordPrompt& reason) const -> bool;
    auto GetPaymentContents(String& strOutput) const -> bool
    {
        strOutput.Set(m_strPayment->Get());
        return true;
    }
    auto GetRecipientAcctID(Identifier& theOutput) const -> bool;
    auto GetRecipientNymID(identifier::Nym& theOutput) const -> bool;
    auto GetRemitterAcctID(Identifier& theOutput) const -> bool;
    auto GetRemitterNymID(identifier::Nym& theOutput) const -> bool;
    auto GetSenderAcctID(Identifier& theOutput) const -> bool;
    auto GetSenderAcctIDForDisplay(Identifier& theOutput) const -> bool;
    auto GetSenderNymID(identifier::Nym& theOutput) const -> bool;
    auto GetSenderNymIDForDisplay(identifier::Nym& theOutput) const -> bool;
    auto GetTransactionNum(TransactionNumber& lOutput) const -> bool;
    auto GetTransNumDisplay(TransactionNumber& lOutput) const -> bool;
    auto GetType() const -> paymentType { return m_Type; }
    auto GetTypeString() const -> const char* { return _GetTypeString(m_Type); }
    auto GetValidFrom(Time& tOutput) const -> bool;
    auto GetValidTo(Time& tOutput) const -> bool;
    auto HasTransactionNum(
        const TransactionNumber& lInput,
        const PasswordPrompt& reason) const -> bool;
    auto Instantiate() const -> OTTrackable*;
    auto Instantiate(const String& strPayment) -> OTTrackable*;
    auto InstantiateNotice() const -> OTTransaction*;
    auto IsCheque() const -> bool { return (CHEQUE == m_Type); }
    auto IsVoucher() const -> bool { return (VOUCHER == m_Type); }
    auto IsInvoice() const -> bool { return (INVOICE == m_Type); }
    auto IsPaymentPlan() const -> bool { return (PAYMENT_PLAN == m_Type); }
    auto IsSmartContract() const -> bool { return (SMART_CONTRACT == m_Type); }
    auto IsNotice() const -> bool { return (NOTICE == m_Type); }
    auto IsValid() const -> bool { return (ERROR_STATE != m_Type); }
    auto Payment() const -> const String& { return m_strPayment; }

    auto IsCancelledCheque(const PasswordPrompt& reason) -> bool;
    // Verify whether the CURRENT date is AFTER the the "VALID TO" date.
    auto IsExpired(bool& bExpired) -> bool;
    void InitPayment();
    auto InstantiateNotice(const String& strNotice) -> OTTransaction*;
    auto ProcessXMLNode(irr::io::IrrXMLReader*& xml) -> std::int32_t override;
    void Release() override;
    void Release_Payment();
    auto SetPayment(const String& strPayment) -> bool;
    auto SetTempRecipientNymID(const identifier::Nym& id) -> bool;
    // Since the temp values are not available until at least ONE instantiating
    // has occured, this function forces that very scenario (cleanly) so you
    // don't have to instantiate-and-then-delete a payment instrument. Instead,
    // just call this, and then the temp values will be available thereafter.
    auto SetTempValues(const PasswordPrompt& reason) -> bool;
    auto SetTempValuesFromCheque(const Cheque& theInput) -> bool;
    auto SetTempValuesFromPaymentPlan(const OTPaymentPlan& theInput) -> bool;
    auto SetTempValuesFromSmartContract(const OTSmartContract& theInput)
        -> bool;
    auto SetTempValuesFromNotice(
        const OTTransaction& theInput,
        const PasswordPrompt& reason) -> bool;
    // Verify whether the CURRENT date is WITHIN the VALID FROM / TO dates.
    auto VerifyCurrentDate(bool& bVerified) -> bool;

    ~OTPayment() override;

protected:
    // Contains the cheque / payment plan / etc in string form.
    OTString m_strPayment;
    paymentType m_Type;
    // Once the actual instrument is loaded up, we copy some temp values to
    // *this object. Until then, this bool (m_bAreTempValuesSet) is set to
    // false.
    bool m_bAreTempValuesSet;

    // Here are the TEMP values: (These are not serialized.)

    // For cheques mostly, and payment plans too.
    bool m_bHasRecipient;
    // For vouchers (cashier's cheques), the Nym who bought the voucher is the
    // remitter, whereas the "sender" is the server Nym whose account the
    // voucher is drawn on.
    bool m_bHasRemitter;
    Amount m_lAmount;
    TransactionNumber m_lTransactionNum;
    TransactionNumber m_lTransNumDisplay;
    // Memo, Consideration, Subject, etc.
    OTString m_strMemo;

    // These are for convenience only, for caching once they happen to be
    // loaded. These values are NOT serialized other than via the payment
    // instrument itself (where they are captured from, whenever it is
    // instantiated.) Until m_bAreTempValuesSet is set to true, these values can
    // NOT be considered available. Use the accessing methods below. These
    // values are not ALL always available, depending on the payment instrument
    // type. Different payment instruments support different temp values.
    OTIdentifier m_InstrumentDefinitionID;
    OTIdentifier m_NotaryID;
    OTNymID m_SenderNymID;
    OTIdentifier m_SenderAcctID;
    OTNymID m_RecipientNymID;
    OTIdentifier m_RecipientAcctID;
    // A voucher (cashier's cheque) has the "bank" as the sender. Whereas the
    // Nym who actually purchased the voucher is the remitter.
    OTNymID m_RemitterNymID;
    // A voucher (cashier's cheque) has the "bank"s account as the sender acct.
    // Whereas the account that was originally used to purchase the voucher is
    // the remitter account.
    OTIdentifier m_RemitterAcctID;
    Time m_VALID_FROM;  // Temporary values. Not always available.
    Time m_VALID_TO;    // Temporary values. Not always available.

    void lowLevelSetTempValuesFromPaymentPlan(const OTPaymentPlan& theInput);
    void lowLevelSetTempValuesFromSmartContract(
        const OTSmartContract& theInput);
    // Before transmission or serialization, this is where the object saves its
    // contents
    void UpdateContents(const PasswordPrompt& reason) override;

private:
    friend api::implementation::Factory;

    using ot_super = Contract;

    OTPayment(const api::Core& api);
    OTPayment(const api::Core& api, const String& strPayment);

    OTPayment() = delete;
};
}  // namespace opentxs
#endif
