// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <boost/container/flat_set.hpp>
#include <algorithm>
#include <cstdint>
#include <iosfwd>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "Proto.hpp"
#include "internal/blockchain/Blockchain.hpp"
#include "internal/blockchain/crypto/Crypto.hpp"
#include "internal/blockchain/database/Database.hpp"
#include "internal/blockchain/node/Node.hpp"
#include "opentxs/Bytes.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/bitcoin/Input.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/network/blockchain/sync/Block.hpp"
#include "util/LMDB.hpp"

namespace opentxs
{
namespace api
{
class Core;
}  // namespace api

namespace blockchain
{
namespace database
{
namespace common
{
class Database;
}  // namespace common
}  // namespace database
}  // namespace blockchain

namespace network
{
namespace blockchain
{
namespace sync
{
class Block;
class Data;
}  // namespace sync
}  // namespace blockchain
}  // namespace network

namespace storage
{
namespace lmdb
{
class LMDB;
}  // namespace lmdb
}  // namespace storage
}  // namespace opentxs

namespace opentxs::blockchain::database
{
class Sync
{
public:
    using Items = std::vector<network::blockchain::sync::Block>;
    using Message = network::blockchain::sync::Data;

    auto Load(const block::Height height, Message& output) const noexcept
        -> bool;
    auto Reorg(const block::Height height) const noexcept -> bool;
    auto SetTip(const block::Position& position) const noexcept -> bool;
    auto Store(const block::Position& tip, const Items& items) const noexcept
        -> bool;
    auto Tip() const noexcept -> block::Position;

    Sync(
        const api::Core& api,
        const common::Database& common,
        const storage::lmdb::LMDB& lmdb,
        const blockchain::Type type) noexcept;

private:
    const api::Core& api_;
    const common::Database& common_;
    const storage::lmdb::LMDB& lmdb_;
    const block::Position blank_position_;
    const blockchain::Type chain_;
    const block::pHash genesis_;
};
}  // namespace opentxs::blockchain::database
