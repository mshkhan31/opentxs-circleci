// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CORE_NYMFILE_HPP
#define OPENTXS_CORE_NYMFILE_HPP

namespace opentxs
{
namespace identifier
{
class Nym;
}  // namespace identifier

class OTPayment;
class PasswordPrompt;
}  // namespace opentxs

namespace opentxs
{
class OPENTXS_EXPORT NymFile
{
public:
    virtual auto CompareID(const identifier::Nym& theIdentifier) const
        -> bool = 0;
    virtual void DisplayStatistics(String& strOutput) const = 0;
    virtual auto GetInboxHash(
        const std::string& acct_id,
        Identifier& theOutput) const -> bool = 0;  // client-side
    virtual auto GetOutboxHash(
        const std::string& acct_id,
        Identifier& theOutput) const -> bool = 0;  // client-side
    virtual auto GetOutpaymentsByIndex(const std::int32_t nIndex) const
        -> std::shared_ptr<Message> = 0;
    virtual auto GetOutpaymentsByTransNum(
        const std::int64_t lTransNum,
        const PasswordPrompt& reason,
        std::unique_ptr<OTPayment>* pReturnPayment = nullptr,
        std::int32_t* pnReturnIndex = nullptr) const
        -> std::shared_ptr<Message> = 0;
    virtual auto GetOutpaymentsCount() const -> std::int32_t = 0;
    virtual auto GetUsageCredits() const -> const std::int64_t& = 0;
    virtual auto ID() const -> const identifier::Nym& = 0;
    virtual auto PaymentCode() const -> std::string = 0;
    virtual auto SerializeNymFile(String& output) const -> bool = 0;

    // Whenever a Nym sends a payment, a copy is dropped std::into his
    // Outpayments. (Payments screen.) A payments message is the original
    // OTMessage that this Nym sent.
    virtual void AddOutpayments(std::shared_ptr<Message> theMessage) = 0;
    // IMPORTANT NOTE: Not all outpayments have a transaction num!
    // Imagine if you sent a cash purse to someone, for example.
    // The cash withdrawal had a transNum, and the eventual cash
    // deposit will have a transNum, but the purse itself does NOT.
    // That's okay in your outpayments box since it's like an outmail
    // box. It's not a ledger, so the items inside don't need a txn#.
    virtual auto GetSetAssetAccounts() -> std::set<std::string>& = 0;
    virtual auto RemoveOutpaymentsByIndex(const std::int32_t nIndex)
        -> bool = 0;
    virtual auto RemoveOutpaymentsByTransNum(
        const std::int64_t lTransNum,
        const PasswordPrompt& reason) -> bool = 0;
    virtual auto SetInboxHash(
        const std::string& acct_id,
        const Identifier& theInput) -> bool = 0;  // client-side
    virtual auto SetOutboxHash(
        const std::string& acct_id,
        const Identifier& theInput) -> bool = 0;  // client-side
    virtual void SetUsageCredits(const std::int64_t& lUsage) = 0;

    virtual ~NymFile() = default;

protected:
    NymFile() = default;

private:
    NymFile(const NymFile&) = delete;
    NymFile(NymFile&&) = delete;
    auto operator=(const NymFile&) -> NymFile& = delete;
    auto operator=(NymFile&&) -> NymFile& = delete;
};
}  // namespace opentxs
#endif
