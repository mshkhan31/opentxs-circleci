// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <boost/asio.hpp>
#include <boost/thread/thread.hpp>
#include <cstddef>
#include <cstdint>
#include <future>
#include <iosfwd>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <set>
#include <shared_mutex>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "1_Internal.hpp"
#if OT_BLOCKCHAIN
#include "blockchain/DownloadTask.hpp"
#include "internal/blockchain/database/Database.hpp"
#endif  // OT_BLOCKCHAIN
#include "internal/core/Core.hpp"
#include "opentxs/Bytes.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#if OT_BLOCKCHAIN
#include "opentxs/blockchain/node/BlockOracle.hpp"
#include "opentxs/blockchain/node/FilterOracle.hpp"
#include "opentxs/blockchain/node/HeaderOracle.hpp"
#include "opentxs/blockchain/node/Manager.hpp"
#include "opentxs/blockchain/node/Wallet.hpp"
#endif  // OT_BLOCKCHAIN
#include "opentxs/blockchain/p2p/Types.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/network/zeromq/ListenCallback.hpp"
#include "opentxs/network/zeromq/Message.hpp"
#include "opentxs/network/zeromq/socket/Router.hpp"
#include "opentxs/util/WorkType.hpp"
#include "util/Blank.hpp"
#include "util/Work.hpp"

namespace opentxs
{
namespace api
{
namespace client
{
namespace blockchain
{
namespace database
{
namespace implementation
{
class Database;
}  // namespace implementation
}  // namespace database
}  // namespace blockchain

namespace internal
{
struct Blockchain;
}  // namespace internal
}  // namespace client

class Core;
}  // namespace api

namespace blockchain
{
namespace block
{
namespace bitcoin
{
namespace internal
{
struct Transaction;
}  // namespace internal

class Block;
class Header;
class Transaction;
}  // namespace bitcoin

class Block;
class Header;
struct Outpoint;
}  // namespace block

namespace internal
{
struct Database;
}  // namespace internal

namespace node
{
class UpdateTransaction;
}  // namespace node

namespace p2p
{
namespace internal
{
struct Address;
}  // namespace internal

class Address;
}  // namespace p2p
}  // namespace blockchain

namespace identifier
{
class Nym;
}  // namespace identifier

namespace network
{
namespace asio
{
class Socket;
}  // namespace asio

namespace blockchain
{
namespace sync
{
class Block;
class Data;
}  // namespace sync
}  // namespace blockchain

namespace zeromq
{
class Frame;
class Message;

namespace socket
{
class Publish;
}  // namespace socket
}  // namespace zeromq
}  // namespace network

namespace proto
{
class BlockchainTransactionOutput;
class BlockchainTransactionProposal;
}  // namespace proto

template <typename T>
struct make_blank;
}  // namespace opentxs

namespace zmq = opentxs::network::zeromq;

#if OT_BLOCKCHAIN
namespace opentxs
{
template <>
struct make_blank<blockchain::block::Height> {
    static auto value(const api::Core&) -> blockchain::block::Height
    {
        return -1;
    }
};
template <>
struct make_blank<blockchain::block::Position> {
    static auto value(const api::Core& api) -> blockchain::block::Position
    {
        return {
            make_blank<blockchain::block::Height>::value(api),
            blockchain::block::BlankHash()};
    }
};
}  // namespace opentxs

namespace opentxs::blockchain::node
{
// parent hash, child hash
using ChainSegment = std::pair<block::pHash, block::pHash>;
using UpdatedHeader =
    std::map<block::pHash, std::pair<std::unique_ptr<block::Header>, bool>>;
using BestHashes = std::map<block::Height, block::pHash>;
using Hashes = std::set<block::pHash>;
using HashVector = std::vector<block::pHash>;
using Segments = std::set<ChainSegment>;
// parent block hash, disconnected block hash
using DisconnectedList = std::multimap<block::pHash, block::pHash>;

using CfheaderJob =
    download::Batch<filter::pHash, filter::pHeader, filter::Type>;
using CfilterJob = download::
    Batch<std::unique_ptr<const node::GCS>, filter::pHeader, filter::Type>;
using BlockJob =
    download::Batch<std::shared_ptr<const block::bitcoin::Block>, int>;
}  // namespace opentxs::blockchain::node
#endif  // OT_BLOCKCHAIN

namespace opentxs::blockchain::node::internal
{
#if OT_BLOCKCHAIN
struct BlockDatabase {
    virtual auto BlockExists(const block::Hash& block) const noexcept
        -> bool = 0;
    virtual auto BlockLoadBitcoin(const block::Hash& block) const noexcept
        -> std::shared_ptr<const block::bitcoin::Block> = 0;
    virtual auto BlockPolicy() const noexcept -> database::BlockStorage = 0;
    virtual auto BlockStore(const block::Block& block) const noexcept
        -> bool = 0;
    virtual auto BlockTip() const noexcept -> block::Position = 0;
    virtual auto SetBlockTip(const block::Position& position) const noexcept
        -> bool = 0;

    virtual ~BlockDatabase() = default;
};

struct BlockOracle : virtual public node::BlockOracle {
    enum class Task : OTZMQWorkType {
        ProcessBlock = OT_ZMQ_INTERNAL_SIGNAL + 0,
        StateMachine = OT_ZMQ_STATE_MACHINE_SIGNAL,
        Shutdown = value(WorkType::Shutdown),
    };

    virtual auto GetBlockJob() const noexcept -> BlockJob = 0;
    virtual auto Heartbeat() const noexcept -> void = 0;
    virtual auto SubmitBlock(const ReadView in) const noexcept -> void = 0;

    virtual auto Init() noexcept -> void = 0;
    virtual auto Shutdown() noexcept -> std::shared_future<void> = 0;

    ~BlockOracle() override = default;
};

struct BlockValidator {
    using BitcoinBlock = block::bitcoin::Block;

    virtual auto Validate(const BitcoinBlock& block) const noexcept -> bool
    {
        return true;
    }

    virtual ~BlockValidator() = default;
};

struct Config {
    bool download_cfilters_{false};
    bool generate_cfilters_{false};
    bool provide_sync_server_{false};
    bool use_sync_server_{false};
    bool disable_wallet_{false};

    auto print() const noexcept -> std::string;
};

struct FilterDatabase {
    using Hash = block::pHash;
    /// block hash, filter header, filter hash
    using Header = std::tuple<block::pHash, filter::pHeader, ReadView>;
    /// block hash, filter
    using Filter = std::pair<ReadView, std::unique_ptr<const node::GCS>>;

    virtual auto FilterHeaderTip(const filter::Type type) const noexcept
        -> block::Position = 0;
    virtual auto FilterTip(const filter::Type type) const noexcept
        -> block::Position = 0;
    virtual auto HaveFilter(const filter::Type type, const block::Hash& block)
        const noexcept -> bool = 0;
    virtual auto HaveFilterHeader(
        const filter::Type type,
        const block::Hash& block) const noexcept -> bool = 0;
    virtual auto LoadFilter(const filter::Type type, const ReadView block)
        const noexcept -> std::unique_ptr<const node::GCS> = 0;
    virtual auto LoadFilterHash(const filter::Type type, const ReadView block)
        const noexcept -> Hash = 0;
    virtual auto LoadFilterHeader(const filter::Type type, const ReadView block)
        const noexcept -> Hash = 0;
    virtual auto SetFilterHeaderTip(
        const filter::Type type,
        const block::Position& position) const noexcept -> bool = 0;
    virtual auto SetFilterTip(
        const filter::Type type,
        const block::Position& position) const noexcept -> bool = 0;
    virtual auto StoreFilters(
        const filter::Type type,
        std::vector<Filter> filters) const noexcept -> bool = 0;
    virtual auto StoreFilters(
        const filter::Type type,
        const std::vector<Header>& headers,
        const std::vector<Filter>& filters,
        const block::Position& tip) const noexcept -> bool = 0;
    virtual auto StoreFilterHeaders(
        const filter::Type type,
        const ReadView previous,
        const std::vector<Header> headers) const noexcept -> bool = 0;

    virtual ~FilterDatabase() = default;
};

struct FilterOracle : virtual public node::FilterOracle {
    using Header = FilterDatabase::Hash;

    virtual auto GetFilterJob() const noexcept -> CfilterJob = 0;
    virtual auto GetHeaderJob() const noexcept -> CfheaderJob = 0;
    virtual auto Heartbeat() const noexcept -> void = 0;
    virtual auto LoadFilterOrResetTip(
        const filter::Type type,
        const block::Position& position) const noexcept
        -> std::unique_ptr<const node::GCS> = 0;
    virtual auto ProcessBlock(const block::bitcoin::Block& block) const noexcept
        -> bool = 0;
    virtual auto ProcessSyncData(
        const block::Hash& prior,
        const std::vector<block::pHash>& hashes,
        const network::blockchain::sync::Data& data) const noexcept -> void = 0;
    virtual auto Tip(const filter::Type type) const noexcept
        -> block::Position = 0;

    virtual auto Start() noexcept -> void = 0;
    virtual auto Shutdown() noexcept -> void = 0;

    ~FilterOracle() override = default;
};

struct HeaderOracle : virtual public node::HeaderOracle {
    using CheckpointBlockHash = block::pHash;
    using PreviousBlockHash = block::pHash;
    using CheckpointFilterHash = block::pHash;
    using CheckpointData = std::tuple<
        block::Height,
        CheckpointBlockHash,
        PreviousBlockHash,
        CheckpointFilterHash>;

    virtual auto GetDefaultCheckpoint() const noexcept -> CheckpointData = 0;
    virtual auto Init() noexcept -> void = 0;
    virtual auto LoadBitcoinHeader(const block::Hash& hash) const noexcept
        -> std::unique_ptr<block::bitcoin::Header> = 0;
    virtual auto ProcessSyncData(
        block::Hash& prior,
        std::vector<block::pHash>& hashes,
        const network::blockchain::sync::Data& data) noexcept
        -> std::size_t = 0;

    ~HeaderOracle() override = default;
};

struct HeaderDatabase {
    virtual auto ApplyUpdate(const UpdateTransaction& update) const noexcept
        -> bool = 0;
    // Throws std::out_of_range if no block at that position
    virtual auto BestBlock(const block::Height position) const noexcept(false)
        -> block::pHash = 0;
    virtual auto CurrentBest() const noexcept
        -> std::unique_ptr<block::Header> = 0;
    virtual auto CurrentCheckpoint() const noexcept -> block::Position = 0;
    virtual auto DisconnectedHashes() const noexcept -> DisconnectedList = 0;
    virtual auto HasDisconnectedChildren(const block::Hash& hash) const noexcept
        -> bool = 0;
    virtual auto HaveCheckpoint() const noexcept -> bool = 0;
    virtual auto HeaderExists(const block::Hash& hash) const noexcept
        -> bool = 0;
    virtual auto IsSibling(const block::Hash& hash) const noexcept -> bool = 0;
    // Throws std::out_of_range if the header does not exist
    virtual auto LoadHeader(const block::Hash& hash) const noexcept(false)
        -> std::unique_ptr<block::Header> = 0;
    virtual auto RecentHashes() const noexcept -> HashVector = 0;
    virtual auto SiblingHashes() const noexcept -> Hashes = 0;
    // Returns null pointer if the header does not exist
    virtual auto TryLoadBitcoinHeader(const block::Hash& hash) const noexcept
        -> std::unique_ptr<block::bitcoin::Header> = 0;
    // Returns null pointer if the header does not exist
    virtual auto TryLoadHeader(const block::Hash& hash) const noexcept
        -> std::unique_ptr<block::Header> = 0;

    virtual ~HeaderDatabase() = default;
};

struct Mempool {
    virtual auto Dump() const noexcept -> std::set<std::string> = 0;
    virtual auto Query(ReadView txid) const noexcept
        -> std::shared_ptr<const block::bitcoin::Transaction> = 0;
    virtual auto Submit(ReadView txid) const noexcept -> bool = 0;
    virtual auto Submit(const std::vector<ReadView>& txids) const noexcept
        -> std::vector<bool> = 0;
    virtual auto Submit(std::unique_ptr<const block::bitcoin::Transaction> tx)
        const noexcept -> void = 0;

    virtual auto Heartbeat() noexcept -> void = 0;

    virtual ~Mempool() = default;
};

struct PeerDatabase {
    using Address = std::unique_ptr<p2p::internal::Address>;
    using Protocol = p2p::Protocol;
    using Service = p2p::Service;
    using Type = p2p::Network;

    virtual auto AddOrUpdate(Address address) const noexcept -> bool = 0;
    virtual auto Get(
        const Protocol protocol,
        const std::set<Type> onNetworks,
        const std::set<Service> withServices) const noexcept -> Address = 0;
    virtual auto Import(std::vector<Address> peers) const noexcept -> bool = 0;

    virtual ~PeerDatabase() = default;
};

struct PeerManager {
    enum class Task : OTZMQWorkType {
        Shutdown = value(WorkType::Shutdown),
        Mempool = value(WorkType::BlockchainMempoolUpdated),
        Register = value(WorkType::AsioRegister),
        Connect = value(WorkType::AsioConnect),
        Disconnect = value(WorkType::AsioDisconnect),
        Getheaders = OT_ZMQ_INTERNAL_SIGNAL + 0,
        Getblock = OT_ZMQ_INTERNAL_SIGNAL + 1,
        BroadcastTransaction = OT_ZMQ_INTERNAL_SIGNAL + 2,
        BroadcastBlock = OT_ZMQ_INTERNAL_SIGNAL + 3,
        JobAvailableCfheaders = OT_ZMQ_INTERNAL_SIGNAL + 4,
        JobAvailableCfilters = OT_ZMQ_INTERNAL_SIGNAL + 5,
        JobAvailableBlock = OT_ZMQ_INTERNAL_SIGNAL + 6,
        Body = OT_ZMQ_INTERNAL_SIGNAL + 126,
        Header = OT_ZMQ_INTERNAL_SIGNAL + 127,
        Heartbeat = OT_ZMQ_HEARTBEAT_SIGNAL,
        ReceiveMessage = OT_ZMQ_RECEIVE_SIGNAL,
        SendMessage = OT_ZMQ_SEND_SIGNAL,
        StateMachine = OT_ZMQ_STATE_MACHINE_SIGNAL,
    };

    virtual auto AddIncomingPeer(const int id, std::uintptr_t endpoint)
        const noexcept -> void = 0;
    virtual auto AddPeer(const p2p::Address& address) const noexcept
        -> bool = 0;
    virtual auto BroadcastBlock(const block::Block& block) const noexcept
        -> bool = 0;
    virtual auto BroadcastTransaction(
        const block::bitcoin::Transaction& tx) const noexcept -> bool = 0;
    virtual auto Connect() noexcept -> bool = 0;
    virtual auto Database() const noexcept -> const PeerDatabase& = 0;
    virtual auto Disconnect(const int id) const noexcept -> void = 0;
    virtual auto Endpoint(const Task type) const noexcept -> std::string = 0;
    virtual auto GetPeerCount() const noexcept -> std::size_t = 0;
    virtual auto GetVerifiedPeerCount() const noexcept -> std::size_t = 0;
    virtual auto Heartbeat() const noexcept -> void = 0;
    virtual auto JobReady(const Task type) const noexcept -> void = 0;
    virtual auto Listen(const p2p::Address& address) const noexcept -> bool = 0;
    virtual auto LookupIncomingSocket(const int id) const noexcept(false)
        -> opentxs::network::asio::Socket = 0;
    virtual auto RequestBlock(const block::Hash& block) const noexcept
        -> bool = 0;
    virtual auto RequestBlocks(
        const std::vector<ReadView>& hashes) const noexcept -> bool = 0;
    virtual auto RequestHeaders() const noexcept -> bool = 0;
    virtual auto VerifyPeer(const int id, const std::string& address)
        const noexcept -> void = 0;

    virtual auto init() noexcept -> void = 0;
    virtual auto Shutdown() noexcept -> std::shared_future<void> = 0;

    virtual ~PeerManager() = default;
};

struct Network : virtual public node::Manager {
    enum class Task : OTZMQWorkType {
        Shutdown = value(WorkType::Shutdown),
        SyncReply = value(WorkType::SyncReply),
        SyncNewBlock = value(WorkType::NewBlock),
        SubmitBlockHeader = OT_ZMQ_INTERNAL_SIGNAL + 0,
        SubmitBlock = OT_ZMQ_INTERNAL_SIGNAL + 2,
        Heartbeat = OT_ZMQ_INTERNAL_SIGNAL + 3,
        SendToAddress = OT_ZMQ_INTERNAL_SIGNAL + 4,
        SendToPaymentCode = OT_ZMQ_INTERNAL_SIGNAL + 5,
        FilterUpdate = OT_ZMQ_NEW_FILTER_SIGNAL,
        StateMachine = OT_ZMQ_STATE_MACHINE_SIGNAL,
    };

    virtual auto BroadcastTransaction(
        const block::bitcoin::Transaction& tx) const noexcept -> bool = 0;
    virtual auto Chain() const noexcept -> Type = 0;
    // amount represents satoshis per 1000 bytes
    virtual auto FeeRate() const noexcept -> Amount = 0;
    auto FilterOracle() const noexcept -> const node::FilterOracle& final
    {
        return FilterOracleInternal();
    }
    virtual auto FilterOracleInternal() const noexcept
        -> const internal::FilterOracle& = 0;
    auto HeaderOracle() const noexcept -> const node::HeaderOracle& final
    {
        return HeaderOracleInternal();
    }
    virtual auto HeaderOracleInternal() const noexcept
        -> const internal::HeaderOracle& = 0;
    virtual auto Heartbeat() const noexcept -> void = 0;
    virtual auto IsSynchronized() const noexcept -> bool = 0;
    virtual auto JobReady(const PeerManager::Task type) const noexcept
        -> void = 0;
    virtual auto Mempool() const noexcept -> const internal::Mempool& = 0;
    virtual auto Reorg() const noexcept
        -> const network::zeromq::socket::Publish& = 0;
    virtual auto RequestBlock(const block::Hash& block) const noexcept
        -> bool = 0;
    virtual auto RequestBlocks(
        const std::vector<ReadView>& hashes) const noexcept -> bool = 0;
    virtual auto Submit(network::zeromq::Message& work) const noexcept
        -> void = 0;
    virtual auto Track(network::zeromq::Message& work) const noexcept
        -> std::future<void> = 0;
    virtual auto UpdateHeight(const block::Height height) const noexcept
        -> void = 0;
    virtual auto UpdateLocalHeight(
        const block::Position position) const noexcept -> void = 0;

    virtual auto HeaderOracleInternal() noexcept -> internal::HeaderOracle& = 0;
    virtual auto FilterOracleInternal() noexcept -> internal::FilterOracle& = 0;
    virtual auto Shutdown() noexcept -> std::shared_future<void> = 0;

    ~Network() override = default;
};

struct SyncDatabase {
    using Height = block::Height;
    using Items = std::vector<network::blockchain::sync::Block>;
    using Message = network::blockchain::sync::Data;

    virtual auto LoadSync(const Height height, Message& output) const noexcept
        -> bool = 0;
    virtual auto ReorgSync(const Height height) const noexcept -> bool = 0;
    virtual auto SetSyncTip(const block::Position& position) const noexcept
        -> bool = 0;
    virtual auto StoreSync(const block::Position& tip, const Items& items)
        const noexcept -> bool = 0;
    virtual auto SyncTip() const noexcept -> block::Position = 0;

    virtual ~SyncDatabase() = default;
};

struct Wallet : virtual public node::Wallet {
    enum class Task : OTZMQWorkType {
        index = OT_ZMQ_INTERNAL_SIGNAL + 0,
        scan = OT_ZMQ_INTERNAL_SIGNAL + 1,
        process = OT_ZMQ_INTERNAL_SIGNAL + 2,
        reorg = OT_ZMQ_INTERNAL_SIGNAL + 3,
    };

    virtual auto ConstructTransaction(
        const proto::BlockchainTransactionProposal& tx,
        std::promise<SendOutcome>&& promise) const noexcept -> void = 0;

    virtual auto Init() noexcept -> void = 0;
    virtual auto Shutdown() noexcept -> std::shared_future<void> = 0;

    ~Wallet() override = default;
};

struct WalletDatabase {
    using FilterType = filter::Type;
    using NodeID = Identifier;
    using pNodeID = OTIdentifier;
    using SubchainIndex = Identifier;
    using pSubchainIndex = OTIdentifier;
    using Subchain = blockchain::crypto::Subchain;
    using SubchainID = std::pair<Subchain, pNodeID>;
    using ElementID = std::pair<Bip32Index, SubchainID>;
    using ElementMap = std::map<Bip32Index, std::vector<Space>>;
    using Pattern = std::pair<ElementID, Space>;
    using Patterns = std::vector<Pattern>;
    using MatchingIndices = std::vector<Bip32Index>;
    using UTXO = std::
        pair<blockchain::block::Outpoint, proto::BlockchainTransactionOutput>;
    using KeyID = blockchain::crypto::Key;
    using State = node::Wallet::TxoState;

    enum class Spend : bool {
        ConfirmedOnly = false,
        UnconfirmedToo = true,
    };

    virtual auto AddConfirmedTransaction(
        const NodeID& balanceNode,
        const Subchain subchain,
        const block::Position& block,
        const std::size_t blockIndex,
        const std::vector<std::uint32_t> outputIndices,
        const block::bitcoin::Transaction& transaction) const noexcept
        -> bool = 0;
    virtual auto AddMempoolTransaction(
        const NodeID& balanceNode,
        const Subchain subchain,
        const std::vector<std::uint32_t> outputIndices,
        const block::bitcoin::Transaction& transaction) const noexcept
        -> bool = 0;
    virtual auto AddOutgoingTransaction(
        const Identifier& proposalID,
        const proto::BlockchainTransactionProposal& proposal,
        const block::bitcoin::Transaction& transaction) const noexcept
        -> bool = 0;
    virtual auto AddProposal(
        const Identifier& id,
        const proto::BlockchainTransactionProposal& tx) const noexcept
        -> bool = 0;
    virtual auto CancelProposal(const Identifier& id) const noexcept
        -> bool = 0;
    virtual auto CompletedProposals() const noexcept
        -> std::set<OTIdentifier> = 0;
    virtual auto ForgetProposals(
        const std::set<OTIdentifier>& ids) const noexcept -> bool = 0;
    virtual auto GetBalance() const noexcept -> Balance = 0;
    virtual auto GetBalance(const identifier::Nym& owner) const noexcept
        -> Balance = 0;
    virtual auto GetBalance(const identifier::Nym& owner, const NodeID& node)
        const noexcept -> Balance = 0;
    virtual auto GetIndex(
        const NodeID& balanceNode,
        const Subchain subchain,
        const FilterType type) const noexcept -> pSubchainIndex = 0;
    virtual auto GetOutputs(State type) const noexcept -> std::vector<UTXO> = 0;
    virtual auto GetOutputs(const identifier::Nym& owner, State type)
        const noexcept -> std::vector<UTXO> = 0;
    virtual auto GetOutputs(
        const identifier::Nym& owner,
        const Identifier& node,
        State type) const noexcept -> std::vector<UTXO> = 0;
    virtual auto GetPatterns(const SubchainIndex& index) const noexcept
        -> Patterns = 0;
    virtual auto GetUnspentOutputs() const noexcept -> std::vector<UTXO> = 0;
    virtual auto GetUnspentOutputs(
        const NodeID& balanceNode,
        const Subchain subchain) const noexcept -> std::vector<UTXO> = 0;
    virtual auto GetUntestedPatterns(
        const SubchainIndex& index,
        const ReadView blockID) const noexcept -> Patterns = 0;
    virtual auto LoadProposal(const Identifier& id) const noexcept
        -> std::optional<proto::BlockchainTransactionProposal> = 0;
    virtual auto LoadProposals() const noexcept
        -> std::vector<proto::BlockchainTransactionProposal> = 0;
    virtual auto LookupContact(const Data& pubkeyHash) const noexcept
        -> std::set<OTIdentifier> = 0;
    virtual auto ReorgTo(
        const NodeID& balanceNode,
        const Subchain subchain,
        const SubchainIndex& index,
        const std::vector<block::Position>& reorg) const noexcept -> bool = 0;
    virtual auto ReserveUTXO(
        const identifier::Nym& spender,
        const Identifier& proposal,
        const Spend policy) const noexcept -> std::optional<UTXO> = 0;
    virtual auto SetDefaultFilterType(const FilterType type) const noexcept
        -> bool = 0;
    virtual auto SubchainAddElements(
        const SubchainIndex& index,
        const ElementMap& elements) const noexcept -> bool = 0;
    virtual auto SubchainLastIndexed(const SubchainIndex& index) const noexcept
        -> std::optional<Bip32Index> = 0;
    virtual auto SubchainLastScanned(const SubchainIndex& index) const noexcept
        -> block::Position = 0;
    virtual auto SubchainMatchBlock(
        const SubchainIndex& index,
        const MatchingIndices& indices,
        const ReadView blockID) const noexcept -> bool = 0;
    virtual auto SubchainSetLastScanned(
        const SubchainIndex& index,
        const block::Position& position) const noexcept -> bool = 0;
    virtual auto TransactionLoadBitcoin(const ReadView txid) const noexcept
        -> std::unique_ptr<block::bitcoin::Transaction> = 0;

    virtual ~WalletDatabase() = default;
};
#endif  // OT_BLOCKCHAIN
}  // namespace opentxs::blockchain::node::internal
