// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"              // IWYU pragma: associated
#include "1_Internal.hpp"            // IWYU pragma: associated
#include "api/client/ui/Imp-qt.hpp"  // IWYU pragma: associated

#include <map>
#include <memory>
#include <tuple>
#include <utility>

#include "internal/api/client/Client.hpp"  // IWYU pragma: keep
#include "internal/ui/UI.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/ui/Blockchains.hpp"
#include "opentxs/ui/qt/BlockchainSelection.hpp"
#include "opentxs/ui/qt/BlockchainStatistics.hpp"
#include "opentxs/ui/qt/SeedValidator.hpp"

//#define OT_METHOD "opentxs::api::client::UI"

namespace opentxs::api::client::ui
{
ImpQt::ImpQt(
    const api::client::Manager& api,
    const api::client::internal::Blockchain& blockchain,
    const Flag& running) noexcept
    : Imp(api, blockchain, running)
    , blank_()
    , accounts_qt_()
    , account_lists_qt_()
    , account_summaries_qt_()
    , activity_summaries_qt_()
    , activity_threads_qt_()
    , blockchain_account_status_qt_()
    , blockchain_selection_qt_()
    , blockchain_statistics_qt_()
    , contact_lists_qt_()
    , contacts_qt_()
    , messagable_lists_qt_()
    , payable_lists_qt_()
    , profiles_qt_()
    , seed_validators_()
    , unit_lists_qt_()
{
    // WARNING: do not access api_.Wallet() during construction
}

auto ImpQt::Blank::get(const std::size_t columns) noexcept
    -> opentxs::ui::BlankModel*
{
    auto lock = Lock{lock_};

    {
        auto it = map_.find(columns);

        if (map_.end() != it) { return &(it->second); }
    }

    return &(map_.emplace(
                     std::piecewise_construct,
                     std::forward_as_tuple(columns),
                     std::forward_as_tuple(columns))
                 .first->second);
}

auto ImpQt::AccountActivityQt(
    const identifier::Nym& nymID,
    const Identifier& accountID,
    const SimpleCallback cb) const noexcept -> opentxs::ui::AccountActivityQt*
{
    auto lock = Lock{lock_};
    auto key = AccountActivityKey{nymID, accountID};
    auto it = accounts_qt_.find(key);

    if (accounts_qt_.end() == it) {
        auto& native = account_activity(lock, nymID, accountID, cb);
        it = accounts_qt_
                 .emplace(
                     std::move(key),
                     opentxs::factory::AccountActivityQtModel(*native))
                 .first;

        OT_ASSERT(it->second);
    }

    return it->second.get();
}

auto ImpQt::AccountListQt(const identifier::Nym& nymID, const SimpleCallback cb)
    const noexcept -> opentxs::ui::AccountListQt*
{
    auto lock = Lock{lock_};
    auto key = AccountListKey{nymID};
    auto it = account_lists_qt_.find(key);

    if (account_lists_qt_.end() == it) {
        auto& native = account_list(lock, nymID, cb);
        it = account_lists_qt_
                 .emplace(
                     std::move(key),
                     opentxs::factory::AccountListQtModel(*native))
                 .first;

        OT_ASSERT(it->second);
    }

    return it->second.get();
}

auto ImpQt::AccountSummaryQt(
    const identifier::Nym& nymID,
    const contact::ContactItemType currency,
    const SimpleCallback cb) const noexcept -> opentxs::ui::AccountSummaryQt*
{
    auto lock = Lock{lock_};
    auto key = AccountSummaryKey{nymID, currency};
    auto it = account_summaries_qt_.find(key);

    if (account_summaries_qt_.end() == it) {
        auto& native = account_summary(lock, nymID, currency, cb);
        it = account_summaries_qt_
                 .emplace(
                     std::move(key),
                     opentxs::factory::AccountSummaryQtModel(*native))
                 .first;

        OT_ASSERT(it->second);
    }

    return it->second.get();
}

auto ImpQt::ActivitySummaryQt(
    const identifier::Nym& nymID,
    const SimpleCallback cb) const noexcept -> opentxs::ui::ActivitySummaryQt*
{
    auto lock = Lock{lock_};
    auto key = ActivitySummaryKey{nymID};
    auto it = activity_summaries_qt_.find(key);

    if (activity_summaries_qt_.end() == it) {
        auto& native = activity_summary(lock, nymID, cb);
        it = activity_summaries_qt_
                 .emplace(
                     std::move(key),
                     opentxs::factory::ActivitySummaryQtModel(*native))
                 .first;

        OT_ASSERT(it->second);
    }

    return it->second.get();
}

auto ImpQt::ActivityThreadQt(
    const identifier::Nym& nymID,
    const Identifier& threadID,
    const SimpleCallback cb) const noexcept -> opentxs::ui::ActivityThreadQt*
{
    auto lock = Lock{lock_};
    auto key = ActivityThreadKey{nymID, threadID};
    auto it = activity_threads_qt_.find(key);

    if (activity_threads_qt_.end() == it) {
        auto& native = activity_thread(lock, nymID, threadID, cb);
        it = activity_threads_qt_
                 .emplace(
                     std::move(key),
                     opentxs::factory::ActivityThreadQtModel(*native))
                 .first;

        OT_ASSERT(it->second);
    }

    return it->second.get();
}

auto ImpQt::BlankModel(const std::size_t columns) const noexcept
    -> QAbstractItemModel*
{
    return blank_.get(columns);
}

auto ImpQt::BlockchainAccountStatusQt(
    const identifier::Nym& nymID,
    const opentxs::blockchain::Type chain,
    const SimpleCallback cb) const noexcept
    -> opentxs::ui::BlockchainAccountStatusQt*
{
    auto lock = Lock{lock_};
    auto key = BlockchainAccountStatusKey{nymID, chain};
    auto it = blockchain_account_status_qt_.find(key);

    if (blockchain_account_status_qt_.end() == it) {
        auto& native = blockchain_account_status(lock, nymID, chain, cb);
        it = blockchain_account_status_qt_
                 .emplace(
                     std::move(key),
                     opentxs::factory::BlockchainAccountStatusQtModel(*native))
                 .first;

        OT_ASSERT(it->second);
    }

    return it->second.get();
}

auto ImpQt::BlockchainSelectionQt(
    const opentxs::ui::Blockchains key,
    const SimpleCallback updateCB) const noexcept
    -> opentxs::ui::BlockchainSelectionQt*
{
    auto lock = Lock{lock_};
    auto it = blockchain_selection_qt_.find(key);

    if (blockchain_selection_qt_.end() == it) {
        auto& native = blockchain_selection(lock, key, updateCB);
        it = blockchain_selection_qt_
                 .emplace(
                     key,
                     std::make_unique<opentxs::ui::BlockchainSelectionQt>(
                         *native))
                 .first;

        OT_ASSERT(it->second);
    }

    return it->second.get();
}

auto ImpQt::BlockchainStatisticsQt(const SimpleCallback cb) const noexcept
    -> opentxs::ui::BlockchainStatisticsQt*
{
    auto lock = Lock{lock_};

    if (false == bool(blockchain_statistics_qt_)) {
        auto& native = blockchain_statistics(lock, cb);
        blockchain_statistics_qt_ =
            opentxs::factory::BlockchainStatisticsQtModel(*native);
    }

    OT_ASSERT(blockchain_statistics_qt_);

    return blockchain_statistics_qt_.get();
}

auto ImpQt::ContactQt(const Identifier& contactID, const SimpleCallback cb)
    const noexcept -> opentxs::ui::ContactQt*
{
    auto lock = Lock{lock_};
    auto key = ContactKey{contactID};
    auto it = contacts_qt_.find(key);

    if (contacts_qt_.end() == it) {
        auto& native = contact(lock, contactID, cb);
        it = contacts_qt_
                 .emplace(
                     std::move(key), opentxs::factory::ContactQtModel(*native))
                 .first;

        OT_ASSERT(it->second);
    }

    return it->second.get();
}

auto ImpQt::ContactListQt(const identifier::Nym& nymID, const SimpleCallback cb)
    const noexcept -> opentxs::ui::ContactListQt*
{
    auto lock = Lock{lock_};
    auto key = ContactListKey{nymID};
    auto it = contact_lists_qt_.find(key);

    if (contact_lists_qt_.end() == it) {
        auto& native = contact_list(lock, nymID, cb);
        it = contact_lists_qt_
                 .emplace(
                     std::move(key),
                     opentxs::factory::ContactListQtModel(*native))
                 .first;

        OT_ASSERT(it->second);
    }

    return it->second.get();
}

auto ImpQt::MessagableListQt(
    const identifier::Nym& nymID,
    const SimpleCallback cb) const noexcept -> opentxs::ui::MessagableListQt*
{
    auto lock = Lock{lock_};
    auto key = MessagableListKey{nymID};
    auto it = messagable_lists_qt_.find(key);

    if (messagable_lists_qt_.end() == it) {
        auto& native = messagable_list(lock, nymID, cb);
        it = messagable_lists_qt_
                 .emplace(
                     std::move(key),
                     opentxs::factory::MessagableListQtModel(*native))
                 .first;

        OT_ASSERT(it->second);
    }

    return it->second.get();
}

auto ImpQt::PayableListQt(
    const identifier::Nym& nymID,
    contact::ContactItemType currency,
    const SimpleCallback cb) const noexcept -> opentxs::ui::PayableListQt*
{
    auto lock = Lock{lock_};
    auto key = PayableListKey{nymID, currency};
    auto it = payable_lists_qt_.find(key);

    if (payable_lists_qt_.end() == it) {
        auto& native = payable_list(lock, nymID, currency, cb);
        it = payable_lists_qt_
                 .emplace(
                     std::move(key),
                     opentxs::factory::PayableListQtModel(*native))
                 .first;

        OT_ASSERT(it->second);
    }

    return it->second.get();
}

auto ImpQt::ProfileQt(const identifier::Nym& nymID, const SimpleCallback cb)
    const noexcept -> opentxs::ui::ProfileQt*
{
    auto lock = Lock{lock_};
    auto key = ProfileKey{nymID};
    auto it = profiles_qt_.find(key);

    if (profiles_qt_.end() == it) {
        auto& native = profile(lock, nymID, cb);
        it = profiles_qt_
                 .emplace(
                     std::move(key), opentxs::factory::ProfileQtModel(*native))
                 .first;

        OT_ASSERT(it->second);
    }

    return it->second.get();
}

auto ImpQt::SeedValidator(
    const opentxs::crypto::SeedStyle type,
    const opentxs::crypto::Language lang) const noexcept
    -> const opentxs::ui::SeedValidator*
{
    auto lock = Lock{lock_};
    auto& langMap = seed_validators_[type];
    const auto [it, added] = langMap.try_emplace(
        lang,
        api_,
        static_cast<std::uint8_t>(type),
        static_cast<std::uint8_t>(lang));
    auto* out = &(it->second);
    opentxs::ui::claim_ownership(out);

    return out;
}

auto ImpQt::ShutdownModels() noexcept -> void
{
    unit_lists_qt_.clear();
    profiles_qt_.clear();
    payable_lists_qt_.clear();
    messagable_lists_qt_.clear();
    contacts_qt_.clear();
    contact_lists_qt_.clear();
    blockchain_statistics_qt_.reset();
    blockchain_selection_qt_.clear();
    blockchain_account_status_qt_.clear();
    activity_threads_qt_.clear();
    activity_summaries_qt_.clear();
    account_summaries_qt_.clear();
    account_lists_qt_.clear();
    accounts_qt_.clear();
    Imp::ShutdownModels();
}

auto ImpQt::UnitListQt(const identifier::Nym& nymID, const SimpleCallback cb)
    const noexcept -> opentxs::ui::UnitListQt*
{
    auto lock = Lock{lock_};
    auto key = UnitListKey{nymID};
    auto it = unit_lists_qt_.find(key);

    if (unit_lists_qt_.end() == it) {
        it = unit_lists_qt_
                 .emplace(
                     std::move(key),
                     opentxs::factory::UnitListQtModel(
                         *unit_list(lock, nymID, cb)))
                 .first;

        OT_ASSERT(it->second);
    }

    return it->second.get();
}

ImpQt::~ImpQt() { Shutdown(); }
}  // namespace opentxs::api::client::ui
