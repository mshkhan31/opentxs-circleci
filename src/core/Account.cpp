// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"              // IWYU pragma: associated
#include "1_Internal.hpp"            // IWYU pragma: associated
#include "opentxs/core/Account.hpp"  // IWYU pragma: associated

#include <cinttypes>
#include <cstdint>
#include <memory>
#include <set>
#include <string>

#include "core/OTStorage.hpp"
#include "internal/api/Api.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/api/Core.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/api/Legacy.hpp"
#include "opentxs/core/Armored.hpp"
#include "opentxs/core/Contract.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Helpers.hpp"
#include "opentxs/core/Ledger.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"
#include "opentxs/core/NymFile.hpp"
#include "opentxs/core/OTTransactionType.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/core/StringXML.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/core/identifier/Server.hpp"
#include "opentxs/core/util/Common.hpp"
#include "opentxs/core/util/Tag.hpp"
#include "opentxs/identity/Nym.hpp"
#include "opentxs/otx/consensus/Base.hpp"

using namespace irr;
using namespace io;

#define OT_METHOD "opentxs::Account::"

namespace opentxs
{

char const* const __TypeStringsAccount[] = {
    "user",       // used by users
    "issuer",     // used by issuers    (these can only go negative.)
    "basket",     // issuer acct used by basket currencies (these can only go
                  // negative)
    "basketsub",  // used by the server (to store backing reserves for basket
                  // sub-accounts)
    "mint",       // used by mints (to store backing reserves for cash)
    "voucher",    // used by the server (to store backing reserves for vouchers)
    "stash",  // used by the server (to store backing reserves for stashes, for
              // smart contracts.)
    "err_acct"};

// Used for generating accounts, thus no accountID needed.
Account::Account(
    const api::Core& core,
    const identifier::Nym& nymID,
    const identifier::Server& notaryID)
    : OTTransactionType(core)
    , acctType_(err_acct)
    , acctInstrumentDefinitionID_(api_.Factory().UnitID())
    , balanceDate_(String::Factory())
    , balanceAmount_(String::Factory())
    , stashTransNum_(0)
    , markForDeletion_(false)
    , inboxHash_(api_.Factory().Identifier())
    , outboxHash_(api_.Factory().Identifier())
    , alias_()
{
    InitAccount();
    SetNymID(nymID);
    SetRealNotaryID(notaryID);
    SetPurportedNotaryID(notaryID);
}

Account::Account(const api::Core& core)
    : OTTransactionType(core)
    , acctType_(err_acct)
    , acctInstrumentDefinitionID_(api_.Factory().UnitID())
    , balanceDate_(String::Factory())
    , balanceAmount_(String::Factory())
    , stashTransNum_(0)
    , markForDeletion_(false)
    , inboxHash_(api_.Factory().Identifier())
    , outboxHash_(api_.Factory().Identifier())
    , alias_()
{
    InitAccount();
}

Account::Account(
    const api::Core& core,
    const identifier::Nym& nymID,
    const Identifier& accountId,
    const identifier::Server& notaryID,
    const String& name)
    : OTTransactionType(core, nymID, accountId, notaryID)
    , acctType_(err_acct)
    , acctInstrumentDefinitionID_(api_.Factory().UnitID())
    , balanceDate_(String::Factory())
    , balanceAmount_(String::Factory())
    , stashTransNum_(0)
    , markForDeletion_(false)
    , inboxHash_(api_.Factory().Identifier())
    , outboxHash_(api_.Factory().Identifier())
    , alias_(name.Get())
{
    InitAccount();
    m_strName = name;
}

Account::Account(
    const api::Core& core,
    const identifier::Nym& nymID,
    const Identifier& accountId,
    const identifier::Server& notaryID)
    : OTTransactionType(core, nymID, accountId, notaryID)
    , acctType_(err_acct)
    , acctInstrumentDefinitionID_(api_.Factory().UnitID())
    , balanceDate_(String::Factory())
    , balanceAmount_(String::Factory())
    , stashTransNum_(0)
    , markForDeletion_(false)
    , inboxHash_(api_.Factory().Identifier())
    , outboxHash_(api_.Factory().Identifier())
    , alias_()
{
    InitAccount();
}

auto Account::_GetTypeString(AccountType accountType) -> char const*
{
    auto index = static_cast<std::int32_t>(accountType);
    return __TypeStringsAccount[index];
}

auto Account::Alias() const -> std::string { return alias_; }

auto Account::ConsensusHash(
    const otx::context::Base& context,
    Identifier& theOutput,
    const PasswordPrompt& reason) const -> bool
{
    auto preimage = Data::Factory();

    auto& nymid = GetNymID();
    if (false == nymid.empty()) {
        preimage->Concatenate(nymid.data(), nymid.size());
    } else {
        LogOutput(OT_METHOD)(__func__)(": Missing nym id.").Flush();
    }

    auto& serverid = context.Notary();
    if (false == serverid.empty()) {
        preimage->Concatenate(serverid.data(), serverid.size());
    } else {
        LogOutput(OT_METHOD)(__func__)(": Missing server id.").Flush();
    }

    auto accountid{api_.Factory().Identifier()};
    GetIdentifier(accountid);
    if (false == accountid->empty()) {
        preimage->Concatenate(accountid->data(), accountid->size());
    } else {
        LogOutput(OT_METHOD)(__func__)(": Missing account id.").Flush();
    }

    if (false == balanceAmount_->empty()) {
        preimage->Concatenate(
            balanceAmount_->Get(), balanceAmount_->GetLength());
    } else {
        LogOutput(OT_METHOD)(__func__)(": No account balance.").Flush();
    }

    const auto nymfile = context.Nymfile(reason);

    auto inboxhash{api_.Factory().Identifier()};
    auto loaded = nymfile->GetInboxHash(accountid->str(), inboxhash);
    if (false == loaded) {
        const_cast<Account&>(*this).GetInboxHash(inboxhash);
    }
    if (false == inboxhash->empty()) {
        preimage->Concatenate(inboxhash->data(), inboxhash->size());
    } else {
        LogOutput(OT_METHOD)(__func__)(": Empty inbox hash.").Flush();
    }

    auto outboxhash{api_.Factory().Identifier()};
    loaded = nymfile->GetOutboxHash(accountid->str(), outboxhash);
    if (false == loaded) {
        const_cast<Account&>(*this).GetOutboxHash(outboxhash);
    }
    if (false == outboxhash->empty()) {
        preimage->Concatenate(outboxhash->data(), outboxhash->size());
    } else {
        LogOutput(OT_METHOD)(__func__)(": Empty outbox hash.").Flush();
    }

    const auto& issuednumbers = context.IssuedNumbers();
    for (const auto num : issuednumbers) {
        preimage->Concatenate(&num, sizeof(num));
    }

    theOutput.Release();

    bool bCalcDigest = theOutput.CalculateDigest(preimage->Bytes());

    if (false == bCalcDigest) {
        theOutput.Release();
        LogOutput(OT_METHOD)(__func__)(
            ": Failed trying to calculate hash (for a ")(GetTypeString())(").")
            .Flush();
    }

    return bCalcDigest;
}

auto Account::create_box(
    std::unique_ptr<Ledger>& box,
    const identity::Nym& signer,
    const ledgerType type,
    const PasswordPrompt& reason) -> bool
{
    const auto& nymID = GetNymID();
    const auto& accountID = GetRealAccountID();
    const auto& serverID = GetRealNotaryID();
    box.reset(api_.Factory().Ledger(nymID, accountID, serverID).release());

    if (false == bool(box)) {
        LogOutput(OT_METHOD)(__func__)(": Failed to construct ledger.").Flush();

        return false;
    }

    const auto created =
        box->CreateLedger(nymID, accountID, serverID, type, true);

    if (false == created) {
        LogOutput(OT_METHOD)(__func__)(": Failed to generate box.").Flush();

        return false;
    }

    const auto signature = box->SignContract(signer, reason);

    if (false == signature) {
        LogOutput(OT_METHOD)(__func__)(": Failed to sign box.").Flush();

        return false;
    }

    const auto serialized = box->SaveContract();

    if (false == serialized) {
        LogOutput(OT_METHOD)(__func__)(": Failed to serialize box.").Flush();

        return false;
    }

    return true;
}

auto Account::LoadContractFromString(const String& theStr) -> bool
{
    return OTTransactionType::LoadContractFromString(theStr);
}

auto Account::LoadInbox(const identity::Nym& nym) const
    -> std::unique_ptr<Ledger>
{
    auto box{api_.Factory().Ledger(
        GetNymID(), GetRealAccountID(), GetRealNotaryID())};

    OT_ASSERT(false != bool(box));

    if (box->LoadInbox() && box->VerifyAccount(nym)) { return box; }

    auto strNymID = String::Factory(GetNymID()),
         strAcctID = String::Factory(GetRealAccountID());
    {
        LogVerbose(OT_METHOD)(__func__)("Unable to load or verify inbox: ")
            .Flush();
        LogVerbose(OT_METHOD)(__func__)(strAcctID)("  For user: ").Flush();
        LogVerbose(OT_METHOD)(__func__)(strNymID).Flush();
    }
    return nullptr;
}

auto Account::LoadOutbox(const identity::Nym& nym) const
    -> std::unique_ptr<Ledger>
{
    auto box{api_.Factory().Ledger(
        GetNymID(), GetRealAccountID(), GetRealNotaryID())};

    OT_ASSERT(false != bool(box));

    if (box->LoadOutbox() && box->VerifyAccount(nym)) { return box; }

    auto strNymID = String::Factory(GetNymID()),
         strAcctID = String::Factory(GetRealAccountID());
    {
        LogVerbose(OT_METHOD)(__func__)("Unable to load or verify outbox: ")
            .Flush();
        LogVerbose(OT_METHOD)(__func__)(strAcctID)(" For user: ").Flush();
        LogVerbose(OT_METHOD)(__func__)(strNymID).Flush();
    }
    return nullptr;
}

auto Account::save_box(
    Ledger& box,
    Identifier& hash,
    bool (Ledger::*save)(Identifier&),
    void (Account::*set)(const Identifier&)) -> bool
{
    if (!IsSameAccount(box)) {
        LogOutput(OT_METHOD)(__func__)(
            ": ERROR: The ledger passed in, "
            "isn't even for this account! Acct ID: ")(GetRealAccountID())(
            ". Other ID: ")(box.GetRealAccountID())(". Notary ID: ")(
            GetRealNotaryID())(". Other ID: ")(box.GetRealNotaryID())(".")
            .Flush();

        return false;
    }

    const bool output = (box.*save)(hash);

    if (output) { (this->*set)(hash); }

    return output;
}

auto Account::SaveInbox(Ledger& box) -> bool
{
    auto hash = api_.Factory().Identifier();

    return SaveInbox(box, hash);
}

auto Account::SaveInbox(Ledger& box, Identifier& hash) -> bool
{
    return save_box(box, hash, &Ledger::SaveInbox, &Account::SetInboxHash);
}

auto Account::SaveOutbox(Ledger& box) -> bool
{
    auto hash = api_.Factory().Identifier();

    return SaveOutbox(box, hash);
}

auto Account::SaveOutbox(Ledger& box, Identifier& hash) -> bool
{
    return save_box(box, hash, &Ledger::SaveOutbox, &Account::SetOutboxHash);
}

void Account::SetInboxHash(const Identifier& input) { inboxHash_ = input; }

auto Account::GetInboxHash(Identifier& output) -> bool
{
    output.Release();

    if (!inboxHash_->empty()) {
        output.SetString(inboxHash_->str());
        return true;
    } else if (
        !GetNymID().empty() && !GetRealAccountID().empty() &&
        !GetRealNotaryID().empty()) {
        auto inbox{api_.Factory().Ledger(
            GetNymID(), GetRealAccountID(), GetRealNotaryID())};

        OT_ASSERT(false != bool(inbox));

        if (inbox->LoadInbox() && inbox->CalculateInboxHash(output)) {
            SetInboxHash(output);
            return true;
        }
    }

    return false;
}

void Account::SetOutboxHash(const Identifier& input) { outboxHash_ = input; }

auto Account::GetOutboxHash(Identifier& output) -> bool
{
    output.Release();

    if (!outboxHash_->empty()) {
        output.SetString(outboxHash_->str());
        return true;
    } else if (
        !GetNymID().empty() && !GetRealAccountID().empty() &&
        !GetRealNotaryID().empty()) {
        auto outbox{api_.Factory().Ledger(
            GetNymID(), GetRealAccountID(), GetRealNotaryID())};

        OT_ASSERT(false != bool(outbox));

        if (outbox->LoadOutbox() && outbox->CalculateOutboxHash(output)) {
            SetOutboxHash(output);
            return true;
        }
    }

    return false;
}

auto Account::InitBoxes(
    const identity::Nym& signer,
    const PasswordPrompt& reason) -> bool
{
    LogDetail(OT_METHOD)(__func__)(": Generating inbox/outbox.").Flush();
    std::unique_ptr<Ledger> inbox{LoadInbox(signer)};
    std::unique_ptr<Ledger> outbox{LoadInbox(signer)};

    if (inbox) {
        LogOutput(OT_METHOD)(__func__)(": Inbox already exists.").Flush();

        return false;
    }

    if (false == create_box(inbox, signer, ledgerType::inbox, reason)) {
        LogOutput(OT_METHOD)(__func__)(": Failed to create inbox.").Flush();

        return false;
    }

    OT_ASSERT(inbox);

    if (false == SaveInbox(*inbox)) {
        LogOutput(OT_METHOD)(__func__)(": Failed to save inbox.").Flush();

        return false;
    }

    if (outbox) {
        LogOutput(OT_METHOD)(__func__)(": Inbox already exists.").Flush();

        return false;
    }

    if (false == create_box(outbox, signer, ledgerType::outbox, reason)) {
        LogOutput(OT_METHOD)(__func__)(": Failed to create outbox.").Flush();

        return false;
    }

    OT_ASSERT(outbox);

    if (false == SaveOutbox(*outbox)) {
        LogOutput(OT_METHOD)(__func__)(": Failed to save outbox.").Flush();

        return false;
    }

    return true;
}

// TODO:  add an override so that OTAccount, when it loads up, it performs the
// check to see the NotaryID, look at the Server Contract and make sure the
// server hashes match.
//
// TODO: override "Verify". Have some way to verify a specific Nym to a specific
// account.
//
// Overriding this so I can set the filename automatically inside based on ID.
auto Account::LoadContract() -> bool
{
    auto id = String::Factory();
    GetIdentifier(id);

    return Contract::LoadContract(
        api_.Internal().Legacy().Account(), id->Get());
}

auto Account::SaveAccount() -> bool
{
    auto id = String::Factory();
    GetIdentifier(id);
    return SaveContract(api_.Internal().Legacy().Account(), id->Get());
}

// Debit a certain amount from the account (presumably the same amount is being
// credited somewhere else)
auto Account::Debit(const Amount amount) -> bool
{
    std::int64_t oldBalance = balanceAmount_->ToLong();
    // The MINUS here is the big difference between Debit and Credit
    std::int64_t newBalance = oldBalance - amount;

    // fail if integer overflow
    if ((amount > 0 && oldBalance < INT64_MIN + amount) ||
        (amount < 0 && oldBalance > INT64_MAX + amount))
        return false;

    // This is where issuer accounts get a pass. They just go negative.
    //
    // IF the new balance is less than zero...
    // AND it's a normal account... (not an issuer)
    // AND the new balance is even less than the old balance...
    // THEN FAIL. The "new less than old" requirement is recent,
    if (newBalance < 0 && !IsAllowedToGoNegative() && newBalance < oldBalance) {
        return false;
    }
    // and it means that we now allow <0 debits on normal accounts,
    // AS LONG AS the result is a HIGHER BALANCE  :-)
    else {
        balanceAmount_->Format("%" PRId64, newBalance);
        balanceDate_->Set(String::Factory(getTimestamp()));
        return true;
    }
}

// Credit a certain amount to the account (presumably the same amount is being
// debited somewhere else)
auto Account::Credit(const Amount amount) -> bool
{
    std::int64_t oldBalance = balanceAmount_->ToLong();
    // The PLUS here is the big difference between Debit and Credit.
    std::int64_t newBalance = oldBalance + amount;

    // fail if integer overflow
    if ((amount > 0 && oldBalance > INT64_MAX - amount) ||
        (amount < 0 && oldBalance < INT64_MIN - amount))
        return false;

    // If the balance gets too big, it may flip to negative due to us using
    // std::int64_t std::int32_t.
    // We'll maybe explicitly check that it's not negative in order to prevent
    // that. TODO.
    //    if (newBalance > 0 || (OTAccount::user != acctType_))
    //    {
    //        balanceAmount_.Format("%" PRId64 "", newBalance);
    //        return true;
    //    }

    // This is where issuer accounts get a pass. They just go negative.
    // IF the new balance is less than zero...
    // AND it's a normal account... (not an issuer)
    // AND the new balance is even less than the old balance...
    // THEN FAIL. The "new less than old" requirement is recent,
    if (newBalance < 0 && !IsAllowedToGoNegative() && newBalance < oldBalance) {
        return false;
    }
    // and it means that we now allow <0 credits on normal accounts,
    // AS LONG AS the result is a HIGHER BALANCE  :-)
    else {
        balanceAmount_->Format("%" PRId64, newBalance);
        balanceDate_->Set(String::Factory(getTimestamp()));
        return true;
    }
}

auto Account::GetInstrumentDefinitionID() const
    -> const identifier::UnitDefinition&
{
    return acctInstrumentDefinitionID_;
}

void Account::InitAccount()
{
    m_strContractType = String::Factory("ACCOUNT");
    acctType_ = Account::user;
}

// Verify Contract ID first, THEN Verify Owner.
// Because we use the ID in this function, so make sure that it is verified
// before calling this.
auto Account::VerifyOwner(const identity::Nym& candidate) const -> bool
{
    auto ID_CANDIDATE = api_.Factory().NymID();
    candidate.GetIdentifier(ID_CANDIDATE);

    return m_AcctNymID == ID_CANDIDATE;
}

// TODO: when entities and roles are added, probably more will go here.
auto Account::VerifyOwnerByID(const identifier::Nym& nymId) const -> bool
{
    return nymId == m_AcctNymID;
}

auto Account::LoadExistingAccount(
    const api::Core& core,
    const Identifier& accountId,
    const identifier::Server& notaryID) -> Account*
{
    auto strDataFolder = String::Factory(core.DataFolder().c_str());
    auto strAccountPath = String::Factory("");

    if (!core.Internal().Legacy().AppendFolder(
            strAccountPath,
            strDataFolder,
            String::Factory(core.Internal().Legacy().Account()))) {
        OT_FAIL;
    }

    if (!core.Internal().Legacy().ConfirmCreateFolder(strAccountPath)) {
        LogOutput(OT_METHOD)(__func__)(
            ": Unable to find or create accounts folder: ")(
            core.Internal().Legacy().Account())(".")
            .Flush();
        return nullptr;
    }

    std::unique_ptr<Account> account{new Account{core}};

    OT_ASSERT(account);

    account->SetRealAccountID(accountId);
    account->SetRealNotaryID(notaryID);
    auto strAcctID = String::Factory(accountId);
    account->m_strFoldername =
        String::Factory(core.Internal().Legacy().Account());
    account->m_strFilename = String::Factory(strAcctID->Get());

    if (!OTDB::Exists(
            core,
            core.DataFolder(),
            account->m_strFoldername->Get(),
            account->m_strFilename->Get(),
            "",
            "")) {
        LogVerbose(OT_METHOD)(__func__)(": File does not exist: ")(
            account->m_strFoldername)(PathSeparator())(account->m_strFilename)(
            ".")
            .Flush();

        return nullptr;
    }

    if (account->LoadContract() && account->VerifyContractID()) {

        return account.release();
    }

    return nullptr;
}

auto Account::GenerateNewAccount(
    const api::Core& core,
    const identifier::Nym& nymID,
    const identifier::Server& notaryID,
    const identity::Nym& serverNym,
    const Identifier& userNymID,
    const identifier::UnitDefinition& instrumentDefinitionID,
    const PasswordPrompt& reason,
    Account::AccountType acctType,
    std::int64_t stashTransNum) -> Account*
{
    std::unique_ptr<Account> output(new Account(core, nymID, notaryID));

    if (output) {
        if (false == output->GenerateNewAccount(
                         serverNym,
                         userNymID,
                         notaryID,
                         instrumentDefinitionID,
                         reason,
                         acctType,
                         stashTransNum)) {
            output.reset();
        }
    }

    return output.release();
}

/*
 Just make sure message has these members populated:
message.m_strNymID;
message.m_strInstrumentDefinitionID;
message.m_strNotaryID;
 */
auto Account::GenerateNewAccount(
    const identity::Nym& server,
    const Identifier& userNymID,
    const identifier::Server& notaryID,
    const identifier::UnitDefinition& instrumentDefinitionID,
    const PasswordPrompt& reason,
    Account::AccountType acctType,
    std::int64_t stashTransNum) -> bool
{
    // First we generate a secure random number into a binary object...
    auto payload = Data::Factory();
    // TODO: hardcoding. Plus: is 100 bytes of random a little much here?
    if (!payload->Randomize(100)) {
        LogOutput(OT_METHOD)(__func__)(
            ": Failed trying to acquire random numbers.")
            .Flush();
        return false;
    }

    // Next we calculate that binary object into a message digest (an
    // OTIdentifier).
    auto newID = api_.Factory().Identifier();
    if (!newID->CalculateDigest(payload->Bytes())) {
        LogOutput(OT_METHOD)(__func__)(": Error generating new account ID.")
            .Flush();
        return false;
    }

    // Next we get that digest (which is a binary hash number)
    // and extract a human-readable standard string format of that hash,
    // into an OTString.
    auto strID = String::Factory(newID);

    // Set the account number based on what we just generated.
    SetRealAccountID(newID);
    // Might as well set them both. (Safe here to do so, for once.)
    SetPurportedAccountID(newID);
    // So it's not blank. The user can always change it.
    m_strName->Set(strID);

    // Next we create the full path filename for the account using the ID.
    m_strFoldername = String::Factory(api_.Internal().Legacy().Account());
    m_strFilename = String::Factory(strID->Get());

    // Then we try to load it, in order to make sure that it doesn't already
    // exist.
    if (OTDB::Exists(
            api_,
            api_.DataFolder(),
            m_strFoldername->Get(),
            m_strFilename->Get(),
            "",
            "")) {
        LogOutput(OT_METHOD)(__func__)(": Account already exists: ")(
            m_strFilename)(".")
            .Flush();
        return false;
    }

    // Set up the various important starting values of the account.
    // Account type defaults to OTAccount::user.
    // But there are also issuer accts.
    acctType_ = acctType;

    // basket, basketsub, mint, voucher, and stash
    // accounts are all "owned" by the server.
    if (IsInternalServerAcct()) {
        server.GetIdentifier(m_AcctNymID);
    } else {
        m_AcctNymID->SetString(String::Factory(userNymID));
    }

    acctInstrumentDefinitionID_->SetString(
        String::Factory(instrumentDefinitionID));

    LogDebug(OT_METHOD)(__func__)(": Creating new account, type: ")(
        instrumentDefinitionID)(".")
        .Flush();

    SetRealNotaryID(notaryID);
    SetPurportedNotaryID(notaryID);

    balanceDate_->Set(String::Factory(getTimestamp()));
    balanceAmount_->Set("0");

    if (IsStashAcct()) {
        OT_ASSERT_MSG(
            stashTransNum > 0,
            "You created a stash account, but "
            "with a zero-or-negative transaction "
            "number for its cron item.");
        stashTransNum_ = stashTransNum;
    }

    // Sign the Account (so we know that we did)... Otherwise someone could put
    // a fake
    // account file on the server if the code wasn't designed to verify the
    // signature on the
    // account.
    SignContract(server, reason);
    SaveContract();

    // Save the Account to storage (based on its ID.)
    SaveAccount();

    // Don't know why I had this here. Putting SaveAccount() instead.
    //    OTString strFilename(m_strFilename);
    //    SaveContract(strFilename.Get()); // Saves the account to a specific
    // filename

    // No need to create the inbox and outbox ledgers...they will be created
    // automatically if they do not exist when they are needed.

    return true;
}

auto Account::GetBalance() const -> std::int64_t
{
    if (balanceAmount_->Exists()) { return balanceAmount_->ToLong(); }
    return 0;
}

auto Account::DisplayStatistics(String& contents) const -> bool
{
    auto strAccountID = String::Factory(GetPurportedAccountID());
    auto strNotaryID = String::Factory(GetPurportedNotaryID());
    auto strNymID = String::Factory(GetNymID());
    auto strInstrumentDefinitionID =
        String::Factory(acctInstrumentDefinitionID_);

    auto acctType = String::Factory();
    TranslateAccountTypeToString(acctType_, acctType);

    contents.Concatenate(
        " Asset Account (%s) Name: %s\n"
        " Last retrieved Balance: %s  on date: %s\n"
        " accountID: %s\n"
        " nymID: %s\n"
        " notaryID: %s\n"
        " instrumentDefinitionID: %s\n"
        "\n",
        acctType->Get(),
        m_strName->Get(),
        balanceAmount_->Get(),
        balanceDate_->Get(),
        strAccountID->Get(),
        strNymID->Get(),
        strNotaryID->Get(),
        strInstrumentDefinitionID->Get());

    return true;
}

auto Account::SaveContractWallet(Tag& parent) const -> bool
{
    auto strAccountID = String::Factory(GetPurportedAccountID());
    auto strNotaryID = String::Factory(GetPurportedNotaryID());
    auto strNymID = String::Factory(GetNymID());
    auto strInstrumentDefinitionID =
        String::Factory(acctInstrumentDefinitionID_);

    auto acctType = String::Factory();
    TranslateAccountTypeToString(acctType_, acctType);

    // Name is in the clear in memory,
    // and base64 in storage.
    auto ascName = Armored::Factory();
    if (m_strName->Exists()) {
        ascName->SetString(m_strName, false);  // linebreaks == false
    }

    TagPtr pTag(new Tag("account"));

    pTag->add_attribute("name", m_strName->Exists() ? ascName->Get() : "");
    pTag->add_attribute("accountID", strAccountID->Get());
    pTag->add_attribute("nymID", strNymID->Get());
    pTag->add_attribute("notaryID", strNotaryID->Get());

    // These are here for informational purposes only,
    // and are not ever actually loaded back up. In the
    // previous version of this code, they were written
    // only as XML comments.
    pTag->add_attribute("infoLastKnownBalance", balanceAmount_->Get());
    pTag->add_attribute("infoDateOfLastBalance", balanceDate_->Get());
    pTag->add_attribute("infoAccountType", acctType->Get());
    pTag->add_attribute(
        "infoInstrumentDefinitionID", strInstrumentDefinitionID->Get());

    parent.add_tag(pTag);

    return true;
}

// Most contracts do not override this function...
// But OTAccount does, because IF THE SIGNER has chosen to SIGN the account
// based on the current balances, then we need to update the m_xmlUnsigned
// member with the current balances and other updated information before the
// signing occurs. (Presumably this is the whole reason why the account is
// being re-signed.)
//
// Normally, in other Contract and derived classes, m_xmlUnsigned is read
// from the file and then kept read-only, since contracts do not normally
// change. But as accounts change in balance, they must be re-signed to keep the
// signatures valid.
void Account::UpdateContents(const PasswordPrompt& reason)
{
    auto strAssetTYPEID = String::Factory(acctInstrumentDefinitionID_);
    auto ACCOUNT_ID = String::Factory(GetPurportedAccountID());
    auto NOTARY_ID = String::Factory(GetPurportedNotaryID());
    auto NYM_ID = String::Factory(GetNymID());

    auto acctType = String::Factory();
    TranslateAccountTypeToString(acctType_, acctType);

    // I release this because I'm about to repopulate it.
    m_xmlUnsigned->Release();

    Tag tag("account");

    tag.add_attribute("version", m_strVersion->Get());
    tag.add_attribute("type", acctType->Get());
    tag.add_attribute("accountID", ACCOUNT_ID->Get());
    tag.add_attribute("nymID", NYM_ID->Get());
    tag.add_attribute("notaryID", NOTARY_ID->Get());
    tag.add_attribute("instrumentDefinitionID", strAssetTYPEID->Get());

    if (IsStashAcct()) {
        TagPtr tagStash(new Tag("stashinfo"));
        tagStash->add_attribute("cronItemNum", std::to_string(stashTransNum_));
        tag.add_tag(tagStash);
    }
    if (!inboxHash_->empty()) {
        auto strHash = String::Factory(inboxHash_);
        TagPtr tagBox(new Tag("inboxHash"));
        tagBox->add_attribute("value", strHash->Get());
        tag.add_tag(tagBox);
    }
    if (!outboxHash_->empty()) {
        auto strHash = String::Factory(outboxHash_);
        TagPtr tagBox(new Tag("outboxHash"));
        tagBox->add_attribute("value", strHash->Get());
        tag.add_tag(tagBox);
    }

    TagPtr tagBalance(new Tag("balance"));

    tagBalance->add_attribute("date", balanceDate_->Get());
    tagBalance->add_attribute("amount", balanceAmount_->Get());

    tag.add_tag(tagBalance);

    if (markForDeletion_) {
        tag.add_tag(
            "MARKED_FOR_DELETION",
            "THIS ACCOUNT HAS BEEN MARKED FOR DELETION AT ITS OWN REQUEST");
    }

    std::string str_result;
    tag.output(str_result);

    m_xmlUnsigned->Concatenate("%s", str_result.c_str());
}

// return -1 if error, 0 if nothing, and 1 if the node was processed.
auto Account::ProcessXMLNode(IrrXMLReader*& xml) -> std::int32_t
{
    std::int32_t retval = 0;

    auto strNodeName = String::Factory(xml->getNodeName());

    // Here we call the parent class first.
    // If the node is found there, or there is some error,
    // then we just return either way.  But if it comes back
    // as '0', then nothing happened, and we'll continue executing.
    //
    // -- Note you can choose not to call the parent if
    // you don't want to use any of those xml tags.
    // As I do below, in the case of OTAccount.
    // if (retval = OTTransactionType::ProcessXMLNode(xml))
    //    return retval;

    if (strNodeName->Compare("account")) {
        auto acctType = String::Factory();

        m_strVersion = String::Factory(xml->getAttributeValue("version"));
        acctType = String::Factory(xml->getAttributeValue("type"));

        if (!acctType->Exists()) {
            LogOutput(OT_METHOD)(__func__)(": Failed: Empty account "
                                           "'type' attribute.")
                .Flush();
            return -1;
        }

        acctType_ = TranslateAccountTypeStringToEnum(acctType);

        if (Account::err_acct == acctType_) {
            LogOutput(OT_METHOD)(__func__)(": Failed: account 'type' "
                                           "attribute contains unknown value.")
                .Flush();
            return -1;
        }

        auto strAcctAssetType =
            String::Factory(xml->getAttributeValue("instrumentDefinitionID"));

        if (strAcctAssetType->Exists()) {
            acctInstrumentDefinitionID_->SetString(strAcctAssetType);
        } else {
            LogOutput(OT_METHOD)(__func__)(": Failed: missing "
                                           "instrumentDefinitionID.")
                .Flush();
            return -1;
        }
        auto strAccountID =
            String::Factory(xml->getAttributeValue("accountID"));
        auto strNotaryID = String::Factory(xml->getAttributeValue("notaryID"));
        auto strAcctNymID = String::Factory(xml->getAttributeValue("nymID"));

        auto ACCOUNT_ID = api_.Factory().Identifier(strAccountID);
        auto NOTARY_ID = api_.Factory().ServerID(strNotaryID);
        auto NYM_ID = api_.Factory().NymID(strAcctNymID);

        SetPurportedAccountID(ACCOUNT_ID);
        SetPurportedNotaryID(NOTARY_ID);
        SetNymID(NYM_ID);

        auto strInstrumentDefinitionID =
            String::Factory(acctInstrumentDefinitionID_);
        LogDebug(OT_METHOD)(__func__)("Account Type: ")(acctType).Flush();
        LogDebug(OT_METHOD)(__func__)("AccountID: ")(strAccountID).Flush();
        LogDebug(OT_METHOD)(__func__)("NymID: ")(strAcctNymID).Flush();
        LogDebug(OT_METHOD)(__func__)("Unit Type ID: ")(
            strInstrumentDefinitionID)
            .Flush();
        LogDebug(OT_METHOD)(__func__)("NotaryID: ")(strNotaryID).Flush();

        retval = 1;
    } else if (strNodeName->Compare("inboxHash")) {

        auto strHash = String::Factory(xml->getAttributeValue("value"));
        if (strHash->Exists()) { inboxHash_->SetString(strHash); }
        LogDebug(OT_METHOD)(__func__)("Account inboxHash: ")(strHash).Flush();
        retval = 1;
    } else if (strNodeName->Compare("outboxHash")) {

        auto strHash = String::Factory(xml->getAttributeValue("value"));
        if (strHash->Exists()) { outboxHash_->SetString(strHash); }
        LogDebug(OT_METHOD)(__func__)("Account outboxHash: ")(strHash).Flush();

        retval = 1;
    } else if (strNodeName->Compare("MARKED_FOR_DELETION")) {
        markForDeletion_ = true;
        LogDebug(OT_METHOD)(__func__)(
            "This asset account has been MARKED_FOR_DELETION at some point"
            "prior. ")
            .Flush();

        retval = 1;
    } else if (strNodeName->Compare("balance")) {
        balanceDate_ = String::Factory(xml->getAttributeValue("date"));
        balanceAmount_ = String::Factory(xml->getAttributeValue("amount"));

        // I convert to integer / std::int64_t and back to string.
        // (Just an easy way to keep the data clean.)

        const auto date = parseTimestamp((balanceDate_->Get()));
        const Amount amount = balanceAmount_->ToLong();

        balanceDate_->Set(String::Factory(formatTimestamp(date)));
        balanceAmount_->Format("%" PRId64, amount);

        LogDebug(OT_METHOD)(__func__)("BALANCE  -- ")(balanceAmount_).Flush();
        LogDebug(OT_METHOD)(__func__)("DATE     --")(balanceDate_).Flush();

        retval = 1;
    } else if (strNodeName->Compare("stashinfo")) {
        if (!IsStashAcct()) {
            LogOutput(OT_METHOD)(__func__)(
                ": Error: Encountered stashinfo "
                "tag while loading NON-STASH account.")
                .Flush();
            return -1;
        }

        std::int64_t lTransNum = 0;
        auto strStashTransNum =
            String::Factory(xml->getAttributeValue("cronItemNum"));
        if (!strStashTransNum->Exists() ||
            ((lTransNum = strStashTransNum->ToLong()) <= 0)) {
            stashTransNum_ = 0;
            LogOutput(OT_METHOD)(__func__)(
                ": Error: Bad transaction number "
                "for supposed corresponding cron item: ")(lTransNum)(".")
                .Flush();
            return -1;
        } else {
            stashTransNum_ = lTransNum;
        }

        LogDebug(OT_METHOD)(__func__)("STASH INFO:   CronItemNum     --")(
            stashTransNum_)
            .Flush();

        retval = 1;
    }

    return retval;
}

auto Account::IsInternalServerAcct() const -> bool
{
    switch (acctType_) {
        case Account::user:
        case Account::issuer:
            return false;
        case Account::basket:
        case Account::basketsub:
        case Account::mint:
        case Account::voucher:
        case Account::stash:
            return true;
        default:
            LogOutput(OT_METHOD)(__func__)(": Unknown account type.").Flush();
            return false;
    }
}

auto Account::IsOwnedByUser() const -> bool
{
    switch (acctType_) {
        case Account::user:
        case Account::issuer:
            return true;
        case Account::basket:
        case Account::basketsub:
        case Account::mint:
        case Account::voucher:
        case Account::stash:
            return false;
        default:
            LogOutput(OT_METHOD)(__func__)(": Unknown account type.").Flush();
            return false;
    }
}

auto Account::IsOwnedByEntity() const -> bool { return false; }

auto Account::IsIssuer() const -> bool { return Account::issuer == acctType_; }

auto Account::IsAllowedToGoNegative() const -> bool
{
    switch (acctType_) {
        // issuer acct controlled by a user
        case Account::issuer:
        // basket issuer acct controlled by the server (for a basket currency)
        case Account::basket:
            return true;
        // user asset acct
        case Account::user:
        // internal server acct for storing reserves for basket sub currencies
        case Account::basketsub:
        // internal server acct for storing reserves for cash withdrawals
        case Account::mint:
        // internal server acct for storing reserves for
        // vouchers (like cashier's cheques)
        case Account::voucher:
        // internal server acct for storing reserves for
        // smart contract stashes. (Money stashed IN the contract.)
        case Account::stash:
            return false;
        default:
            LogOutput(OT_METHOD)(__func__)(": Unknown account type.").Flush();
            return false;
    }
}

void Account::Release_Account()
{
    balanceDate_->Release();
    balanceAmount_->Release();
    inboxHash_->Release();
    outboxHash_->Release();
}

void Account::Release()
{
    Release_Account();
    OTTransactionType::Release();
}

void Account::SetAlias(const std::string& alias) { alias_ = alias; }

Account::~Account() { Release_Account(); }
}  // namespace opentxs
