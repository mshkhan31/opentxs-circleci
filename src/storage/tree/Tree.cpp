// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"           // IWYU pragma: associated
#include "1_Internal.hpp"         // IWYU pragma: associated
#include "storage/tree/Tree.hpp"  // IWYU pragma: associated

#include <functional>

#include "opentxs/api/storage/Driver.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"
#include "opentxs/protobuf/Check.hpp"
#include "opentxs/protobuf/Ciphertext.pb.h"
#include "opentxs/protobuf/StorageItems.pb.h"
#include "opentxs/protobuf/verify/StorageItems.hpp"
#include "storage/Plugin.hpp"
#include "storage/tree/Accounts.hpp"
#include "storage/tree/Contacts.hpp"
#include "storage/tree/Credentials.hpp"
#include "storage/tree/Node.hpp"
#include "storage/tree/Notary.hpp"
#include "storage/tree/Nym.hpp"  // IWYU pragma: keep
#include "storage/tree/Nyms.hpp"
#include "storage/tree/Seeds.hpp"
#include "storage/tree/Servers.hpp"
#include "storage/tree/Units.hpp"

#define TREE_VERSION 6

#define OT_METHOD "opentxs::storage::Tree::"

namespace opentxs::storage
{
Tree::Tree(
    const opentxs::api::storage::Driver& storage,
    const std::string& hash)
    : Node(storage, hash)
    , account_root_(Node::BLANK_HASH)
    , contact_root_(Node::BLANK_HASH)
    , credential_root_(Node::BLANK_HASH)
    , nym_root_(Node::BLANK_HASH)
    , seed_root_(Node::BLANK_HASH)
    , server_root_(Node::BLANK_HASH)
    , unit_root_(Node::BLANK_HASH)
    , account_lock_()
    , account_(nullptr)
    , contact_lock_()
    , contacts_(nullptr)
    , credential_lock_()
    , credentials_(nullptr)
    , notary_lock_()
    , notary_(nullptr)
    , nym_lock_()
    , nyms_(nullptr)
    , seed_lock_()
    , seeds_(nullptr)
    , server_lock_()
    , servers_(nullptr)
    , unit_lock_()
    , units_(nullptr)
    , master_key_lock_()
    , master_key_(nullptr)
{
    if (check_hash(hash)) {
        init(hash);
    } else {
        blank(TREE_VERSION);
    }
}

Tree::Tree(const Tree& rhs)
    : Node(rhs.driver_, "")
    , account_root_(Node::BLANK_HASH)
    , contact_root_(Node::BLANK_HASH)
    , credential_root_(Node::BLANK_HASH)
    , nym_root_(Node::BLANK_HASH)
    , seed_root_(Node::BLANK_HASH)
    , server_root_(Node::BLANK_HASH)
    , unit_root_(Node::BLANK_HASH)
    , account_lock_()
    , account_(nullptr)
    , contact_lock_()
    , contacts_(nullptr)
    , credential_lock_()
    , credentials_(nullptr)
    , notary_lock_()
    , notary_(nullptr)
    , nym_lock_()
    , nyms_(nullptr)
    , seed_lock_()
    , seeds_(nullptr)
    , server_lock_()
    , servers_(nullptr)
    , unit_lock_()
    , units_(nullptr)
    , master_key_lock_()
    , master_key_(nullptr)
{
    Lock lock(rhs.write_lock_);
    version_ = rhs.version_;
    root_ = rhs.root_;
    account_root_ = rhs.account_root_;
    contact_root_ = rhs.contact_root_;
    credential_root_ = rhs.credential_root_;
    notary_root_ = rhs.notary_root_;
    nym_root_ = rhs.nym_root_;
    seed_root_ = rhs.seed_root_;
    server_root_ = rhs.server_root_;
    unit_root_ = rhs.unit_root_;
    master_key_ = rhs.master_key_;
}

auto Tree::accounts() const -> storage::Accounts*
{
    return get_child<storage::Accounts>(account_lock_, account_, account_root_);
}

auto Tree::Accounts() const -> const storage::Accounts& { return *accounts(); }

auto Tree::Contacts() const -> const storage::Contacts& { return *contacts(); }

auto Tree::contacts() const -> storage::Contacts*
{
    return get_child<storage::Contacts>(
        contact_lock_, contacts_, contact_root_);
}

auto Tree::Credentials() const -> const storage::Credentials&
{
    return *credentials();
}

auto Tree::credentials() const -> storage::Credentials*
{
    return get_child<storage::Credentials>(
        credential_lock_, credentials_, credential_root_);
}

template <typename T, typename... Args>
auto Tree::get_child(
    std::mutex& mutex,
    std::unique_ptr<T>& pointer,
    const std::string& hash,
    Args&&... params) const -> T*
{
    Lock lock(mutex);

    if (false == bool(pointer)) {
        pointer.reset(new T(driver_, hash, params...));

        if (false == bool(pointer)) {
            LogOutput(OT_METHOD)(__func__)(": Unable to instantiate.").Flush();

            OT_FAIL;
        }
    }

    lock.unlock();

    return pointer.get();
}

template <typename T, typename... Args>
auto Tree::get_editor(
    std::mutex& mutex,
    std::unique_ptr<T>& pointer,
    std::string& hash,
    Args&&... params) const -> Editor<T>
{
    std::function<void(T*, Lock&)> callback = [&](T* in, Lock& lock) -> void {
        save_child<T>(in, lock, mutex, hash);
    };

    return Editor<T>(
        write_lock_, get_child<T>(mutex, pointer, hash, params...), callback);
}

void Tree::init(const std::string& hash)
{
    std::shared_ptr<proto::StorageItems> serialized{nullptr};
    driver_.LoadProto(hash, serialized);

    if (false == bool(serialized)) {
        LogOutput(OT_METHOD)(__func__)(": Failed to load root index file.")
            .Flush();

        OT_FAIL
    }

    init_version(TREE_VERSION, *serialized);
    account_root_ = normalize_hash(serialized->accounts());
    // NOTE blockchaintransactions field is no longer used
    contact_root_ = normalize_hash(serialized->contacts());
    credential_root_ = normalize_hash(serialized->creds());
    notary_root_ = normalize_hash(serialized->notary());
    nym_root_ = normalize_hash(serialized->nyms());
    seed_root_ = normalize_hash(serialized->seeds());
    server_root_ = normalize_hash(serialized->servers());
    unit_root_ = normalize_hash(serialized->units());

    if (serialized->has_master_secret()) {
        master_key_.reset(new proto::Ciphertext(serialized->master_secret()));

        OT_ASSERT(master_key_);
    }
}

auto Tree::Load(std::shared_ptr<proto::Ciphertext>& output, const bool checking)
    const -> bool
{
    Lock lock(master_key_lock_);

    const bool have = bool(master_key_);

    if (have) {
        output = master_key_;

        return true;
    } else {
        if (false == checking) {
            LogOutput(OT_METHOD)(__func__)(": Master key does not exist.")
                .Flush();
        }
    }

    return false;
}

auto Tree::Migrate(const opentxs::api::storage::Driver& to) const -> bool
{
    bool output{true};
    output &= accounts()->Migrate(to);
    output &= contacts()->Migrate(to);
    output &= credentials()->Migrate(to);
    output &= notary("")->Migrate(to);
    output &= nyms()->Migrate(to);
    output &= seeds()->Migrate(to);
    output &= servers()->Migrate(to);
    output &= units()->Migrate(to);
    output &= migrate(root_, to);

    return output;
}

auto Tree::mutable_Accounts() -> Editor<storage::Accounts>
{
    return get_editor<storage::Accounts>(
        account_lock_, account_, account_root_);
}

auto Tree::mutable_Contacts() -> Editor<storage::Contacts>
{
    return get_editor<storage::Contacts>(
        contact_lock_, contacts_, contact_root_);
}

auto Tree::mutable_Credentials() -> Editor<storage::Credentials>
{
    return get_editor<storage::Credentials>(
        credential_lock_, credentials_, credential_root_);
}

auto Tree::mutable_Notary(const std::string& id) -> Editor<storage::Notary>
{
    return get_editor<storage::Notary>(notary_lock_, notary_, notary_root_, id);
}

auto Tree::mutable_Nyms() -> Editor<storage::Nyms>
{
    return get_editor<storage::Nyms>(nym_lock_, nyms_, nym_root_);
}

auto Tree::mutable_Seeds() -> Editor<storage::Seeds>
{
    return get_editor<storage::Seeds>(seed_lock_, seeds_, seed_root_);
}

auto Tree::mutable_Servers() -> Editor<storage::Servers>
{
    return get_editor<storage::Servers>(server_lock_, servers_, server_root_);
}

auto Tree::mutable_Units() -> Editor<storage::Units>
{
    return get_editor<storage::Units>(unit_lock_, units_, unit_root_);
}

auto Tree::Notary(const std::string& id) const -> const storage::Notary&
{
    return *notary(id);
}

auto Tree::Nyms() const -> const storage::Nyms& { return *nyms(); }

auto Tree::notary(const std::string& id) const -> storage::Notary*
{
    return get_child<storage::Notary>(notary_lock_, notary_, notary_root_, id);
}

auto Tree::nyms() const -> storage::Nyms*
{
    return get_child<storage::Nyms>(nym_lock_, nyms_, nym_root_);
}

auto Tree::save(const Lock& lock) const -> bool
{
    if (!verify_write_lock(lock)) {
        LogOutput(OT_METHOD)(__func__)(": Lock failure.").Flush();
        OT_FAIL
    }

    auto serialized = serialize();

    if (!proto::Validate(serialized, VERBOSE)) { return false; }

    return driver_.StoreProto(serialized, root_);
}

template <typename T>
void Tree::save_child(
    T* input,
    const Lock& lock,
    std::mutex& hashLock,
    std::string& hash) const
{
    if (false == verify_write_lock(lock)) {
        LogOutput(OT_METHOD)(__func__)(": Lock failure.").Flush();
        OT_FAIL
    }

    if (nullptr == input) {
        LogOutput(OT_METHOD)(__func__)(": Null target.").Flush();
        OT_FAIL
    }

    Lock rootLock(hashLock);
    hash = input->Root();
    rootLock.unlock();

    if (false == save(lock)) {
        LogOutput(OT_METHOD)(__func__)(": Save error.").Flush();
        OT_FAIL
    }
}

auto Tree::Seeds() const -> const storage::Seeds& { return *seeds(); }

auto Tree::seeds() const -> storage::Seeds*
{
    return get_child<storage::Seeds>(seed_lock_, seeds_, seed_root_);
}

auto Tree::serialize() const -> proto::StorageItems
{
    proto::StorageItems serialized;
    serialized.set_version(version_);

    Lock accountLock(account_lock_);
    serialized.set_accounts(account_root_);
    accountLock.unlock();

    Lock contactLock(contact_lock_);
    serialized.set_contacts(contact_root_);
    contactLock.unlock();

    Lock credLock(credential_lock_);
    serialized.set_creds(credential_root_);
    credLock.unlock();

    Lock notaryLock(notary_lock_);
    serialized.set_notary(notary_root_);
    notaryLock.unlock();

    Lock nymLock(nym_lock_);
    serialized.set_nyms(nym_root_);
    nymLock.unlock();

    Lock seedLock(seed_lock_);
    serialized.set_seeds(seed_root_);
    seedLock.unlock();

    Lock serverLock(server_lock_);
    serialized.set_servers(server_root_);
    serverLock.unlock();

    Lock unitLock(unit_lock_);
    serialized.set_units(unit_root_);
    unitLock.unlock();

    Lock masterLock(master_key_lock_);

    if (master_key_) { *serialized.mutable_master_secret() = *master_key_; }

    masterLock.unlock();

    return serialized;
}

auto Tree::Servers() const -> const storage::Servers& { return *servers(); }

auto Tree::servers() const -> storage::Servers*
{
    return get_child<storage::Servers>(server_lock_, servers_, server_root_);
}

auto Tree::Store(const proto::Ciphertext& serialized) -> bool
{
    Lock masterLock(master_key_lock_, std::defer_lock);
    Lock writeLock(write_lock_, std::defer_lock);
    std::lock(masterLock, writeLock);
    master_key_ = std::make_shared<proto::Ciphertext>(serialized);
    masterLock.unlock();

    return save(writeLock);
}

auto Tree::Units() const -> const storage::Units& { return *units(); }

auto Tree::units() const -> storage::Units*
{
    return get_child<storage::Units>(unit_lock_, units_, unit_root_);
}

Tree::~Tree() = default;
}  // namespace opentxs::storage
