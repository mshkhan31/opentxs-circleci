// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"            // IWYU pragma: associated
#include "1_Internal.hpp"          // IWYU pragma: associated
#include "crypto/key/Ed25519.hpp"  // IWYU pragma: associated

#include <stdexcept>
#include <string_view>

#include "internal/crypto/key/Factory.hpp"
#include "opentxs/api/Core.hpp"
#include "opentxs/api/crypto/Symmetric.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"
#include "opentxs/core/Secret.hpp"
#include "opentxs/crypto/key/Ed25519.hpp"
#include "opentxs/crypto/key/asymmetric/Algorithm.hpp"
#include "util/Sodium.hpp"

namespace opentxs::factory
{
using ReturnType = crypto::key::implementation::Ed25519;

auto Ed25519Key(
    const api::Core& api,
    const crypto::EcdsaProvider& ecdsa,
    const proto::AsymmetricKey& input) noexcept
    -> std::unique_ptr<crypto::key::Ed25519>
{
    try {

        return std::make_unique<ReturnType>(api, ecdsa, input);
    } catch (const std::exception& e) {
        LogOutput("opentxs::factory::")(__func__)(": Failed to generate key: ")(
            e.what())
            .Flush();

        return nullptr;
    }
}

auto Ed25519Key(
    const api::Core& api,
    const crypto::EcdsaProvider& ecdsa,
    const crypto::key::asymmetric::Role input,
    const VersionNumber version,
    const opentxs::PasswordPrompt& reason) noexcept
    -> std::unique_ptr<crypto::key::Ed25519>
{
    try {

        return std::make_unique<ReturnType>(api, ecdsa, input, version, reason);
    } catch (const std::exception& e) {
        LogOutput("opentxs::factory::")(__func__)(": Failed to generate key: ")(
            e.what())
            .Flush();

        return nullptr;
    }
}

#if OT_CRYPTO_WITH_BIP32
auto Ed25519Key(
    const api::Core& api,
    const crypto::EcdsaProvider& ecdsa,
    const Secret& privateKey,
    const Secret& chainCode,
    const Data& publicKey,
    const proto::HDPath& path,
    const Bip32Fingerprint parent,
    const crypto::key::asymmetric::Role role,
    const VersionNumber version,
    const opentxs::PasswordPrompt& reason) noexcept
    -> std::unique_ptr<crypto::key::Ed25519>
{
    auto sessionKey = api.Symmetric().Key(reason);

    return std::make_unique<ReturnType>(
        api,
        ecdsa,
        privateKey,
        chainCode,
        publicKey,
        path,
        parent,
        role,
        version,
        sessionKey,
        reason);
}
#endif  // OT_CRYPTO_WITH_BIP32
}  // namespace opentxs::factory

namespace opentxs::crypto::key::implementation
{
Ed25519::Ed25519(
    const api::Core& api,
    const crypto::EcdsaProvider& ecdsa,
    const proto::AsymmetricKey& serializedKey) noexcept(false)
    : ot_super(api, ecdsa, serializedKey)
{
}

Ed25519::Ed25519(
    const api::Core& api,
    const crypto::EcdsaProvider& ecdsa,
    const crypto::key::asymmetric::Role role,
    const VersionNumber version,
    const PasswordPrompt& reason) noexcept(false)
    : ot_super(
          api,
          ecdsa,
          crypto::key::asymmetric::Algorithm::ED25519,
          role,
          version,
          reason)
{
    OT_ASSERT(blank_private() != plaintext_key_->Bytes());
}

#if OT_CRYPTO_WITH_BIP32
Ed25519::Ed25519(
    const api::Core& api,
    const crypto::EcdsaProvider& ecdsa,
    const Secret& privateKey,
    const Secret& chainCode,
    const Data& publicKey,
    const proto::HDPath& path,
    const Bip32Fingerprint parent,
    const crypto::key::asymmetric::Role role,
    const VersionNumber version,
    key::Symmetric& sessionKey,
    const PasswordPrompt& reason) noexcept(false)
    : ot_super(
          api,
          ecdsa,
          crypto::key::asymmetric::Algorithm::ED25519,
          privateKey,
          chainCode,
          publicKey,
          path,
          parent,
          role,
          version,
          sessionKey,
          reason)
{
}
#endif  // OT_CRYPTO_WITH_BIP32

Ed25519::Ed25519(const Ed25519& rhs) noexcept
    : key::Ed25519()
    , ot_super(rhs)
{
}

Ed25519::Ed25519(const Ed25519& rhs, const ReadView newPublic) noexcept
    : key::Ed25519()
    , ot_super(rhs, newPublic)
{
}

Ed25519::Ed25519(const Ed25519& rhs, OTSecret&& newSecretKey) noexcept
    : key::Ed25519()
    , ot_super(rhs, std::move(newSecretKey))
{
}

auto Ed25519::blank_private() const noexcept -> ReadView
{
    static const auto blank = space(64);

    return reader(blank);
}

auto Ed25519::TransportKey(
    Data& publicKey,
    Secret& privateKey,
    const PasswordPrompt& reason) const noexcept -> bool
{
    auto lock = Lock{lock_};

    if (false == HasPublic()) { return false; }
    if (false == has_private(lock)) { return false; }

    return sodium::ToCurveKeypair(
        private_key(lock, reason),
        PublicKey(),
        privateKey.WriteInto(Secret::Mode::Mem),
        publicKey.WriteInto());
}
}  // namespace opentxs::crypto::key::implementation
