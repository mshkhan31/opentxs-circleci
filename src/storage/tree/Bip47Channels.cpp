// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                    // IWYU pragma: associated
#include "1_Internal.hpp"                  // IWYU pragma: associated
#include "storage/tree/Bip47Channels.hpp"  // IWYU pragma: associated

#include <list>
#include <mutex>
#include <stdexcept>
#include <tuple>
#include <utility>

#include "internal/contact/Contact.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/storage/Driver.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"
#include "opentxs/protobuf/Bip47Channel.pb.h"
#include "opentxs/protobuf/BlockchainAccountData.pb.h"
#include "opentxs/protobuf/BlockchainDeterministicAccountData.pb.h"
#include "opentxs/protobuf/Check.hpp"
#include "opentxs/protobuf/StorageBip47ChannelList.pb.h"
#include "opentxs/protobuf/StorageBip47Contexts.pb.h"
#include "opentxs/protobuf/StorageItemHash.pb.h"
#include "opentxs/protobuf/verify/Bip47Channel.hpp"
#include "opentxs/protobuf/verify/StorageBip47Contexts.hpp"
#include "storage/Plugin.hpp"
#include "storage/tree/Node.hpp"

#define CHANNEL_VERSION 1
#define CHANNEL_INDEX_VERSION 1

#define OT_METHOD "opentxs::storage::Bip47Channels::"

namespace opentxs::storage
{
Bip47Channels::Bip47Channels(
    const opentxs::api::storage::Driver& storage,
    const std::string& hash)
    : Node(storage, hash)
    , index_lock_()
    , channel_data_()
    , chain_index_()
{
    if (check_hash(hash)) {
        init(hash);
    } else {
        blank(CHANNEL_VERSION);
    }
}

auto Bip47Channels::Chain(const Identifier& channelID) const
    -> contact::ContactItemType
{
    auto lock = sLock{index_lock_};

    return get_channel_data(lock, channelID);
}

auto Bip47Channels::ChannelsByChain(const contact::ContactItemType chain) const
    -> Bip47Channels::ChannelList
{
    return extract_set(chain, chain_index_);
}

auto Bip47Channels::Delete(const std::string& id) -> bool
{
    return delete_item(id);
}

template <typename I, typename V>
auto Bip47Channels::extract_set(const I& id, const V& index) const ->
    typename V::mapped_type
{
    auto lock = sLock{index_lock_};

    try {
        return index.at(id);

    } catch (...) {

        return {};
    }
}

template <typename L>
auto Bip47Channels::get_channel_data(const L& lock, const Identifier& id) const
    -> const Bip47Channels::ChannelData&
{
    try {

        return channel_data_.at(id);
    } catch (const std::out_of_range&) {
        static auto blank = ChannelData{contact::ContactItemType::Error};

        return blank;
    }
}

auto Bip47Channels::index(
    const eLock& lock,
    const Identifier& id,
    const proto::Bip47Channel& data) -> void
{
    const auto& common = data.deterministic().common();
    auto& chain = channel_data_[id];
    chain = contact::internal::translate(common.chain());
    chain_index_[chain].emplace(id);
}

auto Bip47Channels::init(const std::string& hash) -> void
{
    auto proto = std::shared_ptr<proto::StorageBip47Contexts>{};
    driver_.LoadProto(hash, proto);

    if (!proto) {
        LogOutput(OT_METHOD)(__func__)(
            ": Failed to load bip47 channel index file.")
            .Flush();
        OT_FAIL
    }

    init_version(CHANNEL_VERSION, *proto);

    for (const auto& it : proto->context()) {
        item_map_.emplace(
            it.itemid(), Metadata{it.hash(), it.alias(), 0, false});
    }

    if (proto->context().size() != proto->index().size()) {
        repair_indices();
    } else {
        for (const auto& index : proto->index()) {
            auto id = Identifier::Factory(index.channelid());
            auto& chain = channel_data_[id];
            chain = contact::internal::translate(index.chain());
            chain_index_[chain].emplace(std::move(id));
        }
    }
}

auto Bip47Channels::Load(
    const Identifier& id,
    std::shared_ptr<proto::Bip47Channel>& output,
    const bool checking) const -> bool
{
    std::string alias{""};

    return load_proto<proto::Bip47Channel>(id.str(), output, alias, checking);
}

auto Bip47Channels::repair_indices() noexcept -> void
{
    {
        auto lock = eLock{index_lock_};

        for (const auto& [strid, alias] : List()) {
            const auto id = Identifier::Factory(strid);
            auto data = std::shared_ptr<proto::Bip47Channel>{};
            const auto loaded = Load(id, data, false);

            OT_ASSERT(loaded);
            OT_ASSERT(data);

            index(lock, id, *data);
        }
    }

    auto lock = Lock{write_lock_};
    const auto saved = save(lock);

    OT_ASSERT(saved);
}

auto Bip47Channels::save(const std::unique_lock<std::mutex>& lock) const -> bool
{
    if (!verify_write_lock(lock)) {
        LogOutput(OT_METHOD)(__func__)(": Lock failure.").Flush();
        OT_FAIL
    }

    auto serialized = serialize();

    if (!proto::Validate(serialized, VERBOSE)) { return false; }

    return driver_.StoreProto(serialized, root_);
}

auto Bip47Channels::serialize() const -> proto::StorageBip47Contexts
{
    auto serialized = proto::StorageBip47Contexts{};
    serialized.set_version(version_);

    for (const auto& item : item_map_) {
        const bool goodID = !item.first.empty();
        const bool goodHash = check_hash(std::get<0>(item.second));
        const bool good = goodID && goodHash;

        if (good) {
            serialize_index(
                version_, item.first, item.second, *serialized.add_context());
        }
    }

    auto lock = sLock{index_lock_};

    for (const auto& [id, data] : channel_data_) {
        const auto& chain = data;
        auto& index = *serialized.add_index();
        index.set_version(CHANNEL_INDEX_VERSION);
        index.set_channelid(id->str());
        index.set_chain(contact::internal::translate(chain));
    }

    return serialized;
}

auto Bip47Channels::Store(const Identifier& id, const proto::Bip47Channel& data)
    -> bool
{
    {
        auto lock = eLock{index_lock_};
        index(lock, id, data);
    }

    return store_proto(data, id.str(), "");
}
}  // namespace opentxs::storage
