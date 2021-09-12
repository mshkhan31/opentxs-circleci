// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_PROTOBUF_VERIFYCONTACTS_HPP
#define OPENTXS_PROTOBUF_VERIFYCONTACTS_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <algorithm>
#include <cstdint>
#include <string>
#include <utility>

#include "opentxs/protobuf/Basic.hpp"
#include "opentxs/protobuf/Contact.hpp"

namespace opentxs
{
namespace proto
{
enum class ClaimType : bool {
    Indexed = true,
    Normal = false,
};
enum class VerificationType : bool {
    Indexed = true,
    Normal = false,
};

auto ContactAllowedContactData() noexcept -> const VersionMap&;
auto ContactDataAllowedContactSection() noexcept -> const VersionMap&;
auto ContactSectionAllowedItem() noexcept -> const VersionMap&;
auto VerificationAllowedSignature() noexcept -> const VersionMap&;
auto VerificationGroupAllowedIdentity() noexcept -> const VersionMap&;
auto VerificationIdentityAllowedVerification() noexcept -> const VersionMap&;
auto VerificationOfferAllowedClaim() noexcept -> const VersionMap&;
auto VerificationOfferAllowedVerification() noexcept -> const VersionMap&;
auto VerificationSetAllowedGroup() noexcept -> const VersionMap&;

auto ValidContactSectionName(
    const std::uint32_t version,
    const ContactSectionName name) -> bool;
auto ValidContactItemType(
    const ContactSectionVersion version,
    const ContactItemType itemType) -> bool;
auto ValidContactItemAttribute(
    const std::uint32_t version,
    const ContactItemAttribute attribute) -> bool;

auto TranslateSectionName(
    const std::uint32_t enumValue,
    const std::string& lang = "en") -> std::string;
auto TranslateItemType(
    const std::uint32_t enumValue,
    const std::string& lang = "en") -> std::string;
auto TranslateItemAttributes(
    const std::uint32_t enumValue,
    const std::string& lang = "en") -> std::string;
auto ReciprocalRelationship(const std::uint32_t relationship) -> std::uint32_t;
auto CheckCombination(
    const ContactSectionName section,
    const ContactItemType type,
    const std::uint32_t version = 1) -> bool;
auto RequiredVersion(
    const ContactSectionName section,
    const ContactItemType type,
    const std::uint32_t hint = 1) -> std::uint32_t;
auto NymRequiredVersion(
    const std::uint32_t contactDataVersion,
    const std::uint32_t hint) -> std::uint32_t;
auto RequiredAuthorityVersion(
    const std::uint32_t contactDataVersion,
    const std::uint32_t hint) -> std::uint32_t;
}  // namespace proto
}  // namespace opentxs
#endif  // OPENTXS_PROTOBUF_VERIFYCONTACTS_HPP
