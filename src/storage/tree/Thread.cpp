// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"             // IWYU pragma: associated
#include "1_Internal.hpp"           // IWYU pragma: associated
#include "storage/tree/Thread.hpp"  // IWYU pragma: associated

#include <memory>
#include <utility>

#include "opentxs/api/storage/Driver.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"
#include "opentxs/protobuf/Check.hpp"
#include "opentxs/protobuf/StorageThread.pb.h"
#include "opentxs/protobuf/StorageThreadItem.pb.h"
#include "opentxs/protobuf/verify/StorageThread.hpp"
#include "opentxs/protobuf/verify/StorageThreadItem.hpp"
#include "storage/Plugin.hpp"
#include "storage/tree/Mailbox.hpp"
#include "storage/tree/Node.hpp"

#define OT_METHOD "opentxs::storage::Thread::"

namespace opentxs
{
namespace storage
{
Thread::Thread(
    const opentxs::api::storage::Driver& storage,
    const std::string& id,
    const std::string& hash,
    const std::string& alias,
    Mailbox& mailInbox,
    Mailbox& mailOutbox)
    : Node(storage, hash)
    , id_(id)
    , alias_(alias)
    , index_(0)
    , mail_inbox_(mailInbox)
    , mail_outbox_(mailOutbox)
    , items_()
    , participants_()
{
    if (check_hash(hash)) {
        init(hash);
    } else {
        blank(1);
    }
}

Thread::Thread(
    const opentxs::api::storage::Driver& storage,
    const std::string& id,
    const std::set<std::string>& participants,
    Mailbox& mailInbox,
    Mailbox& mailOutbox)
    : Node(storage, Node::BLANK_HASH)
    , id_(id)
    , alias_()
    , index_(0)
    , mail_inbox_(mailInbox)
    , mail_outbox_(mailOutbox)
    , items_()
    , participants_(participants)
{
    blank(1);
}

auto Thread::Add(
    const std::string& id,
    const std::uint64_t time,
    const StorageBox& box,
    const std::string& alias,
    const std::string& contents,
    const std::uint64_t index,
    const std::string& account,
    const std::uint32_t chain) -> bool
{
    Lock lock(write_lock_);

    auto saved{true};
    auto unread{true};

    switch (box) {
        case StorageBox::MAILINBOX: {
            saved = mail_inbox_.Store(id, contents, alias);
        } break;
        case StorageBox::MAILOUTBOX: {
            saved = mail_outbox_.Store(id, contents, alias);
            unread = false;
        } break;
        case StorageBox::OUTGOINGCHEQUE:
        case StorageBox::OUTGOINGTRANSFER:
        case StorageBox::INTERNALTRANSFER: {
            unread = false;
        } break;
        case StorageBox::BLOCKCHAIN:
        case StorageBox::INCOMINGCHEQUE:
        case StorageBox::INCOMINGTRANSFER: {
        } break;
        default: {
            LogOutput(OT_METHOD)(__func__)(": Warning: unknown box.").Flush();
        }
    }

    if (false == saved) {
        LogOutput(OT_METHOD)(__func__)(": Unable to save item.").Flush();

        return false;
    }

    auto& item = items_[id];
    item.set_version(version_);
    item.set_id(id);

    if (0 == index) {
        item.set_index(index_++);
    } else {
        item.set_index(index);
    }

    item.set_time(time);
    item.set_box(static_cast<std::uint32_t>(box));
    item.set_account(account);
    item.set_unread(unread);

    if (StorageBox::BLOCKCHAIN == box) {
        item.set_chain(chain);
        item.set_txid(contents);
    }

    const auto valid = proto::Validate(item, VERBOSE);

    if (false == valid) {
        items_.erase(id);

        return false;
    }

    return save(lock);
}

auto Thread::Alias() const -> std::string
{
    Lock lock(write_lock_);

    return alias_;
}

void Thread::init(const std::string& hash)
{
    std::shared_ptr<proto::StorageThread> serialized;
    driver_.LoadProto(hash, serialized);

    if (false == bool(serialized)) {
        LogOutput(OT_METHOD)(__func__)(": Failed to load thread index file.")
            .Flush();
        OT_FAIL;
    }

    init_version(1, *serialized);

    for (const auto& participant : serialized->participant()) {
        participants_.emplace(participant);
    }

    for (const auto& it : serialized->item()) {
        const auto& index = it.index();
        items_.emplace(it.id(), it);

        if (index >= index_) { index_ = index + 1; }
    }

    Lock lock(write_lock_);
    upgrade(lock);
}

auto Thread::Check(const std::string& id) const -> bool
{
    Lock lock(write_lock_);

    return items_.end() != items_.find(id);
}

auto Thread::ID() const -> std::string { return id_; }

auto Thread::Items() const -> proto::StorageThread
{
    Lock lock(write_lock_);

    return serialize(lock);
}

auto Thread::Migrate(const opentxs::api::storage::Driver& to) const -> bool
{
    return Node::migrate(root_, to);
}

auto Thread::Read(const std::string& id, const bool unread) -> bool
{
    Lock lock(write_lock_);

    auto it = items_.find(id);

    if (items_.end() == it) {
        LogOutput(OT_METHOD)(__func__)(": Item does not exist.").Flush();

        return false;
    }

    auto& item = it->second;

    item.set_unread(unread);

    return save(lock);
}

auto Thread::Remove(const std::string& id) -> bool
{
    Lock lock(write_lock_);

    auto it = items_.find(id);

    if (items_.end() == it) { return false; }

    auto& item = it->second;
    auto box = static_cast<StorageBox>(item.box());
    items_.erase(it);

    switch (box) {
        case StorageBox::MAILINBOX: {
            mail_inbox_.Delete(id);
        } break;
        case StorageBox::MAILOUTBOX: {
            mail_outbox_.Delete(id);
        } break;
        case StorageBox::BLOCKCHAIN: {
        } break;
        default: {
            LogOutput(OT_METHOD)(__func__)(": Warning: unknown box.").Flush();
        }
    }

    return save(lock);
}

auto Thread::Rename(const std::string& newID) -> bool
{
    Lock lock(write_lock_);
    const auto oldID = id_;
    id_ = newID;

    if (0 != participants_.count(oldID)) {
        participants_.erase(oldID);
        participants_.emplace(newID);
    }

    return save(lock);
}

auto Thread::save(const Lock& lock) const -> bool
{
    OT_ASSERT(verify_write_lock(lock));

    auto serialized = serialize(lock);

    if (!proto::Validate(serialized, VERBOSE)) { return false; }

    return driver_.StoreProto(serialized, root_);
}

auto Thread::serialize(const Lock& lock) const -> proto::StorageThread
{
    OT_ASSERT(verify_write_lock(lock));

    proto::StorageThread serialized;
    serialized.set_version(version_);
    serialized.set_id(id_);

    for (const auto& nym : participants_) {
        if (!nym.empty()) { *serialized.add_participant() = nym; }
    }

    auto sorted = sort(lock);

    for (const auto& it : sorted) {
        OT_ASSERT(nullptr != it.second);

        const auto& item = *it.second;
        *serialized.add_item() = item;
    }

    return serialized;
}

auto Thread::SetAlias(const std::string& alias) -> bool
{
    Lock lock(write_lock_);

    alias_ = alias;

    return true;
}

auto Thread::sort(const Lock& lock) const -> Thread::SortedItems
{
    OT_ASSERT(verify_write_lock(lock));

    SortedItems output;

    for (const auto& it : items_) {
        const auto& id = it.first;
        const auto& item = it.second;

        if (!id.empty()) {
            SortKey key{item.index(), item.time(), id};
            output.emplace(key, &item);
        }
    }

    return output;
}

auto Thread::UnreadCount() const -> std::size_t
{
    Lock lock(write_lock_);
    std::size_t output{0};

    for (const auto& it : items_) {
        const auto& item = it.second;

        if (item.unread()) { ++output; }
    }

    return output;
}

void Thread::upgrade(const Lock& lock)
{
    OT_ASSERT(verify_write_lock(lock));

    bool changed{false};

    for (auto& it : items_) {
        auto& item = it.second;
        const auto box = static_cast<StorageBox>(item.box());

        switch (box) {
            case StorageBox::MAILOUTBOX: {
                if (item.unread()) {
                    item.set_unread(false);
                    changed = true;
                }
            } break;
            default: {
            }
        }
    }

    if (changed) { save(lock); }
}
}  // namespace storage
}  // namespace opentxs
