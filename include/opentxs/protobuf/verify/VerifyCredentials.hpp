// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_PROTOBUF_VERIFYCREDENTIALS_HPP
#define OPENTXS_PROTOBUF_VERIFYCREDENTIALS_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include "opentxs/protobuf/Basic.hpp"

namespace opentxs
{
namespace proto
{
auto AsymmetricKeyAllowedCiphertext() noexcept -> const VersionMap&;
auto AsymmetricKeyAllowedHDPath() noexcept -> const VersionMap&;
auto AuthorityAllowedCredential() noexcept -> const VersionMap&;
auto CiphertextAllowedSymmetricKey() noexcept -> const VersionMap&;
auto CredentialAllowedChildParams() noexcept -> const VersionMap&;
auto CredentialAllowedContactData() noexcept -> const VersionMap&;
auto CredentialAllowedKeyCredential() noexcept -> const VersionMap&;
auto CredentialAllowedMasterParams() noexcept -> const VersionMap&;
auto CredentialAllowedSignatures() noexcept -> const VersionMap&;
auto CredentialAllowedVerification() noexcept -> const VersionMap&;
auto EnvelopeAllowedAsymmetricKey() noexcept -> const VersionMap&;
auto EnvelopeAllowedCiphertext() noexcept -> const VersionMap&;
auto EnvelopeAllowedTaggedKey() noexcept -> const VersionMap&;
auto KeyCredentialAllowedAsymmetricKey() noexcept -> const VersionMap&;
auto MasterParamsAllowedNymIDSource() noexcept -> const VersionMap&;
auto MasterParamsAllowedSourceProof() noexcept -> const VersionMap&;
auto NymAllowedAuthority() noexcept -> const VersionMap&;
auto NymAllowedNymIDSource() noexcept -> const VersionMap&;
auto NymIDSourceAllowedAsymmetricKey() noexcept -> const VersionMap&;
auto NymIDSourceAllowedPaymentCode() noexcept -> const VersionMap&;
auto SeedAllowedCiphertext() noexcept -> const VersionMap&;
auto SymmetricKeyAllowedCiphertext() noexcept -> const VersionMap&;
auto TaggedKeyAllowedSymmetricKey() noexcept -> const VersionMap&;
}  // namespace proto
}  // namespace opentxs
#endif  // OPENTXS_PROTOBUF_VERIFYCREDENTIALS_HPP
