// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                  // IWYU pragma: associated
#include "1_Internal.hpp"                // IWYU pragma: associated
#include "crypto/key/EllipticCurve.hpp"  // IWYU pragma: associated

#include <stdexcept>
#include <utility>

#include "crypto/key/Asymmetric.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/Core.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"
#include "opentxs/core/Secret.hpp"
#include "opentxs/crypto/key/EllipticCurve.hpp"
#include "opentxs/crypto/library/EcdsaProvider.hpp"
#include "opentxs/protobuf/AsymmetricKey.pb.h"
#include "opentxs/protobuf/Ciphertext.pb.h"
#include "opentxs/protobuf/Enums.pb.h"

#define OT_METHOD "opentxs::crypto::key::implementation::EllipticCurve::"

namespace opentxs::crypto::key
{
const VersionNumber EllipticCurve::DefaultVersion{2};
const VersionNumber EllipticCurve::MaxVersion{2};
}  // namespace opentxs::crypto::key

namespace opentxs::crypto::key::implementation
{
EllipticCurve::EllipticCurve(
    const api::Core& api,
    const crypto::EcdsaProvider& ecdsa,
    const proto::AsymmetricKey& serialized) noexcept(false)
    : Asymmetric(
          api,
          ecdsa,
          serialized,
          [&](auto& pubkey, auto&) -> EncryptedKey {
              return extract_key(api, ecdsa, serialized, pubkey);
          })
    , ecdsa_(ecdsa)
{
}

EllipticCurve::EllipticCurve(
    const api::Core& api,
    const crypto::EcdsaProvider& ecdsa,
    const crypto::key::asymmetric::Algorithm keyType,
    const crypto::key::asymmetric::Role role,
    const VersionNumber version,
    const PasswordPrompt& reason) noexcept(false)
    : Asymmetric(
          api,
          ecdsa,
          keyType,
          role,
          version,
          [&](auto& pub, auto& prv) -> EncryptedKey {
              return create_key(
                  api,
                  ecdsa,
                  {},
                  role,
                  pub.WriteInto(),
                  prv.WriteInto(Secret::Mode::Mem),
                  prv,
                  {},
                  reason);
          })
    , ecdsa_(ecdsa)
{
    if (false == bool(encrypted_key_)) {
        throw std::runtime_error("Failed to instantiate encrypted_key_");
    }

    OT_ASSERT(0 < plaintext_key_->size());
}

EllipticCurve::EllipticCurve(
    const api::Core& api,
    const crypto::EcdsaProvider& ecdsa,
    const crypto::key::asymmetric::Algorithm keyType,
    const Secret& privateKey,
    const Data& publicKey,
    const crypto::key::asymmetric::Role role,
    const VersionNumber version,
    key::Symmetric& sessionKey,
    const PasswordPrompt& reason) noexcept(false)
    : Asymmetric(
          api,
          ecdsa,
          keyType,
          role,
          true,
          (false == privateKey.empty()),
          version,
          OTData{publicKey},
          [&](auto&, auto&) -> EncryptedKey {
              return encrypt_key(sessionKey, reason, true, privateKey.Bytes());
          })
    , ecdsa_(ecdsa)
{
    auto lock = Lock{lock_};

    if (has_private(lock) && !encrypted_key_) {
        throw std::runtime_error("Failed to instantiate encrypted_key_");
    }
}

EllipticCurve::EllipticCurve(
    const api::Core& api,
    const crypto::EcdsaProvider& ecdsa,
    const crypto::key::asymmetric::Algorithm keyType,
    const Secret& privateKey,
    const Data& publicKey,
    const crypto::key::asymmetric::Role role,
    const VersionNumber version) noexcept(false)
    : Asymmetric(
          api,
          ecdsa,
          keyType,
          role,
          true,
          (false == privateKey.empty()),
          version,
          OTData{publicKey},
          {})
    , ecdsa_(ecdsa)
{
}

EllipticCurve::EllipticCurve(const EllipticCurve& rhs) noexcept
    : Asymmetric(rhs)
    , ecdsa_(rhs.ecdsa_)
{
}

EllipticCurve::EllipticCurve(
    const EllipticCurve& rhs,
    const ReadView newPublic) noexcept
    : Asymmetric(rhs, newPublic)
    , ecdsa_(rhs.ecdsa_)
{
}

EllipticCurve::EllipticCurve(
    const EllipticCurve& rhs,
    OTSecret&& newSecretKey) noexcept
    : Asymmetric(
          rhs,
          [&] {
              auto pubkey = rhs.api_.Factory().Data();
              const auto rc = rhs.ecdsa_.ScalarMultiplyBase(
                  newSecretKey->Bytes(), pubkey->WriteInto());

              if (rc) {

                  return pubkey;
              } else {
                  LogOutput(OT_METHOD)(__func__)(
                      ": Failed to calculate public key")
                      .Flush();

                  return rhs.api_.Factory().Data();
              }
          }(),
          std::move(newSecretKey))
    , ecdsa_(rhs.ecdsa_)
{
}

auto EllipticCurve::asPublicEC() const noexcept
    -> std::unique_ptr<key::EllipticCurve>
{
    auto output = std::unique_ptr<EllipticCurve>{clone_ec()};

    OT_ASSERT(output);

    auto& copy = *output;

    {
        auto lock = Lock{copy.lock_};
        copy.erase_private_data(lock);

        OT_ASSERT(false == copy.has_private(lock));
    }

    return std::move(output);
}

auto EllipticCurve::extract_key(
    const api::Core& api,
    const crypto::EcdsaProvider& ecdsa,
    const proto::AsymmetricKey& proto,
    Data& publicKey) -> std::unique_ptr<proto::Ciphertext>
{
    auto output = std::unique_ptr<proto::Ciphertext>{};
    publicKey.Assign(proto.key());

    if ((proto::KEYMODE_PRIVATE == proto.mode()) && proto.has_encryptedkey()) {
        output = std::make_unique<proto::Ciphertext>(proto.encryptedkey());

        OT_ASSERT(output);
    }

    return output;
}

auto EllipticCurve::IncrementPrivate(
    const Secret& rhs,
    const PasswordPrompt& reason) const noexcept
    -> std::unique_ptr<key::EllipticCurve>
{
    try {
        auto lock = Lock{lock_};
        const auto& lhs = get_private_key(lock, reason);
        auto newKey = api_.Factory().Secret(0);
        auto rc =
            ecdsa_.ScalarAdd(lhs.Bytes(), rhs.Bytes(), newKey->WriteInto());

        if (false == rc) {
            throw std::runtime_error("Failed to increment private key");
        }

        return replace_secret_key(std::move(newKey));
    } catch (const std::exception& e) {
        LogOutput(OT_METHOD)(__func__)(": ")(e.what()).Flush();

        return {};
    }
}

auto EllipticCurve::IncrementPublic(const Secret& rhs) const noexcept
    -> std::unique_ptr<key::EllipticCurve>
{
    try {
        auto newKey = Space{};
        auto rc = ecdsa_.PubkeyAdd(key_->Bytes(), rhs.Bytes(), writer(newKey));

        if (false == rc) {
            throw std::runtime_error("Failed to increment public key");
        }

        return replace_public_key(reader(newKey));
    } catch (const std::exception& e) {
        LogOutput(OT_METHOD)(__func__)(": ")(e.what()).Flush();

        return {};
    }
}

auto EllipticCurve::serialize_public(EllipticCurve* in)
    -> std::shared_ptr<proto::AsymmetricKey>
{
    auto copy = std::unique_ptr<EllipticCurve>{in};

    OT_ASSERT(copy);

    auto serialized = proto::AsymmetricKey{};

    {
        auto lock = Lock{copy->lock_};
        copy->erase_private_data(lock);

        if (false == copy->serialize(lock, serialized)) { return nullptr; }
    }

    return std::make_shared<proto::AsymmetricKey>(serialized);
}

auto EllipticCurve::SignDER(
    const ReadView preimage,
    const crypto::HashType hash,
    Space& output,
    const PasswordPrompt& reason) const noexcept -> bool
{
    auto lock = Lock{lock_};

    if (false == has_private(lock)) {
        LogOutput(OT_METHOD)(__func__)(": Missing private key").Flush();

        return false;
    }

    bool success =
        ecdsa_.SignDER(preimage, private_key(lock, reason), hash, output);

    if (false == success) {
        LogOutput(OT_METHOD)(__func__)(": Failed to sign preimage").Flush();
    }

    return success;
}
}  // namespace opentxs::crypto::key::implementation
