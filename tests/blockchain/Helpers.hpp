// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/network/zeromq/Message.hpp"

#pragma once

#include <boost/container/flat_map.hpp>
#include <boost/intrusive/detail/iterator.hpp>
#include <cstddef>
#include <functional>
#include <iosfwd>
#include <memory>
#include <string>

#include "opentxs/Version.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/FilterType.hpp"

namespace opentxs
{
namespace api
{
class Core;
}  // namespace api

namespace network
{
namespace zeromq
{
class Message;
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs

namespace ot = opentxs;

namespace ottest
{
struct FilterVector {
    std::string filter_hex_{};
    std::string header_hex_{};
};
struct ChainVector {
    std::string genesis_block_hex_{};
    boost::container::flat_map<ot::blockchain::filter::Type, FilterVector>
        filters_{};
};
struct Listener {
    using Message = ot::network::zeromq::Message;

    auto get(std::size_t index) noexcept(false) -> const Message&;

    Listener(const ot::api::Core& api, const std::string& endpoint);

    ~Listener();

private:
    struct Imp;

    std::unique_ptr<Imp> imp_;
};

constexpr auto blank_hash_{
    "0x0000000000000000000000000000000000000000000000000000000000000000"};
constexpr auto btc_genesis_hash_numeric_{
    "000000000019d6689c085ae165831e934ff763ae46a2a6c172b3f1b60a8ce26f"};
constexpr auto btc_genesis_hash_{
    "0x6fe28c0ab6f1b372c1a6a246ae63f74f931e8365e15a089c68d6190000000000"};
constexpr auto ltc_genesis_hash_numeric_{
    "0000050c34a64b415b6b15b37f2216634b5b1669cb9a2e38d76f7213b0671e00"};
constexpr auto ltc_genesis_hash_{
    "0xe2bf047e7e5a191aa4ef34d314979dc9986e0f19251edaba5940fd1fe365a712"};

extern const boost::container::flat_map<ot::blockchain::Type, ChainVector>
    genesis_block_data_;
}  // namespace ottest
