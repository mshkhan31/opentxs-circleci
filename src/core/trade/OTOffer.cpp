// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                    // IWYU pragma: associated
#include "1_Internal.hpp"                  // IWYU pragma: associated
#include "opentxs/core/trade/OTOffer.hpp"  // IWYU pragma: associated

#include <chrono>
#include <cinttypes>
#include <cstdint>
#include <cstring>
#include <memory>
#include <string>

#include "opentxs/Pimpl.hpp"
#include "opentxs/api/Core.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Instrument.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/core/StringXML.hpp"
#include "opentxs/core/identifier/Server.hpp"
#include "opentxs/core/util/Common.hpp"
#include "opentxs/core/util/Tag.hpp"

// Each instance of OTOffer represents a Bid or Ask. (A Market has a list of bid
// offers and a list of ask offers.)

// Also allows for x == 1.
//

#define OT_METHOD "opentxs::OTOffer::"

namespace opentxs
{
OTOffer::OTOffer(const api::Core& core)
    : Instrument(core)
    , m_pTrade(nullptr)
    , m_CURRENCY_TYPE_ID(api_.Factory().Identifier())
    , m_bSelling(false)
    , m_lPriceLimit(0)
    , m_lTransactionNum(0)
    , m_lTotalAssetsOffer(0)
    , m_lFinishedSoFar(0)
    , m_lScale(1)
    , m_lMinimumIncrement(1)
    , m_tDateAddedToMarket()
{
    InitOffer();
}

OTOffer::OTOffer(
    const api::Core& core,
    const identifier::Server& NOTARY_ID,
    const identifier::UnitDefinition& INSTRUMENT_DEFINITION_ID,
    const identifier::UnitDefinition& CURRENCY_ID,
    const std::int64_t& lScale)
    : Instrument(core, NOTARY_ID, INSTRUMENT_DEFINITION_ID)
    , m_pTrade(nullptr)
    , m_CURRENCY_TYPE_ID(CURRENCY_ID)
    , m_bSelling(false)
    , m_lPriceLimit(0)
    , m_lTransactionNum(0)
    , m_lTotalAssetsOffer(0)
    , m_lFinishedSoFar(0)
    , m_lScale(1)
    , m_lMinimumIncrement(1)
    , m_tDateAddedToMarket()
{
    InitOffer();
    SetScale(lScale);
}

auto OTOffer::isPowerOfTen(const std::int64_t& x) -> bool
{
    if (1 == x) return true;

    const std::int64_t lBase = 10;
    std::int64_t lIt = lBase;

    for (std::int32_t i = 1; i < 23; i++) {
        if (x == lIt) return true;
        lIt *= lBase;
    }

    return false;
}

/*
 Let's say you wanted to add an Offer to a Market. But you don't know
 which market.  There are different markets for different combinations
 of asset and currency. There are also higher and lower level markets
 for different trade minimums.

 The server has to be able to match up your Offer to the Right Market,
 so that it can trade with similar offers.

 So in this method, I combine the Instrument Definition ID, the Currency Type
 ID,
 and the minimum increment, and use them to generate a UNIQUE ID, which
 will also be the same, given the same input.

 That is the ID I will use for looking up the offers on the market.
 Basically it's the Market ID, and the Offer just has the SAME ID,
 and that's how you match it up to the market.

 (This is analogous to how Transactions and Transaction Items have the
 same transaction number.)

 THIS MEANS that the user cannot simply set his minimum increment to
 a "divide-into equally" with the market minimum increment. Why not?
 Because since his number will be different from the next guy, they
 will calculate different IDs and thus end up on different markets.

 TODO: therefore the user MUST SUPPLY the EXACT SAME minimum increment
 of the market he wishes to trade on. There's no other way. However,
 I CAN allow the user to ALSO provide a second minimum, which must be
 a multiple of the first.

 TODO: Should add this same method to the Market object as well.


 To use OTOffer::GetIdentifier is simple:

 void blah (OTOffer& theOffer)
 {
    OTIdentifier MARKET_ID(theOffer); // the magic happens right here.

    // (Done.)
 }
 */
void OTOffer::GetIdentifier(Identifier& theIdentifier) const
{
    auto strTemp = String::Factory(),
         strAsset = String::Factory(GetInstrumentDefinitionID()),
         strCurrency = String::Factory(GetCurrencyID());

    std::int64_t lScale = GetScale();

    // In this way we generate a unique ID that will always be consistent
    // for the same instrument definition id, currency ID, and market scale.
    strTemp->Format(
        "ASSET TYPE:\n%s\nCURRENCY TYPE:\n%s\nMARKET SCALE:\n%" PRId64 "\n",
        strAsset->Get(),
        strCurrency->Get(),
        lScale);

    theIdentifier.CalculateDigest(strTemp->Bytes());
}
auto OTOffer::IsMarketOrder() const -> bool { return (0 == GetPriceLimit()); }

auto OTOffer::IsLimitOrder() const -> bool { return (0 != GetPriceLimit()); }

// return -1 if error, 0 if nothing, and 1 if the node was processed.
auto OTOffer::ProcessXMLNode(irr::io::IrrXMLReader*& xml) -> std::int32_t
{
    std::int32_t nReturnVal = 0;

    // Here we call the parent class first.
    // If the node is found there, or there is some error,
    // then we just return either way.  But if it comes back
    // as '0', then nothing happened, and we'll continue executing.
    //
    // -- Note you can choose not to call the parent if
    // you don't want to use any of those xml tags.
    // As I do below, in the case of OTAccount.
    // if (nReturnVal = Contract::ProcessXMLNode(xml))
    //    return nReturnVal;

    if (!strcmp("marketOffer", xml->getNodeName())) {
        m_strVersion = String::Factory(xml->getAttributeValue("version"));

        auto strIsSelling = String::Factory();
        strIsSelling = String::Factory(xml->getAttributeValue("isSelling"));
        if (strIsSelling->Compare("true"))
            m_bSelling = true;
        else
            m_bSelling = false;

        m_strContractType->Set((m_bSelling ? "ASK" : "BID"));

        const auto strNotaryID =
                       String::Factory(xml->getAttributeValue("notaryID")),
                   strInstrumentDefinitionID = String::Factory(
                       xml->getAttributeValue("instrumentDefinitionID")),
                   strCurrencyTypeID = String::Factory(
                       xml->getAttributeValue("currencyTypeID"));

        const auto NOTARY_ID = api_.Factory().ServerID(strNotaryID);
        const auto INSTRUMENT_DEFINITION_ID =
                       api_.Factory().UnitID(strInstrumentDefinitionID),
                   CURRENCY_TYPE_ID = api_.Factory().UnitID(strCurrencyTypeID);

        SetNotaryID(NOTARY_ID);
        SetInstrumentDefinitionID(INSTRUMENT_DEFINITION_ID);
        SetCurrencyID(CURRENCY_TYPE_ID);

        const auto strScale =
            String::Factory(xml->getAttributeValue("marketScale"));
        const std::int64_t lScale = strScale->Exists()
                                        ? strScale->ToLong()
                                        : 0;  // if it doesn't exist,
                                              // the 0 here causes the
                                              // below error to fire.

        if (!isPowerOfTen(lScale)) {
            LogNormal(OT_METHOD)(__func__)(
                ": Failure: marketScale *must* be "
                "1, or a power of 10. Instead I got: ")(lScale)(".")
                .Flush();
            return (-1);
        } else
            SetScale(lScale);

        const auto strPriceLimit =
            String::Factory(xml->getAttributeValue("priceLimit"));
        const std::int64_t lPriceLimit =
            strPriceLimit->Exists() ? strPriceLimit->ToLong()
                                    : 0;  // if it doesn't exist, the 0 here
                                          // causes the below error to fire.

        // NOTE: Market Orders (new) have a 0 price, so this error condition was
        // changed.
        if (!strPriceLimit->Exists())
        //      if (lPriceLimit < 1)
        {
            LogNormal(OT_METHOD)(__func__)(": Failure: priceLimit *must* be "
                                           "provided(")(lPriceLimit)(").")
                .Flush();
            return (-1);
        } else
            SetPriceLimit(lPriceLimit);

        const auto strTotal =
            String::Factory(xml->getAttributeValue("totalAssetsOnOffer"));
        const std::int64_t lTotal = strTotal->Exists()
                                        ? strTotal->ToLong()
                                        : 0;  // if it doesn't exist,
                                              // the 0 here causes the
                                              // below error to fire.
        if (lTotal < 1) {
            LogNormal(OT_METHOD)(__func__)(
                ": Failure: totalAssetsOnOffer "
                "*must* be larger than 0. Instead I got: ")(lTotal)(".")
                .Flush();
            return (-1);
        } else
            SetTotalAssetsOnOffer(lTotal);

        const auto strFinished =
            String::Factory(xml->getAttributeValue("finishedSoFar"));
        const std::int64_t lFinished =
            strFinished->Exists() ? strFinished->ToLong()
                                  : 0;  // if it doesn't exist, the 0 here
                                        // causes the below error to fire.
        if (lFinished < 0) {
            LogNormal(OT_METHOD)(__func__)(
                ": Failure: finishedSoFar *must* "
                "be 0 or larger. Instead I got: ")(lFinished)(".")
                .Flush();
            return (-1);
        } else
            SetFinishedSoFar(lFinished);

        const auto strMinInc =
            String::Factory(xml->getAttributeValue("minimumIncrement"));
        // if it doesn't exist, the 0 here causes the below error to fire.
        const std::int64_t lMinInc =
            strMinInc->Exists() ? strMinInc->ToLong() : 0;

        if ((lMinInc < 1) || (lMinInc > lTotal))  // Minimum increment cannot
        // logically be higher than the
        // total assets on offer...
        {
            LogNormal(OT_METHOD)(__func__)(
                ": Failure: minimumIncrement "
                "*must* be 1 or larger, "
                "and must also be less than the total assets on offer. "
                "Instead I got: ")(lMinInc)(".")
                .Flush();
            return (-1);
        } else
            SetMinimumIncrement(lMinInc);

        const auto strTransNum =
            String::Factory(xml->getAttributeValue("transactionNum"));
        const std::int64_t lTransNum =
            strTransNum->Exists() ? strTransNum->ToLong() : 0;

        SetTransactionNum(lTransNum);

        const auto str_valid_from =
            String::Factory(xml->getAttributeValue("validFrom"));
        const auto str_valid_to =
            String::Factory(xml->getAttributeValue("validTo"));

        const auto tValidFrom = str_valid_from->Exists()
                                    ? parseTimestamp(str_valid_from->Get())
                                    : Time{};
        const auto tValidTo = str_valid_to->Exists()
                                  ? parseTimestamp(str_valid_to->Get())
                                  : Time{};

        if ((tValidTo < tValidFrom) && (tValidTo != Time{})) {
            LogNormal(OT_METHOD)(__func__)(": Failure: validTo date (")(
                tValidFrom)(") cannot be earlier than "
                            "validFrom date (")(tValidTo)(").")
                .Flush();
            return (-1);
        }

        SetValidFrom(tValidFrom);
        SetValidTo(tValidTo);

        LogTrace(OT_METHOD)(__func__)(": Offer Transaction Number: ")(
            m_lTransactionNum)("\n Valid From: ")(tValidFrom)("\n Valid To: ")(
            tValidTo)("\n InstrumentDefinitionID: ")(
            strInstrumentDefinitionID)("\n  CurrencyTypeID: ")(
            strCurrencyTypeID)("\n NotaryID: ")(
            strNotaryID)("\n Price Limit: ")(GetPriceLimit())(
            ",  Total Assets on Offer: ")(GetTotalAssetsOnOffer())(",  ")(
            (m_bSelling ? "sold" : "bought"))(" so far: ")(GetFinishedSoFar())(
            "\n  Scale: ")(GetScale())(".   Minimum Increment: ")(
            GetMinimumIncrement())(".  This offer is a")(
            (m_bSelling ? "n ASK" : " BID"))(".")
            .Flush();

        nReturnVal = 1;
    }

    return nReturnVal;
}

void OTOffer::UpdateContents(const PasswordPrompt& reason)
{
    const auto NOTARY_ID = String::Factory(GetNotaryID()),
               INSTRUMENT_DEFINITION_ID =
                   String::Factory(GetInstrumentDefinitionID()),
               CURRENCY_TYPE_ID = String::Factory(GetCurrencyID());

    // I release this because I'm about to repopulate it.
    m_xmlUnsigned->Release();

    Tag tag("marketOffer");

    tag.add_attribute("version", m_strVersion->Get());

    tag.add_attribute("isSelling", formatBool(!IsBid()));
    tag.add_attribute("notaryID", NOTARY_ID->Get());
    tag.add_attribute(
        "instrumentDefinitionID", INSTRUMENT_DEFINITION_ID->Get());
    tag.add_attribute("currencyTypeID", CURRENCY_TYPE_ID->Get());
    tag.add_attribute("priceLimit", std::to_string(GetPriceLimit()));
    tag.add_attribute(
        "totalAssetsOnOffer", std::to_string(GetTotalAssetsOnOffer()));
    tag.add_attribute("finishedSoFar", std::to_string(GetFinishedSoFar()));
    tag.add_attribute("marketScale", std::to_string(GetScale()));
    tag.add_attribute(
        "minimumIncrement", std::to_string(GetMinimumIncrement()));
    tag.add_attribute("transactionNum", std::to_string(GetTransactionNum()));
    tag.add_attribute("validFrom", formatTimestamp(GetValidFrom()));
    tag.add_attribute("validTo", formatTimestamp(GetValidTo()));

    std::string str_result;
    tag.output(str_result);

    m_xmlUnsigned->Concatenate("%s", str_result.c_str());
}

auto OTOffer::MakeOffer(
    bool bBuyingOrSelling,            // True == SELLING, False == BUYING
    const std::int64_t& lPriceLimit,  // Per Minimum Increment... (Zero price
                                      // means
                                      // it's a market order.)
    const std::int64_t& lTotalAssetsOffer,  // Total assets available for sale
                                            // or
                                            // purchase.
    const std::int64_t& lMinimumIncrement,  // The minimum increment that must
                                            // be
    // bought or sold for each transaction
    const std::int64_t& lTransactionNum,  // The transaction number authorizing
                                          // this
                                          // trade.
    const Time VALID_FROM,                // defaults to RIGHT NOW
    const Time VALID_TO) -> bool  // defaults to 24 hours (a "Day Order")
{
    m_bSelling = bBuyingOrSelling;  // Bid or Ask?
    SetTransactionNum(lTransactionNum);
    SetTotalAssetsOnOffer(lTotalAssetsOffer);  // 500 bushels for sale.

    m_strContractType->Set((m_bSelling ? "ASK" : "BID"));

    // Make sure minimum increment isn't bigger than total Assets.
    // (If you pass them into this function as the same value, it's functionally
    // a "FILL OR KILL" order.)
    std::int64_t lRealMinInc = lMinimumIncrement;
    if (lMinimumIncrement > lTotalAssetsOffer)  // Once the total, minus finish
                                                // so far, is smaller than the
                                                // minimum increment,
        lRealMinInc = lTotalAssetsOffer;  // then the OTTrade object I am linked
                                          // to will expire and remove me from
                                          // the market.
    // OR it could set the minimum increment to the remainder. But then need to
    // calc price.

    SetMinimumIncrement(lRealMinInc);  // Must sell in 50 bushel increments.
                                       // (Perhaps on the 10-bushel market it
                                       // will sell in 5 increments of 10.)
    SetPriceLimit(lPriceLimit);        // Won't sell for any less than $10 per
    // increment. (Always get best market price.)
    SetFinishedSoFar(0);  // So far have already sold 350 bushels. Actual amount
                          // available is (total - finished).

    Time REAL_VALID_FROM = VALID_FROM;
    Time REAL_VALID_TO = VALID_TO;

    if (Time{} >= VALID_FROM) { REAL_VALID_FROM = Clock::now(); }

    if (Time{} >= VALID_TO) {
        // (All offers default to a "DAY ORDER" if valid dates not specified.)
        REAL_VALID_TO = REAL_VALID_FROM + std::chrono::hours{24};
    }

    SetValidFrom(REAL_VALID_FROM);
    SetValidTo(REAL_VALID_TO);

    return true;
}

// Note: m_tDateAddedToMarket is not saved in the Offer Contract, but OTMarket
// sets/saves/loads it.
//
auto OTOffer::GetDateAddedToMarket() const -> Time  // Used in
                                                    // OTMarket::GetOfferList
                                                    // and GetNymOfferList.
{
    return m_tDateAddedToMarket;
}

void OTOffer::SetDateAddedToMarket(const Time tDate)  // Used in OTCron when
                                                      // adding/loading offers.
{
    m_tDateAddedToMarket = tDate;
}

void OTOffer::Release_Offer()
{
    // If there were any dynamically allocated objects, clean them up here.
    m_CURRENCY_TYPE_ID->Release();
}

void OTOffer::Release()
{
    // If there were any dynamically allocated objects, clean them up here.
    Release_Offer();

    Instrument::Release();  // since I've overridden the base class, I call it
                            // now...

    // Then I call this to re-initialize everything
    InitOffer();
}

void OTOffer::InitOffer()
{
    m_strContractType->Set("OFFER");  // in practice should never appear.
                                      // BID/ASK will overwrite.

    // This pointer will get wiped anytime Release() is called... which means
    // anytime LoadContractFromString()
    // is called. For some objects, that screws them up because suddenly the
    // pointer went nullptr when they needed it.
    // In the case of this object, the pointer is reset whenever Cron processes,
    // so this is safe. But in
    // the case of other objects, it's better not to initialize the pointer
    // here, but in the constructor instead.
    // FYI. For example, OTCron has a pointer to m_pServerNym. LoadCron() and
    // the pointer is nullptr. Can't have that!
    // So I moved it to the constructor in that case.

    m_bSelling = false;
    m_lPriceLimit = 0;
    m_lTransactionNum = 0;
    m_lTotalAssetsOffer = 0;
    m_lFinishedSoFar = 0;
    m_lMinimumIncrement = 1;  // This must be 1 or greater. Enforced.
    SetScale(1);              // This must be 1 or greater. Enforced.
}

OTOffer::~OTOffer() { Release_Offer(); }
}  // namespace opentxs
