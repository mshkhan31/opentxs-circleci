// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"     // IWYU pragma: associated
#include "1_Internal.hpp"   // IWYU pragma: associated
#include "util/Sodium.hpp"  // IWYU pragma: associated

extern "C" {
#include <sodium.h>
}

#include <functional>
#include <string_view>

#include "opentxs/Pimpl.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"

namespace opentxs::crypto::sodium
{
auto ExpandSeed(
    const ReadView seed,
    const AllocateOutput privateKey,
    const AllocateOutput publicKey) noexcept -> bool
{
    if ((nullptr == seed.data()) || (0 == seed.size())) {
        LogOutput(__func__)(": Invalid provided seed").Flush();

        return false;
    }

    auto hashed = Data::Factory();
    auto nSeed{seed};

    if (crypto_sign_SEEDBYTES != seed.size()) {
        auto allocator = hashed->WriteInto();

        if (false == bool(allocator)) {
            LogOutput(__func__)(": Failed to get hash allocator").Flush();

            return false;
        }

        auto output = allocator(crypto_sign_SEEDBYTES);

        if (false == output.valid(crypto_sign_SEEDBYTES)) {
            LogOutput(__func__)(": Failed to allocate space for hashed seed")
                .Flush();

            return false;
        }

        if (0 != ::crypto_generichash(
                     output.as<unsigned char>(),
                     output,
                     reinterpret_cast<const unsigned char*>(seed.data()),
                     seed.size(),
                     nullptr,
                     0)) {
            LogOutput(__func__)(": Failed to normalize seed").Flush();

            return false;
        }

        nSeed = hashed->Bytes();
    }

    if ((nullptr == nSeed.data()) || (crypto_sign_SEEDBYTES != nSeed.size())) {
        LogOutput(__func__)(": Invalid normalized seed").Flush();

        return false;
    }

    if (false == bool(privateKey) || false == bool(publicKey)) {
        LogOutput(__func__)(": Invalid output allocator").Flush();

        return false;
    }

    auto prv = privateKey(crypto_sign_SECRETKEYBYTES);
    auto pub = publicKey(crypto_sign_PUBLICKEYBYTES);

    if (false == prv.valid(crypto_sign_SECRETKEYBYTES)) {
        LogOutput(__func__)(": Failed to allocate space for private key")
            .Flush();

        return false;
    }

    if (false == pub.valid(crypto_sign_PUBLICKEYBYTES)) {
        LogOutput(__func__)(": Failed to allocate space for public key")
            .Flush();

        return false;
    }

    return 0 == ::crypto_sign_seed_keypair(
                    pub.as<unsigned char>(),
                    prv.as<unsigned char>(),
                    reinterpret_cast<const unsigned char*>(nSeed.data()));
}

auto ToCurveKeypair(
    const ReadView edPrivate,
    const ReadView edPublic,
    const AllocateOutput curvePrivate,
    const AllocateOutput curvePublic) noexcept -> bool
{
    if (nullptr == edPrivate.data() ||
        crypto_sign_SECRETKEYBYTES != edPrivate.size()) {
        LogOutput(__func__)(": Invalid ed25519 private key").Flush();

        return false;
    }

    if (nullptr == edPublic.data() ||
        crypto_sign_PUBLICKEYBYTES != edPublic.size()) {
        LogOutput(__func__)(": Invalid ed25519 public key").Flush();

        return false;
    }

    if (false == bool(curvePrivate) || false == bool(curvePublic)) {
        LogOutput(__func__)(": Invalid output allocator").Flush();

        return false;
    }

    auto prv = curvePrivate(crypto_scalarmult_curve25519_BYTES);
    auto pub = curvePublic(crypto_scalarmult_curve25519_BYTES);

    if (false == prv.valid(crypto_scalarmult_curve25519_BYTES)) {
        LogOutput(__func__)(": Failed to allocate space for private key")
            .Flush();

        return false;
    }

    if (false == pub.valid(crypto_scalarmult_curve25519_BYTES)) {
        LogOutput(__func__)(": Failed to allocate space for public key")
            .Flush();

        return false;
    }

    if (0 != ::crypto_sign_ed25519_sk_to_curve25519(
                 prv.as<unsigned char>(),
                 reinterpret_cast<const unsigned char*>(edPrivate.data()))) {
        LogOutput(__func__)(
            ": Failed to convert private key from ed25519 to curve25519.")
            .Flush();

        return false;
    }

    if (0 != ::crypto_sign_ed25519_pk_to_curve25519(
                 pub.as<unsigned char>(),
                 reinterpret_cast<const unsigned char*>(edPublic.data()))) {
        LogOutput(__func__)(
            ": Failed to convert public key from ed25519 to curve25519.")
            .Flush();

        return false;
    }

    return true;
}
}  // namespace opentxs::crypto::sodium
