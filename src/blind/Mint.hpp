// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <irrxml/irrXML.hpp>
#include <cstddef>
#include <cstdint>
#include <iosfwd>
#include <map>

#include "opentxs/Types.hpp"
#include "opentxs/blind/Mint.hpp"
#include "opentxs/core/Armored.hpp"
#include "opentxs/core/Contract.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/core/identifier/Server.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"

namespace opentxs
{
namespace api
{
class Core;
class Wallet;
}  // namespace api

namespace identity
{
class Nym;
}  // namespace identity

class PasswordPrompt;
class String;
}  // namespace opentxs

namespace opentxs::blind::mint::implementation
{
class Mint : virtual public opentxs::blind::Mint, virtual public Contract
{
public:
    auto AccountID() const -> OTIdentifier override { return m_CashAccountID; }
    auto Expired() const -> bool override;
    auto GetDenomination(std::int32_t nIndex) const -> std::int64_t override;
    auto GetDenominationCount() const -> std::int32_t override
    {
        return m_nDenominationCount;
    }

    auto GetExpiration() const -> Time override { return m_EXPIRATION; }
    auto GetLargestDenomination(std::int64_t lAmount) const
        -> std::int64_t override;
    auto GetPrivate(Armored& theArmor, std::int64_t lDenomination) const
        -> bool override;
    auto GetPublic(Armored& theArmor, std::int64_t lDenomination) const
        -> bool override;
    auto GetSeries() const -> std::int32_t override { return m_nSeries; }
    auto GetValidFrom() const -> Time override { return m_VALID_FROM; }
    auto GetValidTo() const -> Time override { return m_VALID_TO; }
    auto InstrumentDefinitionID() const
        -> const identifier::UnitDefinition& override
    {
        return m_InstrumentDefinitionID;
    }

    void GenerateNewMint(
        const api::Wallet& wallet,
        const std::int32_t nSeries,
        const Time VALID_FROM,
        const Time VALID_TO,
        const Time MINT_EXPIRATION,
        const identifier::UnitDefinition& theInstrumentDefinitionID,
        const identifier::Server& theNotaryID,
        const identity::Nym& theNotary,
        const std::int64_t nDenom1,
        const std::int64_t nDenom2,
        const std::int64_t nDenom3,
        const std::int64_t nDenom4,
        const std::int64_t nDenom5,
        const std::int64_t nDenom6,
        const std::int64_t nDenom7,
        const std::int64_t nDenom8,
        const std::int64_t nDenom9,
        const std::int64_t nDenom10,
        const std::size_t keySize,
        const PasswordPrompt& reason) override;
    auto LoadContract() -> bool override;
    auto LoadMint(const char* szAppend = nullptr) -> bool override;
    void Release() override;
    void Release_Mint() override;
    void ReleaseDenominations() override;
    auto SaveMint(const char* szAppend = nullptr) -> bool override;
    void SetInstrumentDefinitionID(
        const identifier::UnitDefinition& newID) override
    {
        m_InstrumentDefinitionID = newID;
    }
    void SetSavePrivateKeys(bool bDoIt = true) override
    {
        m_bSavePrivateKeys = bDoIt;
    }
    void UpdateContents(const PasswordPrompt& reason) override;
    auto VerifyContractID() const -> bool override;
    auto VerifyMint(const identity::Nym& theOperator) -> bool override;

    ~Mint() override;

protected:
    using mapOfArmor = std::map<std::int64_t, OTArmored>;

    auto ProcessXMLNode(irr::io::IrrXMLReader*& xml) -> std::int32_t override;

    void InitMint();

    mapOfArmor m_mapPrivate;
    mapOfArmor m_mapPublic;
    OTServerID m_NotaryID;
    OTNymID m_ServerNymID;
    OTUnitID m_InstrumentDefinitionID;
    std::int32_t m_nDenominationCount;
    bool m_bSavePrivateKeys;
    std::int32_t m_nSeries;
    Time m_VALID_FROM;
    Time m_VALID_TO;
    Time m_EXPIRATION;
    OTIdentifier m_CashAccountID;

    Mint(const api::Core& core);
    Mint(
        const api::Core& core,
        const String& strNotaryID,
        const String& strInstrumentDefinitionID);
    Mint(
        const api::Core& core,
        const String& strNotaryID,
        const String& strServerNymID,
        const String& strInstrumentDefinitionID);

private:
    Mint() = delete;
};
}  // namespace opentxs::blind::mint::implementation
