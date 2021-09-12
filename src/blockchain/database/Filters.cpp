// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                     // IWYU pragma: associated
#include "1_Internal.hpp"                   // IWYU pragma: associated
#include "blockchain/database/Filters.hpp"  // IWYU pragma: associated

#include <boost/container/flat_map.hpp>
#include <boost/container/vector.hpp>
#include <algorithm>
#include <cstddef>
#include <memory>
#include <type_traits>
#include <utility>

#include "blockchain/database/common/Database.hpp"
#include "internal/blockchain/Blockchain.hpp"
#include "internal/blockchain/Params.hpp"
#include "internal/blockchain/block/Block.hpp"
#include "internal/blockchain/database/Database.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/Core.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/block/Header.hpp"
#include "opentxs/blockchain/node/FilterOracle.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"
#include "util/LMDB.hpp"

#define OT_METHOD "opentxs::blockchain::database::Filters::"

namespace opentxs::blockchain::database
{
Filters::Filters(
    const api::Core& api,
    const common::Database& common,
    const storage::lmdb::LMDB& lmdb,
    const blockchain::Type chain) noexcept
    : api_(api)
    , common_(common)
    , lmdb_(lmdb)
    , blank_position_(make_blank<block::Position>::value(api))
    , lock_()
{
    import_genesis(chain);
}

auto Filters::CurrentHeaderTip(const filter::Type type) const noexcept
    -> block::Position
{
    auto output{blank_position_};
    auto cb = [this, &output](const auto in) {
        output = blockchain::internal::Deserialize(api_, in);
    };
    lmdb_.Load(
        Table::BlockFilterHeaderBest, static_cast<std::size_t>(type), cb);

    return output;
}

auto Filters::CurrentTip(const filter::Type type) const noexcept
    -> block::Position
{
    auto output{blank_position_};
    auto cb = [this, &output](const auto in) {
        output = blockchain::internal::Deserialize(api_, in);
    };
    lmdb_.Load(Table::BlockFilterBest, static_cast<std::size_t>(type), cb);

    return output;
}

auto Filters::HaveFilter(const filter::Type type, const block::Hash& block)
    const noexcept -> bool
{
    return common_.HaveFilter(type, block.Bytes());
}

auto Filters::HaveFilterHeader(
    const filter::Type type,
    const block::Hash& block) const noexcept -> bool
{
    return common_.HaveFilterHeader(type, block.Bytes());
}

auto Filters::import_genesis(const blockchain::Type chain) const noexcept
    -> void
{
    for (const auto& [style, genesis] : params::Data::Filters().at(chain)) {
        const auto needHeader =
            blank_position_.first == CurrentHeaderTip(style).first;
        const auto needFilter =
            blank_position_.first == CurrentTip(style).first;

        if (false == (needHeader || needFilter)) { return; }

        const auto pBlock = factory::GenesisBlockHeader(api_, chain);

        OT_ASSERT(pBlock);

        const auto& block = *pBlock;
        const auto& blockHash = block.Hash();
        const auto bytes =
            api_.Factory().Data(genesis.second, StringStyle::Hex);
        auto gcs = std::unique_ptr<const node::GCS>{factory::GCS(
            api_,
            style,
            blockchain::internal::BlockHashToFilterKey(blockHash.Bytes()),
            bytes->Bytes())};

        OT_ASSERT(gcs);

        const auto filterHash = gcs->Hash();
        auto success{false};

        if (needHeader) {
            auto header = api_.Factory().Data(genesis.first, StringStyle::Hex);
            auto headers = std::vector<node::internal::FilterDatabase::Header>{
                {blockHash, std::move(header), filterHash->Bytes()}};
            success = common_.StoreFilterHeaders(style, headers);

            OT_ASSERT(success);

            success = SetHeaderTip(style, block.Position());

            OT_ASSERT(success);
        }

        if (needFilter) {
            auto filters =
                std::vector<node::internal::FilterDatabase::Filter>{};
            filters.emplace_back(blockHash.Bytes(), std::move(gcs));

            success = common_.StoreFilters(style, filters);

            OT_ASSERT(success);

            success = SetTip(style, block.Position());

            OT_ASSERT(success);
        }
    }
}

auto Filters::LoadFilter(const filter::Type type, const ReadView block)
    const noexcept -> std::unique_ptr<const blockchain::node::GCS>
{
    return common_.LoadFilter(type, block);
}

auto Filters::LoadFilterHash(const filter::Type type, const ReadView block)
    const noexcept -> Hash
{
    auto output = api_.Factory().Data();

    if (common_.LoadFilterHash(type, block, output->WriteInto())) {

        return output;
    }

    return api_.Factory().Data();
}

auto Filters::LoadFilterHeader(const filter::Type type, const ReadView block)
    const noexcept -> Hash
{
    auto output = api_.Factory().Data();

    if (common_.LoadFilterHeader(type, block, output->WriteInto())) {

        return output;
    }

    return api_.Factory().Data();
}

auto Filters::SetHeaderTip(
    const filter::Type type,
    const block::Position& position) const noexcept -> bool
{
    return lmdb_
        .Store(
            Table::BlockFilterHeaderBest,
            static_cast<std::size_t>(type),
            reader(blockchain::internal::Serialize(position)))
        .first;
}

auto Filters::SetTip(const filter::Type type, const block::Position& position)
    const noexcept -> bool
{
    return lmdb_
        .Store(
            Table::BlockFilterBest,
            static_cast<std::size_t>(type),
            reader(blockchain::internal::Serialize(position)))
        .first;
}

auto Filters::StoreFilters(
    const filter::Type type,
    const std::vector<Header>& headers,
    const std::vector<Filter>& filters,
    const block::Position& tip) const noexcept -> bool
{
    auto output = common_.StoreFilters(type, headers, filters);

    if (false == output) {
        LogOutput(OT_METHOD)(__func__)(": Failed to save filters").Flush();

        return false;
    }

    if (0 > tip.first) { return true; }

    auto parentTxn = lmdb_.TransactionRW();
    output = lmdb_
                 .Store(
                     Table::BlockFilterHeaderBest,
                     static_cast<std::size_t>(type),
                     reader(blockchain::internal::Serialize(tip)),
                     parentTxn)
                 .first;

    if (false == output) {
        LogOutput(OT_METHOD)(__func__)(": Failed to set header tip").Flush();

        return false;
    }

    output = lmdb_
                 .Store(
                     Table::BlockFilterBest,
                     static_cast<std::size_t>(type),
                     reader(blockchain::internal::Serialize(tip)),
                     parentTxn)
                 .first;

    if (false == output) {
        LogOutput(OT_METHOD)(__func__)(": Failed to set filter tip").Flush();

        return false;
    }

    return parentTxn.Finalize(true);
}

auto Filters::StoreFilters(const filter::Type type, std::vector<Filter> filters)
    const noexcept -> bool
{
    return common_.StoreFilters(type, filters);
}

auto Filters::StoreHeaders(
    const filter::Type type,
    const ReadView previous,
    const std::vector<Header> headers) const noexcept -> bool
{
    return common_.StoreFilterHeaders(type, headers);
}
}  // namespace opentxs::blockchain::database
