// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <boost/endian/buffers.hpp>
#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <set>
#include <string>
#include <tuple>
#include <vector>

#include "1_Internal.hpp"
#include "internal/blockchain/p2p/P2P.hpp"
#include "internal/blockchain/p2p/bitcoin/Bitcoin.hpp"
#include "opentxs/Bytes.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/BloomFilter.hpp"
#include "opentxs/blockchain/FilterType.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/blockchain/p2p/Types.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/iterator/Bidirectional.hpp"

namespace opentxs
{
namespace api
{
class Core;
}  // namespace api

namespace blockchain
{
namespace bitcoin
{
class Inventory;
}  // namespace bitcoin

namespace block
{
namespace bitcoin
{
class Header;
class Transaction;
}  // namespace bitcoin
}  // namespace block

namespace node
{
struct GCS;
}  // namespace node

namespace p2p
{
namespace bitcoin
{
namespace message
{
class Cmpctblock;
class Feefilter;
class Getblocks;
class Getblocktxn;
class Merkleblock;
class Reject;
class Sendcmpct;
class Tx;
}  // namespace message

class Header;
}  // namespace bitcoin
}  // namespace p2p
}  // namespace blockchain

namespace network
{
namespace zeromq
{
class Frame;
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs

namespace opentxs::blockchain::p2p::bitcoin
{
struct Message {
    static auto MaxPayload() -> std::size_t;

    virtual auto Encode() const -> OTData = 0;

    virtual auto header() const -> const Header& = 0;
    virtual auto payload() const noexcept -> OTData = 0;

    virtual ~Message() = default;
};
}  // namespace opentxs::blockchain::p2p::bitcoin

namespace opentxs::blockchain::p2p::bitcoin::message
{
using ClientFilterTypeField = be::little_uint8_buf_t;
using HashField = std::array<std::byte, 32>;
using InventoryTypeField = be::little_uint32_buf_t;

struct FilterPrefixBasic {
    ClientFilterTypeField type_;
    HashField hash_;

    auto Hash() const noexcept -> block::pHash;
    auto Type(const blockchain::Type chain) const noexcept -> filter::Type;

    FilterPrefixBasic(
        const blockchain::Type chain,
        const filter::Type type,
        const block::Hash& hash) noexcept(false);
    FilterPrefixBasic() noexcept;
};
struct FilterPrefixChained {
    ClientFilterTypeField type_;
    HashField hash_;
    HashField previous_;

    auto Previous() const noexcept -> filter::pHeader;
    auto Stop() const noexcept -> block::pHash;
    auto Type(const blockchain::Type chain) const noexcept -> filter::Type;

    FilterPrefixChained(
        const blockchain::Type chain,
        const filter::Type type,
        const block::Hash& stop,
        const filter::Header& prefix) noexcept(false);
    FilterPrefixChained() noexcept;
};
struct FilterRequest {
    ClientFilterTypeField type_;
    HeightField start_;
    HashField stop_;

    auto Start() const noexcept -> block::Height;
    auto Stop() const noexcept -> filter::pHash;
    auto Type(const blockchain::Type chain) const noexcept -> filter::Type;

    FilterRequest(
        const blockchain::Type chain,
        const filter::Type type,
        const block::Height start,
        const filter::Hash& stop) noexcept(false);
    FilterRequest() noexcept;
};

auto VerifyChecksum(
    const api::Core& api,
    const Header& header,
    const network::zeromq::Frame& payload) noexcept -> bool;
}  // namespace opentxs::blockchain::p2p::bitcoin::message

namespace opentxs::blockchain::p2p::bitcoin::message::internal
{
struct Addr : virtual public bitcoin::Message {
    using value_type = p2p::internal::Address;
    using const_iterator =
        iterator::Bidirectional<const Addr, const value_type>;

    virtual auto at(const std::size_t position) const noexcept(false)
        -> const value_type& = 0;
    virtual auto begin() const noexcept -> const_iterator = 0;
    virtual auto end() const noexcept -> const_iterator = 0;
    virtual auto size() const noexcept -> std::size_t = 0;

    ~Addr() override = default;
};
struct Block : virtual public bitcoin::Message {
    virtual auto GetBlock() const noexcept -> OTData = 0;

    ~Block() override = default;
};
struct Blocktxn : virtual public bitcoin::Message {
    virtual auto BlockTransactions() const noexcept -> OTData = 0;

    ~Blocktxn() override = default;
};
struct Cfcheckpt : virtual public bitcoin::Message {
    using value_type = filter::Hash;
    using const_iterator =
        iterator::Bidirectional<const Cfcheckpt, const value_type>;

    virtual auto at(const std::size_t position) const noexcept(false)
        -> const value_type& = 0;
    virtual auto begin() const noexcept -> const_iterator = 0;
    virtual auto end() const noexcept -> const_iterator = 0;
    virtual auto size() const noexcept -> std::size_t = 0;
    virtual auto Stop() const noexcept -> const value_type& = 0;
    virtual auto Type() const noexcept -> filter::Type = 0;

    ~Cfcheckpt() override = default;
};
struct Cfheaders : virtual public bitcoin::Message {
    using value_type = filter::Hash;
    using const_iterator =
        iterator::Bidirectional<const Cfheaders, const value_type>;

    virtual auto at(const std::size_t position) const noexcept(false)
        -> const value_type& = 0;
    virtual auto begin() const noexcept -> const_iterator = 0;
    virtual auto end() const noexcept -> const_iterator = 0;
    virtual auto Previous() const noexcept -> const filter::Header& = 0;
    virtual auto size() const noexcept -> std::size_t = 0;
    virtual auto Stop() const noexcept -> const block::Hash& = 0;
    virtual auto Type() const noexcept -> filter::Type = 0;

    ~Cfheaders() override = default;
};
struct Cfilter : virtual public bitcoin::Message {
    virtual auto Bits() const noexcept -> std::uint8_t = 0;
    virtual auto ElementCount() const noexcept -> std::uint32_t = 0;
    virtual auto FPRate() const noexcept -> std::uint32_t = 0;
    virtual auto Filter() const noexcept -> ReadView = 0;
    virtual auto Hash() const noexcept -> const block::Hash& = 0;
    virtual auto Type() const noexcept -> filter::Type = 0;

    ~Cfilter() override = default;
};
struct Filteradd : virtual public bitcoin::Message {
    virtual auto Element() const noexcept -> OTData = 0;

    ~Filteradd() override = default;
};
struct Filterclear : virtual public bitcoin::Message {

    ~Filterclear() override = default;
};
struct Filterload : virtual public bitcoin::Message {
    virtual auto Filter() const noexcept -> OTBloomFilter = 0;

    ~Filterload() override = default;
};
struct Getaddr : virtual public bitcoin::Message {

    ~Getaddr() override = default;
};
struct Getcfcheckpt : virtual public bitcoin::Message {
    virtual auto Stop() const noexcept -> const filter::Hash& = 0;
    virtual auto Type() const noexcept -> filter::Type = 0;

    ~Getcfcheckpt() override = default;
};
struct Getcfheaders : virtual public bitcoin::Message {
    virtual auto Start() const noexcept -> block::Height = 0;
    virtual auto Stop() const noexcept -> const filter::Hash& = 0;
    virtual auto Type() const noexcept -> filter::Type = 0;

    ~Getcfheaders() override = default;
};
struct Getcfilters : virtual public bitcoin::Message {
    virtual auto Start() const noexcept -> block::Height = 0;
    virtual auto Stop() const noexcept -> const block::Hash& = 0;
    virtual auto Type() const noexcept -> filter::Type = 0;

    ~Getcfilters() override = default;
};
struct Getdata : virtual public bitcoin::Message {
    using value_type = blockchain::bitcoin::Inventory;
    using const_iterator =
        iterator::Bidirectional<const Getdata, const value_type>;

    virtual auto at(const std::size_t position) const noexcept(false)
        -> const value_type& = 0;
    virtual auto begin() const noexcept -> const_iterator = 0;
    virtual auto end() const noexcept -> const_iterator = 0;
    virtual auto size() const noexcept -> std::size_t = 0;

    ~Getdata() override = default;
};
struct Getheaders : virtual public bitcoin::Message {
    using value_type = block::Hash;
    using const_iterator =
        iterator::Bidirectional<const Getheaders, const value_type>;

    virtual auto at(const std::size_t position) const noexcept(false)
        -> const value_type& = 0;
    virtual auto begin() const noexcept -> const_iterator = 0;
    virtual auto end() const noexcept -> const_iterator = 0;
    virtual auto size() const noexcept -> std::size_t = 0;
    virtual auto StopHash() const noexcept -> block::pHash = 0;
    virtual auto Version() const noexcept -> ProtocolVersionUnsigned = 0;

    ~Getheaders() override = default;
};
struct Headers : virtual public bitcoin::Message {
    using value_type = block::bitcoin::Header;
    using const_iterator =
        iterator::Bidirectional<const Headers, const value_type>;

    virtual auto at(const std::size_t position) const noexcept(false)
        -> const value_type& = 0;
    virtual auto begin() const noexcept -> const_iterator = 0;
    virtual auto end() const noexcept -> const_iterator = 0;
    virtual auto size() const noexcept -> std::size_t = 0;

    ~Headers() override = default;
};
struct Inv : virtual public bitcoin::Message {
    using value_type = blockchain::bitcoin::Inventory;
    using const_iterator = iterator::Bidirectional<const Inv, const value_type>;

    virtual auto at(const std::size_t position) const noexcept(false)
        -> const value_type& = 0;
    virtual auto begin() const noexcept -> const_iterator = 0;
    virtual auto end() const noexcept -> const_iterator = 0;
    virtual auto size() const noexcept -> std::size_t = 0;

    ~Inv() override = default;
};
struct Mempool : virtual public bitcoin::Message {

    ~Mempool() override = default;
};
struct Notfound : virtual public bitcoin::Message {
    using value_type = blockchain::bitcoin::Inventory;
    using const_iterator =
        iterator::Bidirectional<const Notfound, const value_type>;

    virtual auto at(const std::size_t position) const noexcept(false)
        -> const value_type& = 0;
    virtual auto begin() const noexcept -> const_iterator = 0;
    virtual auto end() const noexcept -> const_iterator = 0;
    virtual auto size() const noexcept -> std::size_t = 0;

    ~Notfound() override = default;
};
struct Ping : virtual public bitcoin::Message {
    virtual auto Nonce() const noexcept -> bitcoin::Nonce = 0;

    ~Ping() override = default;
};
struct Pong : virtual public bitcoin::Message {
    virtual auto Nonce() const noexcept -> bitcoin::Nonce = 0;

    ~Pong() override = default;
};
struct Sendheaders : virtual public bitcoin::Message {

    ~Sendheaders() override = default;
};
struct Tx : virtual public bitcoin::Message {
    virtual auto Transaction() const noexcept
        -> std::unique_ptr<const block::bitcoin::Transaction> = 0;

    ~Tx() override = default;
};
struct Verack : virtual public bitcoin::Message {

    ~Verack() override = default;
};
struct Version : virtual public bitcoin::Message {
    virtual auto Height() const noexcept -> block::Height = 0;
    virtual auto LocalAddress() const noexcept -> tcp::endpoint = 0;
    virtual auto LocalServices() const noexcept
        -> std::set<blockchain::p2p::Service> = 0;
    virtual auto Nonce() const noexcept -> p2p::bitcoin::Nonce = 0;
    virtual auto ProtocolVersion() const noexcept
        -> bitcoin::ProtocolVersion = 0;
    virtual auto Relay() const noexcept -> bool = 0;
    virtual auto RemoteAddress() const noexcept -> tcp::endpoint = 0;
    virtual auto RemoteServices() const noexcept
        -> std::set<blockchain::p2p::Service> = 0;
    virtual auto UserAgent() const noexcept -> const std::string& = 0;

    ~Version() override = default;
};
}  // namespace opentxs::blockchain::p2p::bitcoin::message::internal

namespace opentxs::factory
{
#if OT_BLOCKCHAIN
auto BitcoinP2PAddr(
    const api::Core& api,
    std::unique_ptr<blockchain::p2p::bitcoin::Header> pHeader,
    const blockchain::p2p::bitcoin::ProtocolVersion version,
    const void* payload,
    const std::size_t size)
    -> blockchain::p2p::bitcoin::message::internal::Addr*;
auto BitcoinP2PAddr(
    const api::Core& api,
    const blockchain::Type network,
    const blockchain::p2p::bitcoin::ProtocolVersion version,
    std::vector<std::unique_ptr<blockchain::p2p::internal::Address>>&&
        addresses) -> blockchain::p2p::bitcoin::message::internal::Addr*;
auto BitcoinP2PBlock(
    const api::Core& api,
    std::unique_ptr<blockchain::p2p::bitcoin::Header> header,
    const blockchain::p2p::bitcoin::ProtocolVersion version,
    const void* payload,
    const std::size_t size)
    -> blockchain::p2p::bitcoin::message::internal::Block*;
auto BitcoinP2PBlock(
    const api::Core& api,
    const blockchain::Type network,
    const Data& raw_block)
    -> blockchain::p2p::bitcoin::message::internal::Block*;
auto BitcoinP2PBlocktxn(
    const api::Core& api,
    std::unique_ptr<blockchain::p2p::bitcoin::Header> header,
    const blockchain::p2p::bitcoin::ProtocolVersion version,
    const void* payload,
    const std::size_t size)
    -> blockchain::p2p::bitcoin::message::internal::Blocktxn*;
auto BitcoinP2PBlocktxn(
    const api::Core& api,
    const blockchain::Type network,
    const Data& raw_Blocktxn)
    -> blockchain::p2p::bitcoin::message::internal::Blocktxn*;
auto BitcoinP2PCfcheckpt(
    const api::Core& api,
    std::unique_ptr<blockchain::p2p::bitcoin::Header> pHeader,
    const blockchain::p2p::bitcoin::ProtocolVersion version,
    const void* payload,
    const std::size_t size)
    -> blockchain::p2p::bitcoin::message::internal::Cfcheckpt*;
auto BitcoinP2PCfcheckpt(
    const api::Core& api,
    const blockchain::Type network,
    const blockchain::filter::Type type,
    const blockchain::filter::Hash& stop,
    const std::vector<blockchain::filter::pHash>& headers)
    -> blockchain::p2p::bitcoin::message::internal::Cfcheckpt*;
auto BitcoinP2PCfheaders(
    const api::Core& api,
    std::unique_ptr<blockchain::p2p::bitcoin::Header> pHeader,
    const blockchain::p2p::bitcoin::ProtocolVersion version,
    const void* payload,
    const std::size_t size)
    -> blockchain::p2p::bitcoin::message::internal::Cfheaders*;
auto BitcoinP2PCfheaders(
    const api::Core& api,
    const blockchain::Type network,
    const blockchain::filter::Type type,
    const blockchain::block::Hash& stop,
    const ReadView previousHeader,
    const std::vector<blockchain::filter::pHash>& headers)
    -> blockchain::p2p::bitcoin::message::internal::Cfheaders*;
auto BitcoinP2PCfilter(
    const api::Core& api,
    std::unique_ptr<blockchain::p2p::bitcoin::Header> pHeader,
    const blockchain::p2p::bitcoin::ProtocolVersion version,
    const void* payload,
    const std::size_t size)
    -> blockchain::p2p::bitcoin::message::internal::Cfilter*;
auto BitcoinP2PCfilter(
    const api::Core& api,
    const blockchain::Type network,
    const blockchain::filter::Type type,
    const blockchain::block::Hash& hash,
    const blockchain::node::GCS& filter)
    -> blockchain::p2p::bitcoin::message::internal::Cfilter*;
auto BitcoinP2PCmpctblock(
    const api::Core& api,
    std::unique_ptr<blockchain::p2p::bitcoin::Header> pHeader,
    const blockchain::p2p::bitcoin::ProtocolVersion version,
    const void* payload,
    const std::size_t size) -> blockchain::p2p::bitcoin::message::Cmpctblock*;
auto BitcoinP2PCmpctblock(
    const api::Core& api,
    const blockchain::Type network,
    const Data& raw_cmpctblock)
    -> blockchain::p2p::bitcoin::message::Cmpctblock*;
auto BitcoinP2PFeefilter(
    const api::Core& api,
    std::unique_ptr<blockchain::p2p::bitcoin::Header> pHeader,
    const blockchain::p2p::bitcoin::ProtocolVersion version,
    const void* payload,
    const std::size_t size) -> blockchain::p2p::bitcoin::message::Feefilter*;
auto BitcoinP2PFeefilter(
    const api::Core& api,
    const blockchain::Type network,
    const std::uint64_t fee_rate)
    -> blockchain::p2p::bitcoin::message::Feefilter*;
auto BitcoinP2PFilteradd(
    const api::Core& api,
    std::unique_ptr<blockchain::p2p::bitcoin::Header> pHeader,
    const blockchain::p2p::bitcoin::ProtocolVersion version,
    const void* payload,
    const std::size_t size)
    -> blockchain::p2p::bitcoin::message::internal::Filteradd*;
auto BitcoinP2PFilteradd(
    const api::Core& api,
    const blockchain::Type network,
    const Data& element)
    -> blockchain::p2p::bitcoin::message::internal::Filteradd*;
auto BitcoinP2PFilterclear(
    const api::Core& api,
    std::unique_ptr<blockchain::p2p::bitcoin::Header> header)
    -> blockchain::p2p::bitcoin::message::internal::Filterclear*;
auto BitcoinP2PFilterclear(const api::Core& api, const blockchain::Type network)
    -> blockchain::p2p::bitcoin::message::internal::Filterclear*;
auto BitcoinP2PFilterload(
    const api::Core& api,
    std::unique_ptr<blockchain::p2p::bitcoin::Header> pHeader,
    const blockchain::p2p::bitcoin::ProtocolVersion version,
    const void* payload,
    const std::size_t size)
    -> blockchain::p2p::bitcoin::message::internal::Filterload*;
auto BitcoinP2PFilterload(
    const api::Core& api,
    const blockchain::Type network,
    const blockchain::BloomFilter& filter)
    -> blockchain::p2p::bitcoin::message::internal::Filterload*;
auto BitcoinP2PGetaddr(
    const api::Core& api,
    std::unique_ptr<blockchain::p2p::bitcoin::Header> header)
    -> blockchain::p2p::bitcoin::message::internal::Getaddr*;
auto BitcoinP2PGetaddr(const api::Core& api, const blockchain::Type network)
    -> blockchain::p2p::bitcoin::message::internal::Getaddr*;
auto BitcoinP2PGetblocks(
    const api::Core& api,
    std::unique_ptr<blockchain::p2p::bitcoin::Header> header,
    const blockchain::p2p::bitcoin::ProtocolVersion version,
    const void* payload,
    const std::size_t size) -> blockchain::p2p::bitcoin::message::Getblocks*;
auto BitcoinP2PGetblocks(
    const api::Core& api,
    const blockchain::Type network,
    const std::uint32_t version,
    const std::vector<OTData>& header_hashes,
    const Data& stop_hash) -> blockchain::p2p::bitcoin::message::Getblocks*;
auto BitcoinP2PGetblocktxn(
    const api::Core& api,
    std::unique_ptr<blockchain::p2p::bitcoin::Header> pHeader,
    const blockchain::p2p::bitcoin::ProtocolVersion version,
    const void* payload,
    const std::size_t size) -> blockchain::p2p::bitcoin::message::Getblocktxn*;
auto BitcoinP2PGetblocktxn(
    const api::Core& api,
    const blockchain::Type network,
    const Data& block_hash,
    const std::vector<std::size_t>& txn_indices)
    -> blockchain::p2p::bitcoin::message::Getblocktxn*;
auto BitcoinP2PGetcfcheckpt(
    const api::Core& api,
    std::unique_ptr<blockchain::p2p::bitcoin::Header> pHeader,
    const blockchain::p2p::bitcoin::ProtocolVersion version,
    const void* payload,
    const std::size_t size)
    -> blockchain::p2p::bitcoin::message::internal::Getcfcheckpt*;
auto BitcoinP2PGetcfcheckpt(
    const api::Core& api,
    const blockchain::Type network,
    const blockchain::filter::Type type,
    const blockchain::filter::Hash& stop)
    -> blockchain::p2p::bitcoin::message::internal::Getcfcheckpt*;
auto BitcoinP2PGetcfheaders(
    const api::Core& api,
    std::unique_ptr<blockchain::p2p::bitcoin::Header> pHeader,
    const blockchain::p2p::bitcoin::ProtocolVersion version,
    const void* payload,
    const std::size_t size)
    -> blockchain::p2p::bitcoin::message::internal::Getcfheaders*;
auto BitcoinP2PGetcfheaders(
    const api::Core& api,
    const blockchain::Type network,
    const blockchain::filter::Type type,
    const blockchain::block::Height start,
    const blockchain::block::Hash& stop)
    -> blockchain::p2p::bitcoin::message::internal::Getcfheaders*;
auto BitcoinP2PGetcfilters(
    const api::Core& api,
    std::unique_ptr<blockchain::p2p::bitcoin::Header> pHeader,
    const blockchain::p2p::bitcoin::ProtocolVersion version,
    const void* payload,
    const std::size_t size)
    -> blockchain::p2p::bitcoin::message::internal::Getcfilters*;
auto BitcoinP2PGetcfilters(
    const api::Core& api,
    const blockchain::Type network,
    const blockchain::filter::Type type,
    const blockchain::block::Height start,
    const blockchain::block::Hash& stop)
    -> blockchain::p2p::bitcoin::message::internal::Getcfilters*;
auto BitcoinP2PGetdata(
    const api::Core& api,
    std::unique_ptr<blockchain::p2p::bitcoin::Header> pHeader,
    const blockchain::p2p::bitcoin::ProtocolVersion version,
    const void* payload,
    const std::size_t size)
    -> blockchain::p2p::bitcoin::message::internal::Getdata*;
auto BitcoinP2PGetdata(
    const api::Core& api,
    const blockchain::Type network,
    std::vector<blockchain::bitcoin::Inventory>&& payload)
    -> blockchain::p2p::bitcoin::message::internal::Getdata*;
auto BitcoinP2PGetheaders(
    const api::Core& api,
    std::unique_ptr<blockchain::p2p::bitcoin::Header> header,
    const blockchain::p2p::bitcoin::ProtocolVersion version,
    const void* payload,
    const std::size_t size)
    -> blockchain::p2p::bitcoin::message::internal::Getheaders*;
auto BitcoinP2PGetheaders(
    const api::Core& api,
    const blockchain::Type network,
    const blockchain::p2p::bitcoin::ProtocolVersionUnsigned version,
    std::vector<blockchain::block::pHash>&& history,
    blockchain::block::pHash&& stop)
    -> blockchain::p2p::bitcoin::message::internal::Getheaders*;
auto BitcoinP2PHeaders(
    const api::Core& api,
    std::unique_ptr<blockchain::p2p::bitcoin::Header> header,
    const blockchain::p2p::bitcoin::ProtocolVersion version,
    const void* payload,
    const std::size_t size)
    -> blockchain::p2p::bitcoin::message::internal::Headers*;
auto BitcoinP2PHeaders(
    const api::Core& api,
    const blockchain::Type network,
    std::vector<std::unique_ptr<blockchain::block::bitcoin::Header>>&& headers)
    -> blockchain::p2p::bitcoin::message::internal::Headers*;
auto BitcoinP2PInv(
    const api::Core& api,
    std::unique_ptr<blockchain::p2p::bitcoin::Header> pHeader,
    const blockchain::p2p::bitcoin::ProtocolVersion version,
    const void* payload,
    const std::size_t size)
    -> blockchain::p2p::bitcoin::message::internal::Inv*;
auto BitcoinP2PInv(
    const api::Core& api,
    const blockchain::Type network,
    std::vector<blockchain::bitcoin::Inventory>&& payload)
    -> blockchain::p2p::bitcoin::message::internal::Inv*;
auto BitcoinP2PMempool(
    const api::Core& api,
    std::unique_ptr<blockchain::p2p::bitcoin::Header> header)
    -> blockchain::p2p::bitcoin::message::internal::Mempool*;
auto BitcoinP2PMempool(const api::Core& api, const blockchain::Type network)
    -> blockchain::p2p::bitcoin::message::internal::Mempool*;
auto BitcoinP2PMerkleblock(
    const api::Core& api,
    std::unique_ptr<blockchain::p2p::bitcoin::Header> pHeader,
    const blockchain::p2p::bitcoin::ProtocolVersion version,
    const void* payload,
    const std::size_t size) -> blockchain::p2p::bitcoin::message::Merkleblock*;
auto BitcoinP2PMerkleblock(
    const api::Core& api,
    const blockchain::Type network,
    const Data& block_header,
    const std::uint32_t txn_count,
    const std::vector<OTData>& hashes,
    const std::vector<std::byte>& flags)
    -> blockchain::p2p::bitcoin::message::Merkleblock*;
auto BitcoinP2PNotfound(
    const api::Core& api,
    std::unique_ptr<blockchain::p2p::bitcoin::Header> pHeader,
    const blockchain::p2p::bitcoin::ProtocolVersion version,
    const void* payload,
    const std::size_t size)
    -> blockchain::p2p::bitcoin::message::internal::Notfound*;
auto BitcoinP2PNotfound(
    const api::Core& api,
    const blockchain::Type network,
    std::vector<blockchain::bitcoin::Inventory>&& payload)
    -> blockchain::p2p::bitcoin::message::internal::Notfound*;
auto BitcoinP2PPing(
    const api::Core& api,
    std::unique_ptr<blockchain::p2p::bitcoin::Header> header,
    const blockchain::p2p::bitcoin::ProtocolVersion version,
    const void* payload,
    const std::size_t size)
    -> blockchain::p2p::bitcoin::message::internal::Ping*;
auto BitcoinP2PPing(
    const api::Core& api,
    const blockchain::Type network,
    const std::uint64_t nonce)
    -> blockchain::p2p::bitcoin::message::internal::Ping*;
auto BitcoinP2PPong(
    const api::Core& api,
    std::unique_ptr<blockchain::p2p::bitcoin::Header> header,
    const blockchain::p2p::bitcoin::ProtocolVersion version,
    const void* payload,
    const std::size_t size)
    -> blockchain::p2p::bitcoin::message::internal::Pong*;
auto BitcoinP2PPong(
    const api::Core& api,
    const blockchain::Type network,
    const std::uint64_t nonce)
    -> blockchain::p2p::bitcoin::message::internal::Pong*;
auto BitcoinP2PReject(
    const api::Core& api,
    std::unique_ptr<blockchain::p2p::bitcoin::Header> pHeader,
    const blockchain::p2p::bitcoin::ProtocolVersion version,
    const void* payload,
    const std::size_t size) -> blockchain::p2p::bitcoin::message::Reject*;
auto BitcoinP2PReject(
    const api::Core& api,
    const blockchain::Type network,
    const std::string& message,
    const std::uint8_t code,
    const std::string& reason,
    const Data& extra) -> blockchain::p2p::bitcoin::message::Reject*;
auto BitcoinP2PSendcmpct(
    const api::Core& api,
    std::unique_ptr<blockchain::p2p::bitcoin::Header> header,
    const blockchain::p2p::bitcoin::ProtocolVersion version,
    const void* payload,
    const std::size_t size) -> blockchain::p2p::bitcoin::message::Sendcmpct*;
auto BitcoinP2PSendcmpct(
    const api::Core& api,
    const blockchain::Type network,
    const bool announce,
    const std::uint64_t version)
    -> blockchain::p2p::bitcoin::message::Sendcmpct*;
auto BitcoinP2PSendheaders(
    const api::Core& api,
    std::unique_ptr<blockchain::p2p::bitcoin::Header> header)
    -> blockchain::p2p::bitcoin::message::internal::Sendheaders*;
auto BitcoinP2PSendheaders(const api::Core& api, const blockchain::Type network)
    -> blockchain::p2p::bitcoin::message::internal::Sendheaders*;
auto BitcoinP2PTx(
    const api::Core& api,
    std::unique_ptr<blockchain::p2p::bitcoin::Header> header,
    const blockchain::p2p::bitcoin::ProtocolVersion version,
    const void* payload,
    const std::size_t size) noexcept
    -> std::unique_ptr<blockchain::p2p::bitcoin::message::internal::Tx>;
auto BitcoinP2PTx(
    const api::Core& api,
    const blockchain::Type network,
    const ReadView transaction) noexcept
    -> std::unique_ptr<blockchain::p2p::bitcoin::message::internal::Tx>;
auto BitcoinP2PVerack(
    const api::Core& api,
    std::unique_ptr<blockchain::p2p::bitcoin::Header> header)
    -> blockchain::p2p::bitcoin::message::internal::Verack*;
auto BitcoinP2PVerack(const api::Core& api, const blockchain::Type network)
    -> blockchain::p2p::bitcoin::message::internal::Verack*;
auto BitcoinP2PVersion(
    const api::Core& api,
    std::unique_ptr<blockchain::p2p::bitcoin::Header> header,
    const blockchain::p2p::bitcoin::ProtocolVersion version,
    const void* payload,
    const std::size_t size)
    -> blockchain::p2p::bitcoin::message::internal::Version*;
auto BitcoinP2PVersion(
    const api::Core& api,
    const blockchain::Type network,
    const blockchain::p2p::Network style,
    const std::int32_t version,
    const std::set<blockchain::p2p::Service>& localServices,
    const std::string& localAddress,
    const std::uint16_t localPort,
    const std::set<blockchain::p2p::Service>& remoteServices,
    const std::string& remoteAddress,
    const std::uint16_t remotePort,
    const std::uint64_t nonce,
    const std::string& userAgent,
    const blockchain::block::Height height,
    const bool relay) -> blockchain::p2p::bitcoin::message::internal::Version*;
#endif  // OT_BLOCKCHAIN
}  // namespace opentxs::factory
