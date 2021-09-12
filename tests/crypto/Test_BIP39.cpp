// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <cctype>
#include <cstddef>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "crypto/Bip39.hpp"
#include "opentxs/OT.hpp"
#include "opentxs/api/Context.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/api/HDSeed.hpp"
#include "opentxs/api/Wallet.hpp"
#include "opentxs/api/client/Blockchain.hpp"
#include "opentxs/api/client/Manager.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/crypto/Element.hpp"
#include "opentxs/blockchain/crypto/HD.hpp"
#include "opentxs/blockchain/crypto/HDProtocol.hpp"
#include "opentxs/blockchain/crypto/Subchain.hpp"
#include "opentxs/core/PasswordPrompt.hpp"
#include "opentxs/crypto/Language.hpp"
#include "opentxs/crypto/SeedStrength.hpp"
#include "opentxs/crypto/SeedStyle.hpp"

namespace ot = opentxs;

namespace ottest
{
class Test_BIP39 : public ::testing::Test
{
public:
    static constexpr auto type_{ot::crypto::SeedStyle::BIP39};
    static constexpr auto lang_{ot::crypto::Language::en};
    static std::set<std::string> generated_seeds_;

    const ot::api::client::Manager& api_;
    const ot::OTPasswordPrompt reason_;

    static auto expected_seed_languages() noexcept -> const
        std::map<ot::crypto::SeedStyle, ot::api::HDSeed::SupportedLanguages>&
    {
        using Language = ot::crypto::Language;
        static const auto data = std::
            map<ot::crypto::SeedStyle, ot::api::HDSeed::SupportedLanguages>{
                {ot::crypto::SeedStyle::BIP39,
                 {
                     {Language::en, "English"},
                 }},
                {ot::crypto::SeedStyle::PKT,
                 {
                     {Language::en, "English"},
                 }},
            };

        return data;
    }
    static auto expected_seed_strength() noexcept -> const
        std::map<ot::crypto::SeedStyle, ot::api::HDSeed::SupportedStrengths>&
    {
        using Style = ot::crypto::SeedStyle;
        using Strength = ot::crypto::SeedStrength;
        static const auto data = std::
            map<ot::crypto::SeedStyle, ot::api::HDSeed::SupportedStrengths>{
                {Style::BIP39,
                 {
                     {Strength::Twelve, "12"},
                     {Strength::Fifteen, "15"},
                     {Strength::Eighteen, "18"},
                     {Strength::TwentyOne, "21"},
                     {Strength::TwentyFour, "24"},
                 }},
                {Style::PKT,
                 {
                     {Strength::Fifteen, "15"},
                 }},
            };

        return data;
    }
    static auto expected_seed_types() noexcept
        -> const ot::api::HDSeed::SupportedSeeds&
    {
        static const auto data = ot::api::HDSeed::SupportedSeeds{
            {ot::crypto::SeedStyle::BIP39, "BIP-39"},
            {ot::crypto::SeedStyle::PKT, "Legacy pktwallet"},
        };

        return data;
    }

    static auto word_count(const std::string& in) noexcept -> std::size_t
    {
        if (0 == in.size()) { return 0; }

        auto word = false;
        auto count = std::size_t{};

        for (const auto c : in) {
            if (std::isspace(c)) {
                if (word) {
                    word = false;
                } else {
                    continue;
                }
            } else {
                if (word) {
                    continue;
                } else {
                    word = true;
                    ++count;
                }
            }
        }

        return count;
    }

    auto generate_words(const ot::crypto::SeedStrength count) const
        -> std::size_t
    {
        const auto fingerprint =
            api_.Seeds().NewSeed(type_, lang_, count, reason_);

        EXPECT_EQ(generated_seeds_.count(fingerprint), 0);

        if (0 < generated_seeds_.count(fingerprint)) { return 0; }

        generated_seeds_.insert(fingerprint);

        const auto words = api_.Seeds().Words(fingerprint, reason_);

        return word_count(words);
    }

    Test_BIP39()
        : api_(ot::Context().StartClient(0))
        , reason_(api_.Factory().PasswordPrompt(__func__))
    {
    }
};

std::set<std::string> Test_BIP39::generated_seeds_{};

TEST_F(Test_BIP39, seed_types)
{
    const auto& expected = expected_seed_types();
    const auto test = api_.Seeds().AllowedSeedTypes();

    EXPECT_EQ(test.size(), expected.size());

    for (const auto& [type, text] : expected) {
        try {
            EXPECT_EQ(text, test.at(type));
        } catch (...) {
            EXPECT_TRUE(false);
        }
    }
}

TEST_F(Test_BIP39, seed_languages)
{
    for (const auto& [type, expected] : expected_seed_languages()) {
        const auto test = api_.Seeds().AllowedLanguages(type);

        EXPECT_EQ(test.size(), expected.size());

        for (const auto& [lang, text] : expected) {
            try {
                EXPECT_EQ(text, test.at(lang));
            } catch (...) {
                EXPECT_TRUE(false);
            }
        }
    }
}

TEST_F(Test_BIP39, seed_strength)
{
    for (const auto& [type, expected] : expected_seed_strength()) {
        const auto test = api_.Seeds().AllowedSeedStrength(type);

        EXPECT_EQ(test.size(), expected.size());

        for (const auto& [strength, text] : expected) {
            try {
                EXPECT_EQ(text, test.at(strength));
            } catch (...) {
                EXPECT_TRUE(false);
            }
        }
    }
}

TEST_F(Test_BIP39, word_count)
{
    static const auto vector = std::map<std::string, std::size_t>{
        {"", 0},
        {" ", 0},
        {"     ", 0},
        {"one", 1},
        {" one", 1},
        {"   one", 1},
        {"one ", 1},
        {" one ", 1},
        {"   one   ", 1},
        {"one two", 2},
        {" one  two ", 2},
        {"   one   two ", 2},
        {"   one   two 3", 3},
    };

    for (const auto& [string, count] : vector) {
        EXPECT_EQ(word_count(string), count);
    }
}

TEST_F(Test_BIP39, longest_en)
{
    EXPECT_EQ(api_.Seeds().LongestWord(type_, lang_), 8);
}

TEST_F(Test_BIP39, twelve_words)
{
    const auto& api = api_.Seeds();
    constexpr auto strength{ot::crypto::SeedStrength::Twelve};
    constexpr auto words{12};

    EXPECT_EQ(generate_words(strength), words);
    EXPECT_EQ(api.WordCount(type_, strength), words);
}

TEST_F(Test_BIP39, fifteen_words)
{
    const auto& api = api_.Seeds();
    constexpr auto strength{ot::crypto::SeedStrength::Fifteen};
    constexpr auto words{15};

    EXPECT_EQ(generate_words(strength), words);
    EXPECT_EQ(api.WordCount(type_, strength), words);
}

TEST_F(Test_BIP39, eighteen_words)
{
    const auto& api = api_.Seeds();
    constexpr auto strength{ot::crypto::SeedStrength::Eighteen};
    constexpr auto words{18};

    EXPECT_EQ(generate_words(strength), words);
    EXPECT_EQ(api.WordCount(type_, strength), words);
}

TEST_F(Test_BIP39, twentyone_words)
{
    const auto& api = api_.Seeds();
    constexpr auto strength{ot::crypto::SeedStrength::TwentyOne};
    constexpr auto words{21};

    EXPECT_EQ(generate_words(strength), words);
    EXPECT_EQ(api.WordCount(type_, strength), words);
}

TEST_F(Test_BIP39, twentyfour_words)
{
    const auto& api = api_.Seeds();
    constexpr auto strength{ot::crypto::SeedStrength::TwentyFour};
    constexpr auto words{24};

    EXPECT_EQ(generate_words(strength), words);
    EXPECT_EQ(api.WordCount(type_, strength), words);
}

TEST_F(Test_BIP39, match_a)
{
    const auto test = std::string{"a"};
    const auto expected = std::vector<std::string>{
        "abandon", "ability",  "able",     "about",    "above",    "absent",
        "absorb",  "abstract", "absurd",   "abuse",    "access",   "accident",
        "account", "accuse",   "achieve",  "acid",     "acoustic", "acquire",
        "across",  "act",      "action",   "actor",    "actress",  "actual",
        "adapt",   "add",      "addict",   "address",  "adjust",   "admit",
        "adult",   "advance",  "advice",   "aerobic",  "affair",   "afford",
        "afraid",  "again",    "age",      "agent",    "agree",    "ahead",
        "aim",     "air",      "airport",  "aisle",    "alarm",    "album",
        "alcohol", "alert",    "alien",    "all",      "alley",    "allow",
        "almost",  "alone",    "alpha",    "already",  "also",     "alter",
        "always",  "amateur",  "amazing",  "among",    "amount",   "amused",
        "analyst", "anchor",   "ancient",  "anger",    "angle",    "angry",
        "animal",  "ankle",    "announce", "annual",   "another",  "answer",
        "antenna", "antique",  "anxiety",  "any",      "apart",    "apology",
        "appear",  "apple",    "approve",  "april",    "arch",     "arctic",
        "area",    "arena",    "argue",    "arm",      "armed",    "armor",
        "army",    "around",   "arrange",  "arrest",   "arrive",   "arrow",
        "art",     "artefact", "artist",   "artwork",  "ask",      "aspect",
        "assault", "asset",    "assist",   "assume",   "asthma",   "athlete",
        "atom",    "attack",   "attend",   "attitude", "attract",  "auction",
        "audit",   "august",   "aunt",     "author",   "auto",     "autumn",
        "average", "avocado",  "avoid",    "awake",    "aware",    "away",
        "awesome", "awful",    "awkward",  "axis",
    };
    const auto suggestions = api_.Seeds().ValidateWord(type_, lang_, test);

    EXPECT_EQ(suggestions, expected);
}

TEST_F(Test_BIP39, match_ar)
{
    const auto test = std::string{"ar"};
    const auto expected = std::vector<std::string>{
        "arch",
        "arctic",
        "area",
        "arena",
        "argue",
        "arm",
        "armed",
        "armor",
        "army",
        "around",
        "arrange",
        "arrest",
        "arrive",
        "arrow",
        "art",
        "artefact",
        "artist",
        "artwork"};
    const auto suggestions = api_.Seeds().ValidateWord(type_, lang_, test);

    EXPECT_EQ(suggestions, expected);
}

TEST_F(Test_BIP39, match_arr)
{
    const auto test = std::string{"arr"};
    const auto expected =
        std::vector<std::string>{"arrange", "arrest", "arrive", "arrow"};
    const auto suggestions = api_.Seeds().ValidateWord(type_, lang_, test);

    EXPECT_EQ(suggestions, expected);
}

TEST_F(Test_BIP39, match_arri)
{
    const auto test = std::string{"arri"};
    const auto expected = std::vector<std::string>{"arrive"};
    const auto suggestions = api_.Seeds().ValidateWord(type_, lang_, test);

    EXPECT_EQ(suggestions, expected);
}

TEST_F(Test_BIP39, match_arrive)
{
    const auto test = std::string{"arrive"};
    const auto expected = std::vector<std::string>{"arrive"};
    const auto suggestions = api_.Seeds().ValidateWord(type_, lang_, test);

    EXPECT_EQ(suggestions, expected);
}

TEST_F(Test_BIP39, match_arrived)
{
    const auto test = std::string{"arrived"};
    const auto expected = std::vector<std::string>{};
    const auto suggestions = api_.Seeds().ValidateWord(type_, lang_, test);

    EXPECT_EQ(suggestions, expected);
}

TEST_F(Test_BIP39, match_axe)
{
    const auto test = std::string{"axe"};
    const auto expected = std::vector<std::string>{};
    const auto suggestions = api_.Seeds().ValidateWord(type_, lang_, test);

    EXPECT_EQ(suggestions, expected);
}

TEST_F(Test_BIP39, match_empty_string)
{
    const auto test = std::string{""};
    const auto expected = std::vector<std::string>{};
    const auto suggestions = api_.Seeds().ValidateWord(type_, lang_, test);

    EXPECT_EQ(suggestions, expected);
}

TEST_F(Test_BIP39, validate_en_list)
{
    using Bip39 = opentxs::crypto::implementation::Bip39;

    for (const auto* word : Bip39::words_.at(lang_)) {
        const auto matches = api_.Seeds().ValidateWord(type_, lang_, word);

        EXPECT_EQ(matches.size(), 1);
    }
}

TEST_F(Test_BIP39, pkt_seed_import)
{
    static constexpr auto phrase{
        "forum school old approve bubble warfare robust figure pact glance "
        "farm leg taxi sing ankle"};
    static constexpr auto password{"Password123#"};
    static constexpr auto expected_seed_bytes_{
        "498d7c2713178cb9c78ac00061b0969429792f"};
    static const auto expected_secret_keys_ = std::vector<std::string>{
        "885e744b15e847044bbb33c80c8aeb26abb7a2c2a5120b0a64dec8c12062f3a6",
        "0e9bfa981927a86d2a1329984c5a45a3fa8a0a684c351e1970380d6a2d5fecc1",
        "63e72656faf5f756d82430f9f45e273abc24d4303d11ba5cca78ac49f8f1a73d",
        "d2e890b1e46695e27f1bcb32546f71c11f384d485216034ffdadc54516ba3487",
        "427c65d4380bae5c535cedfa4e080c9b61fad98a153ca2572d7459ee3db78b8d",
        "d8b037cea44b93f9fae44b70bf6a210bd3e8fc4718d3a162b9118a751e9bc347",
        "4536717d7da8f4ac0b21b5cba8e9967f36e1b7b3fa852e7c9bd96ad86fc86552",
        "29c7226f31458c2c0192bdd0837ed26be513d20f7606dc763b4d7cd68391a92e",
        "0ba52306295a4af4548aeea16b2d3422a14f81fefc3a08b77a267e2d4296e83b",
        "ea0ef4bd3597ab55cec5243180d65b9272014241ea6e2a7c895feec147ddef92",
    };
    const auto seed = api_.Seeds().ImportSeed(
        api_.Factory().SecretFromText(phrase),
        api_.Factory().SecretFromText(password),
        ot::crypto::SeedStyle::PKT,
        ot::crypto::Language::en,
        reason_);

    ASSERT_FALSE(seed.empty());

    const auto entropy = api_.Seeds().Bip32Root(seed, reason_);

    EXPECT_EQ(entropy, expected_seed_bytes_);

    static constexpr auto index{0u};
    const auto pNym = api_.Wallet().Nym(reason_, "pkt", {seed, index});

    ASSERT_TRUE(pNym);

    const auto& nym = *pNym;
    const auto accountID = api_.Blockchain().NewHDSubaccount(
        nym.ID(),
        ot::blockchain::crypto::HDProtocol::BIP_84,
        ot::blockchain::Type::Bitcoin,
        ot::blockchain::Type::PKT,
        reason_);

    ASSERT_FALSE(accountID->empty());

    const auto& account = api_.Blockchain().HDSubaccount(nym.ID(), accountID);

    EXPECT_EQ(account.Standard(), ot::blockchain::crypto::HDProtocol::BIP_84);

    static constexpr auto subchain{ot::blockchain::crypto::Subchain::External};

    while (*account.LastGenerated(subchain) < 10) {
        account.GenerateNext(subchain, reason_);
    }

    using Index = ot::Bip32Index;

    for (auto i = Index{0}; i < 10u; ++i) {
        const auto& element = account.BalanceElement(subchain, i);
        const auto pKey = element.PrivateKey(reason_);

        ASSERT_TRUE(pKey);

        const auto& key = *pKey;
        const auto secret = [&] {
            auto out = api_.Factory().Data(key.PrivateKey(reason_));

            return out;
        }();

        EXPECT_EQ(secret->asHex(), expected_secret_keys_.at(i));
    }
}
}  // namespace ottest
