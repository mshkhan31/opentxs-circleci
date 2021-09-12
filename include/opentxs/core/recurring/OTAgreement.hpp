// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// OTAgreement is derived from OTCronItem.  It handles re-occuring billing.

#ifndef OPENTXS_CORE_RECURRING_OTAGREEMENT_HPP
#define OPENTXS_CORE_RECURRING_OTAGREEMENT_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <irrxml/irrXML.hpp>
#include <cstdint>
#include <deque>

#include "opentxs/Types.hpp"
#include "opentxs/core/Account.hpp"
#include "opentxs/core/Contract.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/OTTransactionType.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/core/cron/OTCronItem.hpp"
#include "opentxs/core/identifier/Nym.hpp"

namespace opentxs
{
namespace api
{
class Core;
}  // namespace api

namespace identifier
{
class Server;
class UnitDefinition;
}  // namespace identifier

namespace identity
{
class Nym;
}  // namespace identity

namespace otx
{
namespace context
{
class Client;
class Server;
}  // namespace context
}  // namespace otx

class NumList;
class PasswordPrompt;
}  // namespace opentxs

namespace opentxs
{
// An Agreement occurs between TWO PEOPLE, and is for a CONSIDERATION.
// Thus, we add the RECIPIENT (already have SENDER from OTTrackable.)
//
// While other instruments are derived from OTTrackable (like OTCheque) in order
// to gain a transaction number and sender user/acct, Agreements are derived
// from
// a further subclass of trackable: OTCronItem.
//
// OTCronItems are allowed to be posted on the OTCron object, which performs
// regular
// processing on a timely basis to the items that are posted there. In this way,
// payment authorizations can be posted (and expire properly), and trades can be
// posted with valid date ranges, and payment plans can be instituted, and so
// on.
//
// OTAgreement is derived from OTCronItem because it allows people to post
// Agreements
// on OTCron until a certain expiration period, so that third parties can query
// the
// server and verify the agreements, and so that copies of the agreement,
// stamped
// with the server's signature, can be made available to the parties and to 3rd
// parties.
//
class OPENTXS_EXPORT OTAgreement : public OTCronItem
{
private:  // Private prevents erroneous use by other classes.
    using ot_super = OTCronItem;

private:
    OTIdentifier m_RECIPIENT_ACCT_ID;
    OTNymID m_RECIPIENT_NYM_ID;

protected:
    OTString m_strConsideration;  // Presumably an agreement is in return for
                                  // some consideration. Memo here.

    OTString m_strMerchantSignedCopy;  // The merchant sends it over, then the
                                       // payer confirms it, which adds
    // his own transaction numbers and signs it. This, unfortunately,
    // invalidates the merchant's version, so we store
    // a copy of the merchant's signed agreement INSIDE our own. The server can
    // do the hard work of comparing them, though
    // such will probably occur through a comparison function I'll have to add
    // right here in this class.

    void onFinalReceipt(
        OTCronItem& theOrigCronItem,
        const std::int64_t& lNewTransactionNumber,
        Nym_p theOriginator,
        Nym_p pRemover,
        const PasswordPrompt& reason) override;
    void onRemovalFromCron(const PasswordPrompt& reason) override;

    // Numbers used for CLOSING a transaction. (finalReceipt.)
    std::deque<TransactionNumber> m_dequeRecipientClosingNumbers;

public:
    auto GetOriginType() const -> originType override
    {
        return originType::origin_payment_plan;
    }

    void setCustomerNymId(const identifier::Nym& NYM_ID);

    auto GetConsideration() const -> const String&
    {
        return m_strConsideration;
    }
    void SetMerchantSignedCopy(const String& strMerchantCopy)
    {
        m_strMerchantSignedCopy = strMerchantCopy;
    }
    auto GetMerchantSignedCopy() const -> const String&
    {
        return m_strMerchantSignedCopy;
    }

    // SetAgreement replaced with the 2 functions below. See notes even lower.
    //
    //    bool    SetAgreement(const std::int64_t& lTransactionNum,    const
    // OTString& strConsideration,
    //                       const Time& VALID_FROM=0,    const Time&
    // VALID_TO=0);

    auto SetProposal(
        otx::context::Server& context,
        const Account& MERCHANT_ACCT,
        const String& strConsideration,
        const Time VALID_FROM = {},
        const Time VALID_TO = {}) -> bool;

    // Merchant Nym is passed here so we can verify the signature before
    // confirming.
    auto Confirm(
        otx::context::Server& context,
        const Account& PAYER_ACCT,
        const identifier::Nym& p_id_MERCHANT_NYM,
        const identity::Nym* pMERCHANT_NYM = nullptr) -> bool;

    // What should be the process here?

    /*
        FIRST: (Construction)

     OTAgreement(const identifier::Server& NOTARY_ID,
                 const identifier::UnitDefinition& INSTRUMENT_DEFINITION_ID);
       OR:
     OTAgreement(const identifier::Server& NOTARY_ID, const Identifier&
    INSTRUMENT_DEFINITION_ID,
                 const Identifier& SENDER_ACCT_ID, const Identifier&
    SENDER_NYM_ID,
                 const Identifier& RECIPIENT_ACCT_ID, const Identifier&
    RECIPIENT_NYM_ID);
       OR:
     OTPaymentPlan * pPlan = new OTPaymentPlan(pAccount->GetRealNotaryID(),
                                    pAccount->GetInstrumentDefinitionID(),
                                    pAccount->GetRealAccountID(),
                                    pAccount->GetNymID(),
                                    RECIPIENT_ACCT_ID, RECIPIENT_NYM_ID);
     THEN:  (Agreement)

     bool bSuccessSetAgreement = pPlan->SetAgreement(lTransactionNumber,
                                                    PLAN_CONSIDERATION,
    VALID_FROM, VALID_TO);

     THEN, (OTPaymentPlan) adds TWO OPTIONS (additional and independent of each
    other):

     bool        SetInitialPayment(const std::int64_t& lAmount, Time
                    tTimeUntilInitialPayment=0); // default: now.
     bool        SetPaymentPlan(const std::int64_t& lPaymentAmount, Time
                                tTimeUntilPlanStart=OT_TIME_MONTH_IN_SECONDS,
                                Time
                                tBetweenPayments=OT_TIME_MONTH_IN_SECONDS, //
    Default: 30 days.
                                Time tPlanLength=0, std::int32_t
    nMaxPayments=0);


     The new process is the same, but it adds some additional transaction
    numbers...

     HERE IS THE WAY I ENVISION IT BEING CALLED:

     ---- The MERCHANT does these steps: -----

     Step one, though it says PaymentPlan, is basically the OTAgreement
    constructor.
     Its primary concern is with determining the server, payer, payee, accounts,
    etc.

     1) OTPaymentPlan * pPlan =
        new OTPaymentPlan(pAccount->GetRealNotaryID(),
            pAccount->GetInstrumentDefinitionID(),
            pAccount->GetRealAccountID(),
            pAccount->GetNymID(),
            RECIPIENT_ACCT_ID, RECIPIENT_NYM_ID);

    STILL, this is the MERCHANT. Step two is concerned with the specific terms
    of the offer.

     2) bool bOffer =
            pPlan->SetProposal(MERCHANT_NYM,
                        PLAN_CONSIDERATION, VALID_FROM, VALID_TO);
      (lMerchantTransactionNumber, lMerchantClosingNumber are set internally
    using the MERCHANT_NYM.)

     ==> Optionally, the merchant also calls SetInitialPayment and/or
    SetPaymentPlan at this time.
     ==> Next, the merchant signs it, and sends to the recipient.

     THE RECIPIENT:

     3) bool bConfirmation =  pPlan->Confirm(identity::Nym& PAYER_NYM,
                                             Nym *
    pMERCHANT_NYM=nullptr,
                                             OTIdentifier *
    p_id_MERCHANT_NYM=nullptr);

     (Transaction number and closing number are retrieved from Nym at this
    time.)

     NO NEED TO SIGN ANYTHING AFTER THIS POINT, and the Payment Plan should
    store a copy of itself at this time.
    (That is, STORE A COPY of the Merchant's signed version, since the above
    call to Confirm will change the plan
     and sign it again. The server is left with the chore of comparing the two
    against each other, which I will
     probably have to code right here in this class!  TOdo.)

     */

    // This function verifies both Nyms and both signatures.
    // Due to the peculiar nature of how OTAgreement/OTPaymentPlan works, there
    // are two signed
    // copies stored. The merchant signs first, adding his transaction numbers
    // (2), and then he
    // sends it to the customer, who also adds two numbers and signs. (Also
    // resetting the creation date.)
    // The problem is, adding the additional transaction numbers invalidates the
    // first (merchant's)
    // signature.
    // The solution is, when the customer confirms the agreement, he stores an
    // internal copy of the
    // merchant's signed version.  This way later, in VERIFY AGREEMENT, the
    // internal copy can be loaded,
    // and BOTH Nyms can be checked to verify that BOTH transaction numbers are
    // valid for each.
    // The two versions of the contract can also be compared to each other, to
    // make sure that none of
    // the vital terms, values, clauses, etc are different between the two.
    //
    virtual auto VerifyAgreement(
        const otx::context::Client& recipient,
        const otx::context::Client& sender) const -> bool = 0;

    virtual auto CompareAgreement(const OTAgreement& rhs) const -> bool;

    inline auto GetRecipientAcctID() const -> const Identifier&
    {
        return m_RECIPIENT_ACCT_ID;
    }
    inline auto GetRecipientNymID() const -> const identifier::Nym&
    {
        return m_RECIPIENT_NYM_ID;
    }
    inline void SetRecipientAcctID(const Identifier& ACCT_ID)
    {
        m_RECIPIENT_ACCT_ID = ACCT_ID;
    }
    inline void SetRecipientNymID(const identifier::Nym& NYM_ID)
    {
        m_RECIPIENT_NYM_ID = NYM_ID;
    }

    // The recipient must also provide an opening and closing transaction
    // number(s).
    //
    auto GetRecipientClosingTransactionNoAt(std::uint32_t nIndex) const
        -> std::int64_t;
    auto GetRecipientCountClosingNumbers() const -> std::int32_t;

    void AddRecipientClosingTransactionNo(
        const std::int64_t& lClosingTransactionNo);

    // This is a higher-level than the above functions. It calls them.
    // Below is the abstraction, above is the implementation.

    auto GetRecipientOpeningNum() const -> TransactionNumber;
    auto GetRecipientClosingNum() const -> TransactionNumber;

    // From OTCronItem (parent class of this)
    /*
     inline void SetCronPointer(OTCron& theCron) { m_pCron = &theCron; }

     inline void SetCreationDate(const Time& CREATION_DATE) {
     m_CREATION_DATE = CREATION_DATE; }
     inline const Time& GetCreationDate() const { return m_CREATION_DATE; }

     // These are for:
     // std::deque<std::int64_t> m_dequeClosingNumbers;
     //
     // They are numbers used for CLOSING a transaction. (finalReceipt, someday
     more.)

     std::int64_t    GetClosingTransactionNoAt(std::int32_t nIndex) const;
     std::int32_t     GetCountClosingNumbers() const;

     void    AddClosingTransactionNo(const std::int64_t& lClosingTransactionNo);
     */
    auto CanRemoveItemFromCron(const otx::context::Client& context)
        -> bool override;

    void HarvestOpeningNumber(otx::context::Server& context) override;
    void HarvestClosingNumbers(otx::context::Server& context) override;

    // Return True if should stay on OTCron's list for more processing.
    // Return False if expired or otherwise should be removed.
    auto ProcessCron(const PasswordPrompt& reason)
        -> bool override;  // OTCron calls
                           // this regularly,
                           // which is my
                           // chance to
                           // expire, etc.

    // From OTTrackable (parent class of OTCronItem, parent class of this)
    /*
     inline std::int64_t GetTransactionNum() const { return m_lTransactionNum; }
     inline void SetTransactionNum(std::int64_t lTransactionNum) {
     m_lTransactionNum
     = lTransactionNum; }

     inline const Identifier&    GetSenderAcctID()        { return
     m_SENDER_ACCT_ID; }
     inline const Identifier&    GetSenderNymID()        { return
     m_SENDER_NYM_ID; }
     inline void            SetSenderAcctID(const Identifier& ACCT_ID)
     { m_SENDER_ACCT_ID = ACCT_ID; }
     inline void            SetSenderNymID(const identifier::Nym& NYM_ID)
     { m_SENDER_NYM_ID = NYM_ID; }
     */

    auto HasTransactionNum(const std::int64_t& lInput) const -> bool override;
    void GetAllTransactionNumbers(NumList& numlistOutput) const override;

    // From OTInstrument (parent class of OTTrackable, parent class of
    // OTCronItem, parent class of this)
    /*
     OTInstrument(const identifier::Server& NOTARY_ID, const Identifier&
     INSTRUMENT_DEFINITION_ID)
     : Contract()

     inline const Identifier& GetInstrumentDefinitionID() const { return
     m_InstrumentDefinitionID; }
     inline const Identifier& GetNotaryID() const { return m_NotaryID; }

     inline void SetInstrumentDefinitionID(const Identifier&
     INSTRUMENT_DEFINITION_ID)  {
     m_InstrumentDefinitionID    =
     INSTRUMENT_DEFINITION_ID; }
     inline void SetNotaryID(const identifier::Server& NOTARY_ID) { m_NotaryID =
     NOTARY_ID; }

     inline Time GetValidFrom()    const { return m_VALID_FROM; }
     inline Time GetValidTo()        const { return m_VALID_TO; }

     inline void SetValidFrom(Time TIME_FROM)    { m_VALID_FROM    =
     TIME_FROM; }
     inline void SetValidTo(Time TIME_TO)        { m_VALID_TO    = TIME_TO;
     }

     bool VerifyCurrentDate(); // Verify the current date against the VALID FROM
     / TO dates.
     */

    // From OTScriptable, we override this function. OTScriptable now does fancy
    // stuff like checking to see
    // if the Nym is an agent working on behalf of a party to the contract.
    // That's how all OTScriptable-derived
    // objects work by default.  But OTAgreement (payment plan) and OTTrade do
    // it the old way: they just check to
    // see if theNym has signed *this.
    //
    auto VerifyNymAsAgent(
        const identity::Nym& theNym,
        const identity::Nym& theSignerNym) const -> bool override;

    auto VerifyNymAsAgentForAccount(
        const identity::Nym& theNym,
        const Account& theAccount) const -> bool override;

    /*
     From Contract, I have:

     virtual bool SignContract (const identity::Nym& theNym);

     */
    auto SendNoticeToAllParties(
        const api::Core& core,
        bool bSuccessMsg,
        const identity::Nym& theServerNym,
        const identifier::Server& theNotaryID,
        const std::int64_t& lNewTransactionNumber,
        // const std::int64_t& lInReferenceTo, //
        // each party has its own opening trans #.
        const String& strReference,
        const PasswordPrompt& reason,
        OTString pstrNote = String::Factory(),
        OTString pstrAttachment = String::Factory(),
        identity::Nym* pActualNym = nullptr) const -> bool;

    // Nym receives an Item::acknowledgment or Item::rejection.
    static auto DropServerNoticeToNymbox(
        const api::Core& core,
        bool bSuccessMsg,
        const identity::Nym& theServerNym,
        const identifier::Server& NOTARY_ID,
        const identifier::Nym& NYM_ID,
        const TransactionNumber& lNewTransactionNumber,
        const TransactionNumber& lInReferenceTo,
        const String& strReference,
        originType theOriginType,
        OTString pstrNote,
        OTString pstrAttachment,
        const identifier::Nym& actualNymID,
        const PasswordPrompt& reason) -> bool;

    ~OTAgreement() override;

    void InitAgreement();

    void Release() override;
    void Release_Agreement();
    auto IsValidOpeningNumber(const std::int64_t& lOpeningNum) const
        -> bool override;
    auto GetOpeningNumber(const identifier::Nym& theNymID) const
        -> std::int64_t override;
    auto GetClosingNumber(const Identifier& theAcctID) const
        -> std::int64_t override;
    // return -1 if error, 0 if nothing, and 1 if the node was processed.
    auto ProcessXMLNode(irr::io::IrrXMLReader*& xml) -> std::int32_t override;
    void UpdateContents(const PasswordPrompt& reason)
        override;  // Before transmission or serialization,
                   // this
                   // is where the ledger saves its contents

protected:
    OTAgreement(const api::Core& core);
    OTAgreement(
        const api::Core& core,
        const identifier::Server& NOTARY_ID,
        const identifier::UnitDefinition& INSTRUMENT_DEFINITION_ID);
    OTAgreement(
        const api::Core& core,
        const identifier::Server& NOTARY_ID,
        const identifier::UnitDefinition& INSTRUMENT_DEFINITION_ID,
        const Identifier& SENDER_ACCT_ID,
        const identifier::Nym& SENDER_NYM_ID,
        const Identifier& RECIPIENT_ACCT_ID,
        const identifier::Nym& RECIPIENT_NYM_ID);

    OTAgreement() = delete;
};
}  // namespace opentxs
#endif
