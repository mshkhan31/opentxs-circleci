// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstdint>
#include <memory>

#include "opentxs/Types.hpp"
#include "opentxs/crypto/Types.hpp"
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
class Ed25519;
class Keypair;
class RSA;
class Secp256k1;
class Symmetric;
}  // namespace key

class AsymmetricProvider;
class EcdsaProvider;
class SymmetricProvider;
}  // namespace crypto

namespace proto
{
class AsymmetricKey;
class HDPath;
class SymmetricKey;
}  // namespace proto

class Data;
class NymParameters;
class PasswordPrompt;
class Secret;
}  // namespace opentxs

namespace opentxs::factory
{
#if OT_CRYPTO_SUPPORTED_KEY_ED25519
auto Ed25519Key(
    const api::Core& api,
    const crypto::EcdsaProvider& ecdsa,
    const proto::AsymmetricKey& serializedKey) noexcept
    -> std::unique_ptr<crypto::key::Ed25519>;
auto Ed25519Key(
    const api::Core& api,
    const crypto::EcdsaProvider& ecdsa,
    const crypto::key::asymmetric::Role role,
    const VersionNumber version,
    const opentxs::PasswordPrompt& reason) noexcept
    -> std::unique_ptr<crypto::key::Ed25519>;
#if OT_CRYPTO_WITH_BIP32
auto Ed25519Key(
    const api::Core& api,
    const crypto::EcdsaProvider& ecdsa,
    const opentxs::Secret& privateKey,
    const opentxs::Secret& chainCode,
    const Data& publicKey,
    const proto::HDPath& path,
    const Bip32Fingerprint parent,
    const crypto::key::asymmetric::Role role,
    const VersionNumber version,
    const opentxs::PasswordPrompt& reason) noexcept
    -> std::unique_ptr<crypto::key::Ed25519>;
#endif  // OT_CRYPTO_WITH_BIP32
#endif  // OT_CRYPTO_SUPPORTED_KEY_ED25519
auto Keypair() noexcept -> std::unique_ptr<crypto::key::Keypair>;
auto Keypair(
    const api::Core& api,
    const opentxs::crypto::key::asymmetric::Role role,
    std::unique_ptr<crypto::key::Asymmetric> publicKey,
    std::unique_ptr<crypto::key::Asymmetric> privateKey) noexcept(false)
    -> std::unique_ptr<crypto::key::Keypair>;
#if OT_CRYPTO_SUPPORTED_KEY_RSA
auto RSAKey(
    const api::Core& api,
    const crypto::AsymmetricProvider& engine,
    const proto::AsymmetricKey& input) noexcept
    -> std::unique_ptr<crypto::key::RSA>;
auto RSAKey(
    const api::Core& api,
    const crypto::AsymmetricProvider& engine,
    const crypto::key::asymmetric::Role input,
    const VersionNumber version,
    const NymParameters& options,
    const opentxs::PasswordPrompt& reason) noexcept
    -> std::unique_ptr<crypto::key::RSA>;
#endif  // OT_CRYPTO_SUPPORTED_KEY_RSA
#if OT_CRYPTO_SUPPORTED_KEY_SECP256K1
auto Secp256k1Key(
    const api::Core& api,
    const crypto::EcdsaProvider& ecdsa,
    const proto::AsymmetricKey& serializedKey) noexcept
    -> std::unique_ptr<crypto::key::Secp256k1>;
auto Secp256k1Key(
    const api::Core& api,
    const crypto::EcdsaProvider& ecdsa,
    const crypto::key::asymmetric::Role role,
    const VersionNumber version,
    const opentxs::PasswordPrompt& reason) noexcept
    -> std::unique_ptr<crypto::key::Secp256k1>;
auto Secp256k1Key(
    const api::Core& api,
    const crypto::EcdsaProvider& ecdsa,
    const opentxs::Secret& privateKey,
    const Data& publicKey,
    const crypto::key::asymmetric::Role role,
    const VersionNumber version,
    const opentxs::PasswordPrompt& reason) noexcept
    -> std::unique_ptr<crypto::key::Secp256k1>;
auto Secp256k1Key(
    const api::Core& api,
    const crypto::EcdsaProvider& ecdsa,
    const opentxs::Secret& privateKey,
    const opentxs::Secret& chainCode,
    const Data& publicKey,
    const proto::HDPath& path,
    const Bip32Fingerprint parent,
    const crypto::key::asymmetric::Role role,
    const VersionNumber version,
    const opentxs::PasswordPrompt& reason) noexcept
    -> std::unique_ptr<crypto::key::Secp256k1>;
auto Secp256k1Key(
    const api::Core& api,
    const crypto::EcdsaProvider& ecdsa,
    const Secret& privateKey,
    const Secret& chainCode,
    const Data& publicKey,
    const proto::HDPath& path,
    const Bip32Fingerprint parent,
    const crypto::key::asymmetric::Role role,
    const VersionNumber version) noexcept
    -> std::unique_ptr<crypto::key::Secp256k1>;
#endif  // OT_CRYPTO_SUPPORTED_KEY_SECP256K1
auto SymmetricKey() noexcept -> std::unique_ptr<crypto::key::Symmetric>;
auto SymmetricKey(
    const api::Core& api,
    const crypto::SymmetricProvider& engine,
    const opentxs::PasswordPrompt& reason,
    const opentxs::crypto::key::symmetric::Algorithm mode) noexcept
    -> std::unique_ptr<crypto::key::Symmetric>;
auto SymmetricKey(
    const api::Core& api,
    const crypto::SymmetricProvider& engine,
    const proto::SymmetricKey serialized) noexcept
    -> std::unique_ptr<crypto::key::Symmetric>;
auto SymmetricKey(
    const api::Core& api,
    const crypto::SymmetricProvider& engine,
    const opentxs::Secret& seed,
    const std::uint64_t operations,
    const std::uint64_t difficulty,
    const std::size_t size,
    const crypto::key::symmetric::Source type) noexcept
    -> std::unique_ptr<crypto::key::Symmetric>;
auto SymmetricKey(
    const api::Core& api,
    const crypto::SymmetricProvider& engine,
    const opentxs::Secret& seed,
    const ReadView salt,
    const std::uint64_t operations,
    const std::uint64_t difficulty,
    const std::uint64_t parallel,
    const std::size_t size,
    const crypto::key::symmetric::Source type) noexcept
    -> std::unique_ptr<crypto::key::Symmetric>;
auto SymmetricKey(
    const api::Core& api,
    const crypto::SymmetricProvider& engine,
    const opentxs::Secret& raw,
    const opentxs::PasswordPrompt& reason) noexcept
    -> std::unique_ptr<crypto::key::Symmetric>;
}  // namespace opentxs::factory
