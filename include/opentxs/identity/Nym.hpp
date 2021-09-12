// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_IDENTITY_NYM_HPP
#define OPENTXS_IDENTITY_NYM_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <cstdint>
#include <memory>
#include <set>
#include <string>
#include <tuple>

#include "opentxs/Types.hpp"
#include "opentxs/contact/Types.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Secret.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/crypto/HashType.hpp"
#include "opentxs/crypto/key/Keypair.hpp"
#include "opentxs/crypto/key/asymmetric/Algorithm.hpp"
#include "opentxs/iterator/Bidirectional.hpp"

namespace google
{
namespace protobuf
{
class MessageLite;
}  // namespace protobuf
}  // namespace google

namespace opentxs
{
namespace crypto
{
namespace key
{
class Symmetric;
}  // namespace key
}  // namespace crypto

namespace identifier
{
class Nym;
class UnitDefinition;
}  // namespace identifier

namespace identity
{
class Authority;
class Source;
}  // namespace identity

namespace proto
{
class ContactData;
class Nym;
class Signature;
}  // namespace proto

class ContactData;
class NymParameters;
class PasswordPrompt;
class PaymentCode;
class Signature;
class Tag;
}  // namespace opentxs

namespace opentxs
{
namespace identity
{
class OPENTXS_EXPORT Nym
{
public:
    using KeyTypes = std::vector<crypto::key::asymmetric::Algorithm>;
    using AuthorityKeys = std::pair<OTIdentifier, KeyTypes>;
    using NymKeys = std::pair<OTNymID, std::vector<AuthorityKeys>>;
    using Serialized = proto::Nym;
    using key_type = Identifier;
    using value_type = Authority;
    using const_iterator =
        opentxs::iterator::Bidirectional<const Nym, const value_type>;

    static const VersionNumber DefaultVersion;
    static const VersionNumber MaxVersion;

    virtual auto Alias() const -> std::string = 0;
    virtual auto at(const key_type& id) const noexcept(false)
        -> const value_type& = 0;
    virtual auto at(const std::size_t& index) const noexcept(false)
        -> const value_type& = 0;
    virtual auto begin() const noexcept -> const_iterator = 0;
    virtual auto BestEmail() const -> std::string = 0;
    virtual auto BestPhoneNumber() const -> std::string = 0;
    virtual auto BestSocialMediaProfile(
        const contact::ContactItemType type) const -> std::string = 0;
    virtual auto cbegin() const noexcept -> const_iterator = 0;
    virtual auto cend() const noexcept -> const_iterator = 0;
    virtual auto Claims() const -> const opentxs::ContactData& = 0;
    virtual auto CompareID(const Nym& RHS) const -> bool = 0;
    virtual auto CompareID(const identifier::Nym& rhs) const -> bool = 0;
    virtual auto ContactCredentialVersion() const -> VersionNumber = 0;
    virtual auto ContactDataVersion() const -> VersionNumber = 0;
    virtual auto Contracts(
        const contact::ContactItemType currency,
        const bool onlyActive) const -> std::set<OTIdentifier> = 0;
    virtual auto EmailAddresses(bool active = true) const -> std::string = 0;
    virtual auto EncryptionTargets() const noexcept -> NymKeys = 0;
    virtual auto end() const noexcept -> const_iterator = 0;
    virtual void GetIdentifier(identifier::Nym& theIdentifier) const = 0;
    virtual void GetIdentifier(String& theIdentifier) const = 0;
    virtual auto GetPrivateAuthKey(
        crypto::key::asymmetric::Algorithm keytype =
            crypto::key::asymmetric::Algorithm::Null) const
        -> const crypto::key::Asymmetric& = 0;
    virtual auto GetPrivateEncrKey(
        crypto::key::asymmetric::Algorithm keytype =
            crypto::key::asymmetric::Algorithm::Null) const
        -> const crypto::key::Asymmetric& = 0;
    virtual auto GetPrivateSignKey(
        crypto::key::asymmetric::Algorithm keytype =
            crypto::key::asymmetric::Algorithm::Null) const
        -> const crypto::key::Asymmetric& = 0;

    virtual auto GetPublicAuthKey(
        crypto::key::asymmetric::Algorithm keytype =
            crypto::key::asymmetric::Algorithm::Null) const
        -> const crypto::key::Asymmetric& = 0;
    virtual auto GetPublicEncrKey(
        crypto::key::asymmetric::Algorithm keytype =
            crypto::key::asymmetric::Algorithm::Null) const
        -> const crypto::key::Asymmetric& = 0;
    // OT uses the signature's metadata to narrow down its search for the
    // correct public key.
    // 'S' (signing key) or
    // 'E' (encryption key) OR
    // 'A' (authentication key)
    virtual auto GetPublicKeysBySignature(
        crypto::key::Keypair::Keys& listOutput,
        const Signature& theSignature,
        char cKeyType = '0') const -> std::int32_t = 0;
    virtual auto GetPublicSignKey(
        crypto::key::asymmetric::Algorithm keytype =
            crypto::key::asymmetric::Algorithm::Null) const
        -> const crypto::key::Asymmetric& = 0;
    virtual auto HasCapability(const NymCapability& capability) const
        -> bool = 0;
    virtual auto HasPath() const -> bool = 0;
    virtual auto ID() const -> const identifier::Nym& = 0;

    virtual auto Name() const -> std::string = 0;

    virtual auto Path(proto::HDPath& output) const -> bool = 0;
    virtual auto PathRoot() const -> const std::string = 0;
    virtual auto PathChildSize() const -> int = 0;
    virtual auto PathChild(int index) const -> std::uint32_t = 0;
    virtual auto PaymentCode() const -> std::string = 0;
    virtual auto PaymentCodePath(AllocateOutput destination) const -> bool = 0;
    virtual auto PaymentCodePath(proto::HDPath& output) const -> bool = 0;
    virtual auto PhoneNumbers(bool active = true) const -> std::string = 0;
    virtual auto Revision() const -> std::uint64_t = 0;

    virtual auto Serialize(AllocateOutput destination) const -> bool = 0;
    OPENTXS_NO_EXPORT virtual auto Serialize(Serialized& serialized) const
        -> bool = 0;
    virtual void SerializeNymIDSource(Tag& parent) const = 0;
    OPENTXS_NO_EXPORT virtual auto Sign(
        const google::protobuf::MessageLite& input,
        const crypto::SignatureRole role,
        proto::Signature& signature,
        const PasswordPrompt& reason,
        const crypto::HashType hash = crypto::HashType::Error) const
        -> bool = 0;
    virtual auto size() const noexcept -> std::size_t = 0;
    virtual auto SocialMediaProfiles(
        const contact::ContactItemType type,
        bool active = true) const -> std::string = 0;
    virtual auto SocialMediaProfileTypes() const
        -> const std::set<contact::ContactItemType> = 0;
    virtual auto Source() const -> const identity::Source& = 0;
    virtual auto TransportKey(Data& pubkey, const PasswordPrompt& reason) const
        -> OTSecret = 0;

    virtual auto Unlock(
        const crypto::key::Asymmetric& dhKey,
        const std::uint32_t tag,
        const crypto::key::asymmetric::Algorithm type,
        const crypto::key::Symmetric& key,
        PasswordPrompt& reason) const noexcept -> bool = 0;
    OPENTXS_NO_EXPORT virtual auto Verify(
        const google::protobuf::MessageLite& input,
        proto::Signature& signature) const -> bool = 0;
    virtual auto VerifyPseudonym() const -> bool = 0;

    virtual auto AddChildKeyCredential(
        const Identifier& strMasterID,
        const NymParameters& nymParameters,
        const PasswordPrompt& reason) -> std::string = 0;
    virtual auto AddClaim(const Claim& claim, const PasswordPrompt& reason)
        -> bool = 0;
    virtual auto AddContract(
        const identifier::UnitDefinition& instrumentDefinitionID,
        const contact::ContactItemType currency,
        const PasswordPrompt& reason,
        const bool primary,
        const bool active = true) -> bool = 0;
    virtual auto AddEmail(
        const std::string& value,
        const PasswordPrompt& reason,
        const bool primary,
        const bool active) -> bool = 0;
    virtual auto AddPaymentCode(
        const opentxs::PaymentCode& code,
        const contact::ContactItemType currency,
        const PasswordPrompt& reason,
        const bool primary,
        const bool active = true) -> bool = 0;
    virtual auto AddPhoneNumber(
        const std::string& value,
        const PasswordPrompt& reason,
        const bool primary,
        const bool active) -> bool = 0;
    virtual auto AddPreferredOTServer(
        const Identifier& id,
        const PasswordPrompt& reason,
        const bool primary) -> bool = 0;
    virtual auto AddSocialMediaProfile(
        const std::string& value,
        const contact::ContactItemType type,
        const PasswordPrompt& reason,
        const bool primary,
        const bool active) -> bool = 0;
    virtual auto DeleteClaim(const Identifier& id, const PasswordPrompt& reason)
        -> bool = 0;
    virtual auto SetCommonName(
        const std::string& name,
        const PasswordPrompt& reason) -> bool = 0;
    virtual auto SetContactData(
        const ReadView protobuf,
        const PasswordPrompt& reason) -> bool = 0;
    OPENTXS_NO_EXPORT virtual auto SetContactData(
        const proto::ContactData& data,
        const PasswordPrompt& reason) -> bool = 0;
    virtual auto SetScope(
        const contact::ContactItemType type,
        const std::string& name,
        const PasswordPrompt& reason,
        const bool primary) -> bool = 0;

    virtual ~Nym() = default;

protected:
    Nym() noexcept = default;

private:
    Nym(const Nym&) = delete;
    Nym(Nym&&) = delete;
    auto operator=(const Nym&) -> Nym& = delete;
    auto operator=(Nym&&) -> Nym& = delete;
};
}  // namespace identity
}  // namespace opentxs
#endif
