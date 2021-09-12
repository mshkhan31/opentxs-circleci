// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                      // IWYU pragma: associated
#include "1_Internal.hpp"                    // IWYU pragma: associated
#include "opentxs/ui/qt/ActivityThread.hpp"  // IWYU pragma: associated

#include <QDateTime>
#include <QObject>
#include <QString>
#include <QVariant>
#include <memory>
#include <string>

#include "internal/ui/UI.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/contact/ContactItemType.hpp"
#include "opentxs/core/Identifier.hpp"
#include "ui/activitythread/ActivityThreadItem.hpp"
#include "ui/qt/DraftValidator.hpp"
#include "util/Polarity.hpp"  // IWYU pragma: keep

class QValidator;

namespace opentxs::factory
{
auto ActivityThreadQtModel(ui::internal::ActivityThread& parent) noexcept
    -> std::unique_ptr<ui::ActivityThreadQt>
{
    using ReturnType = ui::ActivityThreadQt;

    return std::make_unique<ReturnType>(parent);
}
}  // namespace opentxs::factory

namespace opentxs::ui
{
struct ActivityThreadQt::Imp {
    internal::ActivityThread& parent_;
    implementation::DraftValidator validator_;

    Imp(internal::ActivityThread& parent) noexcept
        : parent_(parent)
        , validator_(parent_)
    {
    }
};

ActivityThreadQt::ActivityThreadQt(internal::ActivityThread& parent) noexcept
    : Model(parent.GetQt())
    , imp_(std::make_unique<Imp>(parent).release())
{
    if (nullptr != internal_) {
        internal_->SetColumnCount(nullptr, 6);
        internal_->SetRoleData({
            {ActivityThreadQt::IntAmountRole, "intamount"},
            {ActivityThreadQt::StringAmountRole, "stramount"},
            {ActivityThreadQt::LoadingRole, "loading"},
            {ActivityThreadQt::MemoRole, "memo"},
            {ActivityThreadQt::PendingRole, "pending"},
            {ActivityThreadQt::PolarityRole, "polarity"},
            {ActivityThreadQt::TextRole, "text"},
            {ActivityThreadQt::TimeRole, "time"},
            {ActivityThreadQt::TypeRole, "type"},
            {ActivityThreadQt::OutgoingRole, "outgoing"},
            {ActivityThreadQt::FromRole, "from"},
        });
    }

    imp_->parent_.SetCallbacks({
        []() -> void {},
        [this]() -> void { emit displayNameUpdate(); },
        [this]() -> void { emit draftUpdate(); },
        [this](bool value) -> void { emit canMessageUpdate(value); },
    });
}

auto ActivityThreadQt::canMessage() const noexcept -> bool
{
    return imp_->parent_.CanMessage();
}

auto ActivityThreadQt::displayName() const noexcept -> QString
{
    return imp_->parent_.DisplayName().c_str();
}

auto ActivityThreadQt::draft() const noexcept -> QString
{
    return imp_->parent_.GetDraft().c_str();
}

auto ActivityThreadQt::draftValidator() const noexcept -> QValidator*
{
    return &(imp_->validator_);
}

auto ActivityThreadQt::headerData(int section, Qt::Orientation, int role)
    const noexcept -> QVariant
{
    if (Qt::DisplayRole != role) { return {}; }

    switch (section) {
        case TimeColumn: {
            return "Time";
        }
        case FromColumn: {
            return "Sender";
        }
        case TextColumn: {
            return "Event";
        }
        case AmountColumn: {
            return "Amount";
        }
        case MemoColumn: {
            return "Memo";
        }
        case LoadingColumn: {
            return "Loading";
        }
        case PendingColumn: {
            return "Pending";
        }
        default: {

            return {};
        }
    }
}

auto ActivityThreadQt::participants() const noexcept -> QString
{
    return imp_->parent_.Participants().c_str();
}

auto ActivityThreadQt::pay(
    const QString& amount,
    const QString& sourceAccount,
    const QString& memo) const noexcept -> bool
{
    return imp_->parent_.Pay(
        amount.toStdString(),
        Identifier::Factory(sourceAccount.toStdString()),
        memo.toStdString(),
        PaymentType::Cheque);
}

auto ActivityThreadQt::paymentCode(const int currency) const noexcept -> QString
{
    return imp_->parent_
        .PaymentCode(static_cast<contact::ContactItemType>(currency))
        .c_str();
}

auto ActivityThreadQt::sendDraft() const noexcept -> bool
{
    return imp_->parent_.SendDraft();
}

auto ActivityThreadQt::setDraft(QString draft) -> void
{
    imp_->parent_.SetDraft(draft.toStdString());
}

auto ActivityThreadQt::threadID() const noexcept -> QString
{
    return imp_->parent_.ThreadID().c_str();
}

ActivityThreadQt::~ActivityThreadQt()
{
    if (nullptr != imp_) {
        delete imp_;
        imp_ = nullptr;
    }
}
}  // namespace opentxs::ui

namespace opentxs::ui::implementation
{
auto ActivityThreadItem::qt_data(
    const int column,
    const int role,
    QVariant& out) const noexcept -> void
{
    switch (role) {
        case Qt::DisplayRole: {
            switch (column) {
                case ActivityThreadQt::TimeColumn: {
                    qt_data(column, ActivityThreadQt::TimeRole, out);
                } break;
                case ActivityThreadQt::FromColumn: {
                    qt_data(column, ActivityThreadQt::FromRole, out);
                } break;
                case ActivityThreadQt::TextColumn: {
                    qt_data(column, ActivityThreadQt::TextRole, out);
                } break;
                case ActivityThreadQt::AmountColumn: {
                    qt_data(column, ActivityThreadQt::StringAmountRole, out);
                } break;
                case ActivityThreadQt::MemoColumn: {
                    qt_data(column, ActivityThreadQt::MemoRole, out);
                } break;
                case ActivityThreadQt::LoadingColumn:
                case ActivityThreadQt::PendingColumn:
                default: {
                }
            }
        } break;
        case Qt::CheckStateRole: {
            switch (column) {
                case ActivityThreadQt::LoadingColumn: {
                    qt_data(column, ActivityThreadQt::LoadingRole, out);
                } break;
                case ActivityThreadQt::PendingColumn: {
                    qt_data(column, ActivityThreadQt::PendingRole, out);
                } break;
                case ActivityThreadQt::TextColumn:
                case ActivityThreadQt::AmountColumn:
                case ActivityThreadQt::MemoColumn:
                case ActivityThreadQt::TimeColumn:
                default: {
                }
            }
        } break;
        case ActivityThreadQt::IntAmountRole: {
            out = static_cast<int>(Amount());
        } break;
        case ActivityThreadQt::StringAmountRole: {
            out = DisplayAmount().c_str();
        } break;
        case ActivityThreadQt::LoadingRole: {
            out = Loading();
        } break;
        case ActivityThreadQt::MemoRole: {
            out = Memo().c_str();
        } break;
        case ActivityThreadQt::PendingRole: {
            out = Pending();
        } break;
        case ActivityThreadQt::PolarityRole: {
            out = polarity(Amount());
        } break;
        case ActivityThreadQt::TextRole: {
            out = Text().c_str();
        } break;
        case ActivityThreadQt::TimeRole: {
            auto output = QDateTime{};
            output.setSecsSinceEpoch(Clock::to_time_t(Timestamp()));

            out = output;
        } break;
        case ActivityThreadQt::TypeRole: {
            out = static_cast<int>(Type());
        } break;
        case ActivityThreadQt::OutgoingRole: {
            out = Outgoing();
        } break;
        case ActivityThreadQt::FromRole: {
            out = From().c_str();
        } break;
        default: {
        }
    }
}
}  // namespace opentxs::ui::implementation
