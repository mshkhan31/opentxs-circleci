// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                    // IWYU pragma: associated
#include "1_Internal.hpp"                  // IWYU pragma: associated
#include "ui/qt/DestinationValidator.hpp"  // IWYU pragma: associated

#include <QObject>
#include <map>
#include <sstream>
#include <string>
#include <type_traits>
#include <utility>

#include "opentxs/Pimpl.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/api/client/Blockchain.hpp"
#include "opentxs/api/client/Manager.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/crypto/AddressStyle.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/crypto/PaymentCode.hpp"

// #define OT_METHOD "opentxs::ui::BlockchainDestinationValidator::"

namespace opentxs::ui
{
#if OT_BLOCKCHAIN
using Super = DestinationValidator::Imp;

struct BlockchainDestinationValidator final : public Super {
    auto fixup(QString& input) const -> void final
    {
        strip_invalid(input, true);

        if (0 == input.size()) { reset(std::string{}); }
    }
    auto getDetails() const -> QString final { return details_.c_str(); }
    auto validate(QString& input, int&) const -> QValidator::State final
    {
        fixup(input);

        if (0 == input.size()) {
            reset(std::string{});

            return QValidator::State::Intermediate;
        }

        const auto candidate = input.toStdString();
        const auto code = api_.Factory().PaymentCode(candidate);
        auto text = std::stringstream{};
        text << "Address format: ";

        if (const auto ver = code->Version(); 0u < ver) {
            text << "version " << std::to_string(ver) << " payment code";
            reset(text.str());

            return QValidator::State::Acceptable;
        }

        const auto decoded = api_.Blockchain().DecodeAddress(candidate);
        const auto& [data, style, chains, supported] = decoded;

        if (0 == data->size()) {
            text << "unknown";
            reset(text.str());

            return QValidator::State::Intermediate;
        }

        OT_ASSERT(0 < chains.size());

        const auto type = *chains.begin();
        const auto chain = blockchain::DisplayString(type);
        const auto validChain = (0u < chains.count(chain_));
        using Style = blockchain::crypto::AddressStyle;

        switch (style) {
            case Style::P2PKH: {
                text << "P2PKH";
            } break;
            case Style::P2SH: {
                text << "P2SH";
            } break;
            case Style::P2WPKH: {
                text << "P2WPKH";
            } break;
            case Style::P2WSH: {
                text << "P2WSH";
            } break;
            case Style::P2TR: {
                text << "P2TR";
            } break;
            case Style::Unknown:
            default: {
                text << "unsupported";
                reset(text.str());

                return QValidator::State::Intermediate;
            }
        }

        if (validChain) {
            if (supported) {
                reset(text.str());

                return QValidator::State::Acceptable;
            } else {
                text << " not supported on " << chain;

                return QValidator::State::Invalid;
            }
        } else {
            text = std::stringstream{};
            text << "This address is only valid on ";

            if (blockchain::IsTestnet(type)) {
                text << "testnet";
            } else {
                text << chain;
            }

            reset(text.str());

            return QValidator::State::Intermediate;
        }
    }

    BlockchainDestinationValidator(
        const api::client::Manager& api,
        DestinationValidator& main,
        blockchain::Type chain,
        Parent& parent) noexcept
        : parent_(main)
        , api_(api)
        , chain_(chain)
        , details_()
    {
    }
    ~BlockchainDestinationValidator() final = default;

private:
    DestinationValidator& parent_;
    const api::client::Manager& api_;
    const blockchain::Type chain_;
    mutable std::string details_;

    auto reset(std::string&& details) const noexcept -> void
    {
        details_ = std::move(details);

        emit parent_.detailsChanged(details_.c_str());
    }
};
#endif  // OT_BLOCKCHAIN

auto DestinationValidator::Imp::Blockchain(
    const api::client::Manager& api,
    DestinationValidator& main,
    const Identifier& account,
    Parent& parent) noexcept -> std::unique_ptr<Imp>
{
#if OT_BLOCKCHAIN
    const auto [chain, owner] = api.Blockchain().LookupAccount(account);

    if (blockchain::Type::Unknown == chain) { return nullptr; }

    return std::make_unique<BlockchainDestinationValidator>(
        api, main, chain, parent);
#else
    return nullptr;
#endif  // OT_BLOCKCHAIN
}
}  // namespace opentxs::ui
