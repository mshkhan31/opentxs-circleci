// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"      // IWYU pragma: associated
#include "1_Internal.hpp"    // IWYU pragma: associated
#include "crypto/Bip39.hpp"  // IWYU pragma: associated

#include <boost/algorithm/string.hpp>
#include <boost/multiprecision/cpp_int.hpp>
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <string_view>
#include <utility>
#include <vector>

#include "2_Factory.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/api/Core.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/api/crypto/Crypto.hpp"
#include "opentxs/api/crypto/Hash.hpp"
#include "opentxs/api/crypto/Symmetric.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"
#include "opentxs/core/Secret.hpp"
#include "opentxs/crypto/HashType.hpp"
#include "opentxs/crypto/SeedStyle.hpp"
#include "opentxs/crypto/key/Symmetric.hpp"
#include "opentxs/crypto/key/symmetric/Source.hpp"
#include "util/Allocator.hpp"
#include "util/ByteLiterals.hpp"
#include "util/Container.hpp"

#define OT_METHOD "opentxs::crypto::implementation::Bip39::"

namespace opentxs
{
auto Factory::Bip39(const api::Crypto& api) noexcept
    -> std::unique_ptr<crypto::Bip39>
{
    using ReturnType = crypto::implementation::Bip39;

    return std::make_unique<ReturnType>(api);
}
}  // namespace opentxs

namespace opentxs::crypto::implementation
{
const std::size_t Bip39::BitsPerWord{11};
const std::uint8_t Bip39::ByteBits{8};
const std::size_t Bip39::DictionarySize{2048};
const std::size_t Bip39::EntropyBitDivisor{32};
const std::size_t Bip39::HmacOutputSizeBytes{64};
const std::size_t Bip39::HmacIterationCount{2048};
const std::string Bip39::PassphrasePrefix{"mnemonic"};
const std::size_t Bip39::ValidMnemonicWordMultiple{3};

Bip39::Bip39(const api::Crypto& crypto) noexcept
    : crypto_(crypto)
{
}

auto Bip39::bitShift(std::size_t theBit) noexcept -> std::byte
{
    return static_cast<std::byte>(1 << (ByteBits - (theBit % ByteBits) - 1));
}

auto Bip39::entropy_to_words(
    const Secret& entropy,
    Secret& words,
    const Language lang) const noexcept -> bool
{
    const auto bytes = entropy.Bytes();

    switch (bytes.size()) {
        case 16:
        case 20:
        case 24:
        case 28:
        case 32:
            break;
        default: {
            LogOutput(OT_METHOD)(__func__)(": Invalid entropy size: ")(
                bytes.size())
                .Flush();

            return false;
        }
    }

    const auto entropyBitCount = std::size_t{bytes.size() * ByteBits};
    const auto checksumBits = std::size_t{entropyBitCount / EntropyBitDivisor};
    const auto entropyPlusCheckBits =
        std::size_t{entropyBitCount + checksumBits};
    const auto wordCount = std::size_t{entropyPlusCheckBits / BitsPerWord};

    if (0 != (wordCount % ValidMnemonicWordMultiple)) {
        LogOutput(OT_METHOD)(__func__)(
            ": (0 != (wordCount % ValidMnemonicWordMultiple))")
            .Flush();

        return false;
    }

    if (0 != (entropyPlusCheckBits % BitsPerWord)) {
        LogOutput(OT_METHOD)(__func__)(
            ": (0 != (entropyPlusCheckBits % BitsPerWord))")
            .Flush();

        return false;
    }

    auto newData = opentxs::Data::Factory();  // TODO should be secret
    auto digestOutput = opentxs::Data::Factory();

    if (false == crypto_.Hash().Digest(
                     opentxs::crypto::HashType::Sha256,
                     bytes,
                     digestOutput->WriteInto())) {
        LogOutput(OT_METHOD)(__func__)(
            ": Digest(opentxs::crypto::HashType::Sha256...) failed.")
            .Flush();

        return false;
    } else {
        newData->Concatenate(bytes);
        newData += digestOutput;
    }

    auto mnemonicWords = MnemonicWords{};
    auto bitIndex = std::size_t{0};

    for (std::size_t currentWord = 0; currentWord < wordCount; currentWord++) {
        auto indexDict = std::size_t{0};

        for (std::size_t bit_iterator = 0; bit_iterator < BitsPerWord;
             bit_iterator++) {
            bitIndex = ((BitsPerWord * currentWord) + bit_iterator);
            indexDict <<= 1;
            const auto byteIndex =
                bitIndex / static_cast<std::size_t>(ByteBits);
            auto indexed_byte = std::byte{0};
            const bool bExtracted = newData->Extract(
                reinterpret_cast<std::uint8_t&>(indexed_byte), byteIndex);

            if (!bExtracted) {
                LogOutput(OT_METHOD)(__func__)(": (!bExtracted) -- returning")
                    .Flush();

                return false;
            }

            if (std::to_integer<std::uint8_t>(
                    indexed_byte & bitShift(bitIndex)) > 0) {
                indexDict++;
            }
        }

        OT_ASSERT(indexDict < DictionarySize);

        try {
            const auto& dictionary = words_.at(lang);
            const auto& theString = dictionary.at(indexDict);
            mnemonicWords.push_back(theString);
        } catch (...) {
            LogOutput(OT_METHOD)(__func__)(": Unsupported language").Flush();

            return false;
        }
    }

    if (mnemonicWords.size() != ((bitIndex + 1) / BitsPerWord)) {
        LogOutput(OT_METHOD)(__func__)(
            ": (mnemonicWords.size() != ((bitIndex + 1) / BitsPerWord))")
            .Flush();

        return false;
    }

    auto output = std::string{};
    auto nIndex = int{-1};

    for (const auto& word : mnemonicWords) {
        ++nIndex;

        if (nIndex > 0) { output += " "; }

        output += word;
    }

    words.AssignText(output);

    return true;
}

auto Bip39::find_longest_words(const Words& in) noexcept -> LongestWords
{
    auto output = LongestWords{};

    for (const auto& [lang, words] : in) {
        auto& max = output[lang];

        for (const auto& word : words) {
            max = std::max(max, std::strlen(word));
        }
    }

    return output;
}

auto Bip39::GetSuggestions(const Language lang, const std::string_view word)
    const noexcept -> Suggestions
{
    if (word.size() == 0) { return {}; }

    try {
        auto output = Suggestions{};
        const auto& words = words_.at(lang);

        if (0 == words.size()) { return {}; }

        for (const auto candidate : words) {
            const auto csize = std::strlen(candidate);
            const auto size = word.size();

            if (csize < size) { continue; }

            const auto start = word.data();
            const auto end = start + size;
            const auto prefix = std::distance(
                start, std::mismatch(start, end, candidate).first);

            if (0 > prefix) { continue; }

            if (size == static_cast<std::size_t>(prefix)) {
                if (csize == size) {
                    output.clear();
                    output.emplace_back(candidate);

                    return output;
                }

                output.emplace_back(candidate);
            }
        }

        dedup(output);

        return output;
    } catch (...) {

        return {};
    }
}

auto Bip39::LongestWord(const Language lang) const noexcept -> std::size_t
{
    try {

        return longest_words_.at(lang);
    } catch (...) {

        return {};
    }
}

auto Bip39::SeedToWords(const Secret& seed, Secret& words, const Language lang)
    const noexcept -> bool
{
    return entropy_to_words(seed, words, lang);
}

auto Bip39::tokenize(const Language lang, const ReadView words) noexcept(false)
    -> std::vector<std::size_t>
{
    auto s = std::vector<std::string>{};
    boost::split(
        s, words, [](char c) { return c == ' '; }, boost::token_compress_on);
    const auto& d = [&] {
        try {

            return words_.at(lang);
        } catch (...) {

            throw std::runtime_error{"Unsupported language"};
        }
    }();
    auto output = std::vector<std::size_t>{};
    output.reserve(s.size());
    const auto first = d.begin();

    for (const auto& word : s) {
        if (auto it = std::find(d.begin(), d.end(), word); it != d.end()) {
            output.emplace_back(std::distance(first, it));
        }
    }

    return output;
}

auto Bip39::words_to_root_bip39(
    const Secret& words,
    Secret& bip32RootNode,
    const Secret& passphrase) const noexcept -> bool
{
    auto salt = std::string{PassphrasePrefix};

    if (passphrase.size() > 0) { salt += std::string{passphrase.Bytes()}; }

    auto dataOutput = opentxs::Data::Factory();  // TODO should be secret
    const auto dataSalt = opentxs::Data::Factory(salt.data(), salt.size());
    crypto_.Hash().PKCS5_PBKDF2_HMAC(
        words,
        dataSalt,
        HmacIterationCount,
        crypto::HashType::Sha512,
        HmacOutputSizeBytes,
        dataOutput);
    bip32RootNode.Assign(dataOutput->Bytes());

    return true;
}

auto Bip39::words_to_root_pkt(
    const api::Core& api,
    const Language lang,
    const Secret& words,
    Secret& bip32RootNode,
    const Secret& passphrase) const noexcept -> bool
{
    const auto indices = tokenize(lang, words.Bytes());

    if (const auto size{indices.size()}; 15u != size) {
        LogOutput(OT_METHOD)(__func__)(": incorrect number of words: ")(size)
            .Flush();

        return false;
    }

    using Allocator = SecureAllocator<std::uint8_t>;
    using Vector = std::vector<std::uint8_t, Allocator>;
    auto ent = [&] {
        namespace mp = boost::multiprecision;
        using BigInt = mp::number<mp::cpp_int_backend<
            168,
            168,
            mp::unsigned_magnitude,
            mp::unchecked,
            void>>;
        auto alloc = SecureAllocator<BigInt>{};
        auto pSeed = std::allocate_shared<BigInt>(alloc);

        OT_ASSERT(pSeed);

        auto& seed = *pSeed;
        auto out = Vector{};

        for (auto i{indices.crbegin()}; i != indices.crend(); ++i) {
            seed <<= 11;
            seed |= BigInt{*i};
        }

        mp::export_bits(seed, std::back_inserter(out), 8, true);

        return out;
    }();
    static constexpr auto reader = [](const Vector& in) {
        return ReadView{reinterpret_cast<const char*>(in.data()), in.size()};
    };

    OT_ASSERT(21 == ent.size());

    const auto version = (ent[0] >> 1) & 0x0f;

    if (0 != version) {
        LogOutput(OT_METHOD)(__func__)(": unsupported version: ")(version)
            .Flush();

        return false;
    }

    const auto encrypted = ent[0] & 0x01;

    {
        const auto checksum = ent[1];
        ent[0] &= 0x1f;
        ent[1] &= 0x00;

        try {
            const auto calculated = [&] {
                auto hash = Space{};
                crypto_.Hash().Digest(
                    HashType::Blake2b256, reader(ent), writer(hash));

                if (0 == hash.size()) {
                    throw std::runtime_error{"failed to calculate hash"};
                }

                return std::to_integer<std::uint8_t>(hash.at(0));
            }();

            if (checksum != calculated) {
                throw std::runtime_error{"checksum failure"};
            }
        } catch (const std::exception& e) {
            LogOutput(OT_METHOD)(__func__)(": ")(e.what()).Flush();

            return false;
        }
    }

    if (encrypted) {
        if (0 == passphrase.size()) {
            LogOutput(OT_METHOD)(__func__)(
                ": passphrase required but not provided")
                .Flush();

            return false;
        }

        static const auto salt = std::string{"pktwallet seed 0"};
        static constexpr auto keyBytes = std::size_t{19u};
        const auto key = [&] {
            const auto sKey = api.Symmetric().Key(
                passphrase,
                salt,
                32,
                256_MiB,
                8,
                keyBytes,
                key::symmetric::Source::Argon2id);
            auto output = api.Factory().Secret(0u);
            const auto reason = api.Factory().PasswordPrompt(__func__);
            sKey->RawKey(reason, output);

            return output;
        }();

        OT_ASSERT(keyBytes == key->size());

        auto v = key->Bytes();
        auto k{v.data()};

        for (auto b = std::next(ent.begin(), 2); b < ent.end(); ++b, ++k) {
            (*b) ^= (*k);
        }
    }

    const auto view = ReadView{
        reinterpret_cast<const char*>(std::next(ent.data(), 2)),
        ent.size() - 2u};
    bip32RootNode.Assign(view);

    return true;
}

auto Bip39::WordsToSeed(
    const api::Core& api,
    const SeedStyle type,
    const Language lang,
    const Secret& words,
    Secret& seed,
    const Secret& passphrase) const noexcept -> bool
{
    switch (type) {
        case SeedStyle::BIP39: {

            return words_to_root_bip39(words, seed, passphrase);
        }
        case SeedStyle::PKT: {

            return words_to_root_pkt(api, lang, words, seed, passphrase);
        }
        default: {
            LogOutput(OT_METHOD)(__func__)(": unsupported type").Flush();

            return false;
        }
    }
}
}  // namespace opentxs::crypto::implementation
