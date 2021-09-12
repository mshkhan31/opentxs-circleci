// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                        // IWYU pragma: associated
#include "1_Internal.hpp"                      // IWYU pragma: associated
#include "blockchain/crypto/AccountIndex.hpp"  // IWYU pragma: associated

#include <map>
#include <memory>
#include <shared_mutex>

#include "opentxs/Types.hpp"
#include "opentxs/api/Core.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/core/Identifier.hpp"

// #define OT_METHOD "opentxs::blockchain::crypto::AccountIndex::"

namespace opentxs::blockchain::crypto
{
struct AccountIndex::Imp {
    using Accounts = std::set<OTIdentifier>;

    auto AccountList(const identifier::Nym& nymID) const noexcept -> Accounts
    {
        auto lock = sLock{lock_};

        try {

            return nym_index_.at(nymID);
        } catch (...) {

            return {};
        }
    }
    auto AccountList(const Chain chain) const noexcept -> Accounts
    {
        auto lock = sLock{lock_};

        try {

            return chain_index_.at(chain);
        } catch (...) {

            return {};
        }
    }
    auto AccountList() const noexcept -> Accounts
    {
        auto lock = sLock{lock_};

        return all_;
    }
    auto Query(const Identifier& account) const noexcept -> Data
    {
        auto lock = sLock{lock_};

        try {

            return map_.at(account);
        } catch (...) {

            return blank_;
        }
    }
    auto Register(
        const Identifier& account,
        const identifier::Nym& owner,
        Chain chain) const noexcept -> void
    {
        auto lock = eLock{lock_};
        map_.try_emplace(account, chain, owner);
        chain_index_[chain].emplace(account);
        nym_index_[owner].emplace(account);
        all_.emplace(account);
    }

    Imp(const api::Core& api) noexcept
        : blank_(Chain::Unknown, api.Factory().NymID())
        , lock_()
        , map_()
        , chain_index_()
        , nym_index_()
        , all_()
    {
    }

private:
    using Map = std::map<OTIdentifier, Data>;
    using ChainIndex = std::map<Chain, Accounts>;
    using NymIndex = std::map<OTNymID, Accounts>;

    const Data blank_;
    mutable std::shared_mutex lock_;
    mutable Map map_;
    mutable ChainIndex chain_index_;
    mutable NymIndex nym_index_;
    mutable Accounts all_;

    Imp() = delete;
    Imp(const Imp&) = delete;
    Imp(Imp&&) = delete;
    auto operator=(const Imp&) -> Imp& = delete;
    auto operator=(Imp&&) -> Imp& = delete;
};

AccountIndex::AccountIndex(const api::Core& api) noexcept
    : imp_(std::make_unique<Imp>(api).release())
{
}

auto AccountIndex::AccountList(const identifier::Nym& nymID) const noexcept
    -> std::set<OTIdentifier>
{
    return imp_->AccountList(nymID);
}

auto AccountIndex::AccountList(const Chain chain) const noexcept
    -> std::set<OTIdentifier>
{
    return imp_->AccountList(chain);
}

auto AccountIndex::AccountList() const noexcept -> std::set<OTIdentifier>
{
    return imp_->AccountList();
}

auto AccountIndex::Query(const Identifier& account) const noexcept -> Data
{
    return imp_->Query(account);
}

auto AccountIndex::Register(
    const Identifier& account,
    const identifier::Nym& owner,
    Chain chain) const noexcept -> void
{
    imp_->Register(account, owner, chain);
}

AccountIndex::~AccountIndex()
{
    if (nullptr != imp_) {
        delete imp_;
        imp_ = nullptr;
    }
}
}  // namespace opentxs::blockchain::crypto
