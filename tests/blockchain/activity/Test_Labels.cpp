// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <memory>
#include <optional>
#include <string>

#include "Helpers.hpp"
#include "opentxs/api/client/Blockchain.hpp"
#include "opentxs/api/client/Contacts.hpp"
#include "opentxs/api/client/Manager.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/block/bitcoin/Output.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/block/bitcoin/Outputs.hpp"
#include "opentxs/blockchain/block/bitcoin/Transaction.hpp"
#include "opentxs/blockchain/crypto/HD.hpp"
#include "opentxs/blockchain/crypto/Subchain.hpp"
#include "opentxs/contact/Contact.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/PasswordPrompt.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/crypto/Types.hpp"

using Subchain = ot::blockchain::crypto::Subchain;

std::string txid_{};
ot::Bip32Index first_index_{};
ot::Bip32Index second_index_{};
constexpr auto label_1_{"label one"};
constexpr auto label_2_{"label two"};

namespace ottest
{
TEST_F(Test_BlockchainActivity, init)
{
    EXPECT_FALSE(nym_1_id().empty());
    EXPECT_FALSE(nym_2_id().empty());
    EXPECT_FALSE(account_1_id().empty());
    EXPECT_FALSE(account_2_id().empty());
    EXPECT_FALSE(contact_1_id().empty());
    EXPECT_FALSE(contact_2_id().empty());
    EXPECT_FALSE(contact_3_id().empty());
    EXPECT_FALSE(contact_4_id().empty());

    {
        const auto contact = api_.Contacts().Contact(contact_1_id());

        ASSERT_TRUE(contact);
        EXPECT_EQ(contact->Label(), nym_1_name_);
        EXPECT_EQ(api_.Contacts().ContactName(contact_1_id()), nym_1_name_);
    }

    {
        const auto contact = api_.Contacts().Contact(contact_2_id());

        ASSERT_TRUE(contact);
        EXPECT_EQ(contact->Label(), nym_2_name_);
        EXPECT_EQ(api_.Contacts().ContactName(contact_2_id()), nym_2_name_);
    }

    {
        const auto contact = api_.Contacts().Contact(contact_3_id());

        ASSERT_TRUE(contact);
        EXPECT_EQ(contact->Label(), contact_3_name_);
        EXPECT_EQ(api_.Contacts().ContactName(contact_3_id()), contact_3_name_);
    }

    {
        const auto contact = api_.Contacts().Contact(contact_4_id());

        ASSERT_TRUE(contact);
        EXPECT_EQ(contact->Label(), contact_4_name_);
        EXPECT_EQ(api_.Contacts().ContactName(contact_4_id()), contact_4_name_);
    }
}

TEST_F(Test_BlockchainActivity, unlabled)
{
    const auto& account =
        api_.Blockchain().HDSubaccount(nym_1_id(), account_1_id());
    const auto indexOne = account.Reserve(Subchain::External, reason_);
    const auto indexTwo = account.Reserve(Subchain::External, reason_);

    ASSERT_TRUE(indexOne.has_value());
    ASSERT_TRUE(indexTwo.has_value());

    first_index_ = indexOne.value();
    second_index_ = indexTwo.value();
    const auto& keyOne =
        account.BalanceElement(Subchain::External, first_index_);
    const auto& keyTwo =
        account.BalanceElement(Subchain::External, second_index_);
    const auto incoming = get_test_transaction(keyOne, keyTwo);

    ASSERT_TRUE(incoming);

    txid_ = incoming->ID().asHex();

    ASSERT_TRUE(api_.Blockchain().ProcessTransaction(
        ot::blockchain::Type::Bitcoin, *incoming, reason_));

    auto transaction = api_.Blockchain().LoadTransactionBitcoin(txid_);

    ASSERT_TRUE(transaction);
    EXPECT_TRUE(transaction->Memo(api_.Blockchain()).empty());
    EXPECT_TRUE(transaction->Outputs().at(0).Note(api_.Blockchain()).empty());
    EXPECT_TRUE(transaction->Outputs().at(1).Note(api_.Blockchain()).empty());
}

TEST_F(Test_BlockchainActivity, label_A)
{
    ASSERT_TRUE(api_.Blockchain().AssignLabel(
        nym_1_id(),
        account_1_id(),
        Subchain::External,
        first_index_,
        label_1_));

    auto transaction = api_.Blockchain().LoadTransactionBitcoin(txid_);

    ASSERT_TRUE(transaction);
    EXPECT_STREQ(transaction->Memo(api_.Blockchain()).c_str(), label_1_);
    EXPECT_EQ(transaction->Outputs().at(0).Note(api_.Blockchain()), label_1_);
    EXPECT_TRUE(transaction->Outputs().at(1).Note(api_.Blockchain()).empty());
    ASSERT_TRUE(api_.Blockchain().AssignLabel(
        nym_1_id(), account_1_id(), Subchain::External, first_index_, ""));

    transaction = api_.Blockchain().LoadTransactionBitcoin(txid_);

    ASSERT_TRUE(transaction);
    EXPECT_TRUE(transaction->Memo(api_.Blockchain()).empty());
    EXPECT_TRUE(transaction->Outputs().at(0).Note(api_.Blockchain()).empty());
    EXPECT_TRUE(transaction->Outputs().at(1).Note(api_.Blockchain()).empty());
}

TEST_F(Test_BlockchainActivity, label_B)
{
    ASSERT_TRUE(api_.Blockchain().AssignLabel(
        nym_1_id(),
        account_1_id(),
        Subchain::External,
        second_index_,
        label_2_));

    auto transaction = api_.Blockchain().LoadTransactionBitcoin(txid_);

    ASSERT_TRUE(transaction);
    EXPECT_STREQ(transaction->Memo(api_.Blockchain()).c_str(), label_2_);
    EXPECT_TRUE(transaction->Outputs().at(0).Note(api_.Blockchain()).empty());
    EXPECT_EQ(transaction->Outputs().at(1).Note(api_.Blockchain()), label_2_);
    ASSERT_TRUE(api_.Blockchain().AssignLabel(
        nym_1_id(), account_1_id(), Subchain::External, second_index_, ""));

    transaction = api_.Blockchain().LoadTransactionBitcoin(txid_);

    ASSERT_TRUE(transaction);
    EXPECT_TRUE(transaction->Memo(api_.Blockchain()).empty());
    EXPECT_TRUE(transaction->Outputs().at(0).Note(api_.Blockchain()).empty());
    EXPECT_TRUE(transaction->Outputs().at(1).Note(api_.Blockchain()).empty());
}

TEST_F(Test_BlockchainActivity, label_AB)
{
    ASSERT_TRUE(api_.Blockchain().AssignLabel(
        nym_1_id(),
        account_1_id(),
        Subchain::External,
        first_index_,
        label_1_));

    auto transaction = api_.Blockchain().LoadTransactionBitcoin(txid_);

    ASSERT_TRUE(transaction);
    EXPECT_STREQ(transaction->Memo(api_.Blockchain()).c_str(), label_1_);
    EXPECT_EQ(transaction->Outputs().at(0).Note(api_.Blockchain()), label_1_);
    EXPECT_TRUE(transaction->Outputs().at(1).Note(api_.Blockchain()).empty());
    ASSERT_TRUE(api_.Blockchain().AssignLabel(
        nym_1_id(),
        account_1_id(),
        Subchain::External,
        second_index_,
        label_2_));

    transaction = api_.Blockchain().LoadTransactionBitcoin(txid_);

    ASSERT_TRUE(transaction);
    // Memo is undefined
    EXPECT_FALSE(transaction->Memo(api_.Blockchain()).empty());
    EXPECT_EQ(transaction->Outputs().at(0).Note(api_.Blockchain()), label_1_);
    EXPECT_EQ(transaction->Outputs().at(1).Note(api_.Blockchain()), label_2_);

    ASSERT_TRUE(api_.Blockchain().AssignLabel(
        nym_1_id(), account_1_id(), Subchain::External, first_index_, ""));
    ASSERT_TRUE(api_.Blockchain().AssignLabel(
        nym_1_id(), account_1_id(), Subchain::External, second_index_, ""));

    transaction = api_.Blockchain().LoadTransactionBitcoin(txid_);

    ASSERT_TRUE(transaction);
    EXPECT_TRUE(transaction->Memo(api_.Blockchain()).empty());
    EXPECT_TRUE(transaction->Outputs().at(0).Note(api_.Blockchain()).empty());
    EXPECT_TRUE(transaction->Outputs().at(1).Note(api_.Blockchain()).empty());
}

TEST_F(Test_BlockchainActivity, label_BA)
{
    ASSERT_TRUE(api_.Blockchain().AssignLabel(
        nym_1_id(),
        account_1_id(),
        Subchain::External,
        second_index_,
        label_2_));

    auto transaction = api_.Blockchain().LoadTransactionBitcoin(txid_);

    ASSERT_TRUE(transaction);
    EXPECT_STREQ(transaction->Memo(api_.Blockchain()).c_str(), label_2_);
    EXPECT_TRUE(transaction->Outputs().at(0).Note(api_.Blockchain()).empty());
    EXPECT_EQ(transaction->Outputs().at(1).Note(api_.Blockchain()), label_2_);
    ASSERT_TRUE(api_.Blockchain().AssignLabel(
        nym_1_id(),
        account_1_id(),
        Subchain::External,
        first_index_,
        label_1_));

    transaction = api_.Blockchain().LoadTransactionBitcoin(txid_);

    ASSERT_TRUE(transaction);
    // Memo is undefined
    EXPECT_FALSE(transaction->Memo(api_.Blockchain()).empty());
    EXPECT_EQ(transaction->Outputs().at(0).Note(api_.Blockchain()), label_1_);
    EXPECT_EQ(transaction->Outputs().at(1).Note(api_.Blockchain()), label_2_);

    ASSERT_TRUE(api_.Blockchain().AssignLabel(
        nym_1_id(), account_1_id(), Subchain::External, first_index_, ""));
    ASSERT_TRUE(api_.Blockchain().AssignLabel(
        nym_1_id(), account_1_id(), Subchain::External, second_index_, ""));

    transaction = api_.Blockchain().LoadTransactionBitcoin(txid_);

    ASSERT_TRUE(transaction);
    EXPECT_TRUE(transaction->Memo(api_.Blockchain()).empty());
    EXPECT_TRUE(transaction->Outputs().at(0).Note(api_.Blockchain()).empty());
    EXPECT_TRUE(transaction->Outputs().at(1).Note(api_.Blockchain()).empty());
}

TEST_F(Test_BlockchainActivity, memo)
{
    constexpr auto memo{"memo"};

    auto transaction = api_.Blockchain().LoadTransactionBitcoin(txid_);

    ASSERT_TRUE(transaction);
    EXPECT_TRUE(transaction->Memo(api_.Blockchain()).empty());
    EXPECT_TRUE(transaction->Outputs().at(0).Note(api_.Blockchain()).empty());
    EXPECT_TRUE(transaction->Outputs().at(1).Note(api_.Blockchain()).empty());
    ASSERT_TRUE(api_.Blockchain().AssignTransactionMemo(txid_, memo));

    transaction = api_.Blockchain().LoadTransactionBitcoin(txid_);

    ASSERT_TRUE(transaction);
    EXPECT_STREQ(transaction->Memo(api_.Blockchain()).c_str(), memo);
    EXPECT_TRUE(transaction->Outputs().at(0).Note(api_.Blockchain()).empty());
    EXPECT_TRUE(transaction->Outputs().at(1).Note(api_.Blockchain()).empty());
    ASSERT_TRUE(api_.Blockchain().AssignLabel(
        nym_1_id(),
        account_1_id(),
        Subchain::External,
        first_index_,
        label_1_));

    transaction = api_.Blockchain().LoadTransactionBitcoin(txid_);

    ASSERT_TRUE(transaction);
    EXPECT_STREQ(transaction->Memo(api_.Blockchain()).c_str(), memo);
    EXPECT_EQ(transaction->Outputs().at(0).Note(api_.Blockchain()), label_1_);
    EXPECT_TRUE(transaction->Outputs().at(1).Note(api_.Blockchain()).empty());
    ASSERT_TRUE(api_.Blockchain().AssignLabel(
        nym_1_id(),
        account_1_id(),
        Subchain::External,
        second_index_,
        label_2_));

    transaction = api_.Blockchain().LoadTransactionBitcoin(txid_);

    ASSERT_TRUE(transaction);
    EXPECT_STREQ(transaction->Memo(api_.Blockchain()).c_str(), memo);
    EXPECT_EQ(transaction->Outputs().at(0).Note(api_.Blockchain()), label_1_);
    EXPECT_EQ(transaction->Outputs().at(1).Note(api_.Blockchain()), label_2_);
    ASSERT_TRUE(api_.Blockchain().AssignTransactionMemo(txid_, ""));

    transaction = api_.Blockchain().LoadTransactionBitcoin(txid_);

    ASSERT_TRUE(transaction);
    // Memo is undefined
    EXPECT_FALSE(transaction->Memo(api_.Blockchain()).empty());
    EXPECT_EQ(transaction->Outputs().at(0).Note(api_.Blockchain()), label_1_);
    EXPECT_EQ(transaction->Outputs().at(1).Note(api_.Blockchain()), label_2_);
    ASSERT_TRUE(api_.Blockchain().AssignLabel(
        nym_1_id(), account_1_id(), Subchain::External, first_index_, ""));
    ASSERT_TRUE(api_.Blockchain().AssignLabel(
        nym_1_id(), account_1_id(), Subchain::External, second_index_, ""));

    transaction = api_.Blockchain().LoadTransactionBitcoin(txid_);

    ASSERT_TRUE(transaction);
    EXPECT_TRUE(transaction->Memo(api_.Blockchain()).empty());
}
}  // namespace ottest
