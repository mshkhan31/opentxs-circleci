// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <gtest/gtest.h>
#include <cstddef>
#include <iosfwd>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <string_view>
#include <tuple>
#include <utility>
#include <vector>

#include "Basic.hpp"
#include "opentxs/OT.hpp"
#include "opentxs/api/Context.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/api/client/Blockchain.hpp"
#include "opentxs/api/client/Manager.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/Header.hpp"
#include "opentxs/blockchain/block/bitcoin/Header.hpp"
#include "opentxs/blockchain/node/HeaderOracle.hpp"
#include "opentxs/blockchain/node/Manager.hpp"

namespace opentxs
{
namespace api
{
namespace client
{
class Manager;
}  // namespace client
}  // namespace api

namespace blockchain
{
namespace node
{
class HeaderOracle;
class Manager;
}  // namespace node
}  // namespace blockchain
}  // namespace opentxs

namespace b = ot::blockchain;
namespace bb = b::block;
namespace bc = b::node;

#define BLOCK_1 "block 01_XXXXXXXXXXXXXXXXXXXXXXX"
#define BLOCK_2 "block 02_XXXXXXXXXXXXXXXXXXXXXXX"
#define BLOCK_3 "block 03_XXXXXXXXXXXXXXXXXXXXXXX"
#define BLOCK_4 "block 04_XXXXXXXXXXXXXXXXXXXXXXX"
#define BLOCK_5 "block 05_XXXXXXXXXXXXXXXXXXXXXXX"
#define BLOCK_6 "block 06_XXXXXXXXXXXXXXXXXXXXXXX"
#define BLOCK_7 "block 07_XXXXXXXXXXXXXXXXXXXXXXX"
#define BLOCK_8 "block 08_XXXXXXXXXXXXXXXXXXXXXXX"
#define BLOCK_9 "block 09_XXXXXXXXXXXXXXXXXXXXXXX"
#define BLOCK_10 "block 10_XXXXXXXXXXXXXXXXXXXXXXX"
#define BLOCK_11 "block 11_XXXXXXXXXXXXXXXXXXXXXXX"
#define BLOCK_12 "block 12_XXXXXXXXXXXXXXXXXXXXXXX"
#define BLOCK_13 "block 13_XXXXXXXXXXXXXXXXXXXXXXX"
#define BLOCK_14 "block 14_XXXXXXXXXXXXXXXXXXXXXXX"

namespace ottest
{
class Test_HeaderOracle_base : public ::testing::Test
{
public:
    using Block = std::pair<std::string, std::string>;
    using Position = std::pair<bb::Height, std::string>;
    using BestChainVector = std::vector<std::string>;
    using Test = std::tuple<std::string, Position, BestChainVector>;
    using Status = bb::Header::Status;
    using HeaderData =
        std::tuple<std::string, std::string, bb::Height, Status, Status>;
    using PostStateVector = std::vector<HeaderData>;
    using ExpectedSiblings = std::set<std::string>;

    static const std::vector<Block> create_1_;
    static const std::vector<Test> sequence_1_;
    static const PostStateVector post_state_1_;
    static const BestChainVector best_chain_1_;
    static const ExpectedSiblings siblings_1_;
    static const std::vector<Block> create_2_;
    static const std::vector<Test> sequence_2_;
    static const PostStateVector post_state_2_;
    static const BestChainVector best_chain_2_;
    static const ExpectedSiblings siblings_2_;
    static const std::vector<Block> create_3_;
    static const std::vector<Test> sequence_3_;
    static const PostStateVector post_state_3_;
    static const BestChainVector best_chain_3_;
    static const ExpectedSiblings siblings_3_;
    static const std::vector<Block> create_4_;
    static const std::vector<Test> sequence_4_;
    static const PostStateVector post_state_4_;
    static const BestChainVector best_chain_4_;
    static const ExpectedSiblings siblings_4_;
    static const std::vector<Block> create_5_;
    static const std::vector<Test> sequence_5_;
    static const PostStateVector post_state_5_;
    static const BestChainVector best_chain_5_;
    static const ExpectedSiblings siblings_5_;
    static const std::vector<Block> create_6_;
    static const std::vector<Test> sequence_6_;
    static const PostStateVector post_state_6_;
    static const BestChainVector best_chain_6_;
    static const ExpectedSiblings siblings_6_;
    static const std::vector<Block> create_7_;
    static const std::vector<Test> sequence_7_;
    static const PostStateVector post_state_7_;
    static const BestChainVector best_chain_7_;
    static const ExpectedSiblings siblings_7_;
    static const std::vector<Block> create_8_;
    static const std::vector<Test> sequence_8_;
    static const PostStateVector post_state_8a_;
    static const PostStateVector post_state_8b_;
    static const BestChainVector best_chain_8a_;
    static const BestChainVector best_chain_8b_;
    static const ExpectedSiblings siblings_8a_;
    static const ExpectedSiblings siblings_8b_;
    static const std::vector<Block> create_9_;
    static const std::vector<Test> sequence_9_;
    static const PostStateVector post_state_9_;
    static const BestChainVector best_chain_9_;
    static const ExpectedSiblings siblings_9_;
    static const std::vector<Block> create_10_;
    static const std::vector<Test> sequence_10_;
    static const std::vector<std::string> bitcoin_;

    const ot::api::client::Manager& api_;
    const b::Type type_;
    std::unique_ptr<bc::Manager> network_;
    bc::HeaderOracle& header_oracle_;
    std::map<std::string, std::unique_ptr<bb::Header>> test_blocks_;

    static auto init_network(
        const ot::api::client::Manager& api,
        const b::Type type) noexcept -> std::unique_ptr<bc::Manager>;

    auto apply_blocks(const std::vector<Test>& vector) -> bool;
    auto apply_blocks_batch(const std::vector<Test>& vector) -> bool;
    auto create_blocks(const std::vector<Block>& vector) -> bool;
    auto get_block_hash(const std::string& hash) -> bb::pHash;
    auto get_test_block(const std::string& hash) -> std::unique_ptr<bb::Header>;
    auto make_position(const bb::Height height, const std::string_view hash)
        -> bb::Position;
    auto make_test_block(const std::string& hash, const bb::Hash& parent)
        -> bool;
    auto verify_best_chain(const BestChainVector& vector) -> bool;
    auto verify_hash_sequence(
        const bb::Height start,
        const std::size_t stop,
        const std::vector<std::string>& expected) -> bool;
    auto verify_post_state(const PostStateVector& vector) -> bool;
    auto verify_siblings(const ExpectedSiblings& vector) -> bool;

    Test_HeaderOracle_base(const b::Type type);

    ~Test_HeaderOracle_base() override;
};

struct Test_HeaderOracle_btc : public Test_HeaderOracle_base {
    Test_HeaderOracle_btc();

    ~Test_HeaderOracle_btc() override;
};

struct Test_HeaderOracle : public Test_HeaderOracle_base {
    Test_HeaderOracle();

    ~Test_HeaderOracle() override;
};
}  // namespace ottest
