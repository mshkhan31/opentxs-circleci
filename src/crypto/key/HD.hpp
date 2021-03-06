// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <memory>
#include <string>
#include <tuple>

#include "Proto.hpp"
#include "crypto/key/EllipticCurve.hpp"
#include "opentxs/Bytes.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/core/Secret.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/crypto/key/Asymmetric.hpp"
#include "opentxs/crypto/key/HD.hpp"
#include "opentxs/crypto/key/asymmetric/Algorithm.hpp"
#include "opentxs/crypto/key/asymmetric/Role.hpp"
#include "opentxs/protobuf/Ciphertext.pb.h"
#include "opentxs/protobuf/Enums.pb.h"

namespace opentxs
{
namespace api
{
class Core;
}  // namespace api

namespace crypto
{
namespace key
{
class Symmetric;
}  // namespace key

class EcdsaProvider;
}  // namespace crypto

namespace proto
{
class AsymmetricKey;
class HDPath;
}  // namespace proto

class Data;
class PasswordPrompt;
}  // namespace opentxs

namespace opentxs::crypto::key::implementation
{
class HD : virtual public key::HD, public EllipticCurve
{
public:
    auto Chaincode(const PasswordPrompt& reason) const noexcept
        -> ReadView final;
    auto ChildKey(const Bip32Index index, const PasswordPrompt& reason)
        const noexcept -> std::unique_ptr<key::HD> final;
    auto Depth() const noexcept -> int final;
    auto Fingerprint() const noexcept -> Bip32Fingerprint final;
    auto Parent() const noexcept -> Bip32Fingerprint final { return parent_; }
    auto Path() const noexcept -> const std::string final;
    auto Path(proto::HDPath& output) const noexcept -> bool final;
    auto Xprv(const PasswordPrompt& reason) const noexcept -> std::string final;
    auto Xpub(const PasswordPrompt& reason) const noexcept -> std::string final;

protected:
    void erase_private_data(const Lock& lock) final;

    HD(const api::Core& api,
       const crypto::EcdsaProvider& ecdsa,
       const proto::AsymmetricKey& serializedKey)
    noexcept(false);
    HD(const api::Core& api,
       const crypto::EcdsaProvider& ecdsa,
       const crypto::key::asymmetric::Algorithm keyType,
       const crypto::key::asymmetric::Role role,
       const VersionNumber version,
       const PasswordPrompt& reason)
    noexcept(false);
    HD(const api::Core& api,
       const crypto::EcdsaProvider& ecdsa,
       const crypto::key::asymmetric::Algorithm keyType,
       const Secret& privateKey,
       const Data& publicKey,
       const crypto::key::asymmetric::Role role,
       const VersionNumber version,
       key::Symmetric& sessionKey,
       const PasswordPrompt& reason)
    noexcept(false);
#if OT_CRYPTO_WITH_BIP32
    HD(const api::Core& api,
       const crypto::EcdsaProvider& ecdsa,
       const crypto::key::asymmetric::Algorithm keyType,
       const Secret& privateKey,
       const Secret& chainCode,
       const Data& publicKey,
       const proto::HDPath& path,
       const Bip32Fingerprint parent,
       const crypto::key::asymmetric::Role role,
       const VersionNumber version,
       key::Symmetric& sessionKey,
       const PasswordPrompt& reason)
    noexcept(false);
    HD(const api::Core& api,
       const crypto::EcdsaProvider& ecdsa,
       const crypto::key::asymmetric::Algorithm keyType,
       const Secret& privateKey,
       const Secret& chainCode,
       const Data& publicKey,
       const proto::HDPath& path,
       const Bip32Fingerprint parent,
       const crypto::key::asymmetric::Role role,
       const VersionNumber version)
    noexcept(false);
#endif  // OT_CRYPTO_WITH_BIP32
    HD(const HD&) noexcept;
    HD(const HD& rhs, const ReadView newPublic) noexcept;
    HD(const HD& rhs, OTSecret&& newSecretKey) noexcept;

private:
    const std::shared_ptr<const proto::HDPath> path_;
    const std::unique_ptr<const proto::Ciphertext> chain_code_;
    mutable OTSecret plaintext_chain_code_;
    const Bip32Fingerprint parent_;

    auto chaincode(const Lock& lock, const PasswordPrompt& reason)
        const noexcept -> ReadView;
    auto get_chain_code(const Lock& lock, const PasswordPrompt& reason) const
        noexcept(false) -> Secret&;
    auto get_params() const noexcept
        -> std::tuple<bool, Bip32Depth, Bip32Index>;
    auto serialize(const Lock& lock, Serialized& serialized) const noexcept
        -> bool final;

    HD() = delete;
    HD(HD&&) = delete;
    auto operator=(const HD&) -> HD& = delete;
    auto operator=(HD&&) -> HD& = delete;
};
}  // namespace opentxs::crypto::key::implementation
