// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                    // IWYU pragma: associated
#include "1_Internal.hpp"                  // IWYU pragma: associated
#include "api/network/asio/Acceptors.hpp"  // IWYU pragma: associated

#include <map>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <string>
#include <utility>

#include "api/network/asio/Acceptor.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"
#include "opentxs/network/asio/Endpoint.hpp"

#define OT_METHOD "opentxs::api::network::asio::Acceptors::Imp::"

namespace opentxs::api::network::asio
{
struct Acceptors::Imp {
    auto Start(
        const opentxs::network::asio::Endpoint& endpoint,
        Callback cb) noexcept -> bool
    {
        try {
            auto lock = Lock{lock_};
            const auto [it, added] = map_.try_emplace(
                endpoint.str(), endpoint, parent_, context_, std::move(cb));

            if (added) {
                it->second.Start();
                LogNormal("TCP socket opened for incoming connections on ")(
                    endpoint.str())
                    .Flush();
            } else {
                throw std::runtime_error{
                    std::string{"Listen socket already open on "} +
                    endpoint.str()};
            }

            return added;
        } catch (const std::exception& e) {
            LogOutput(OT_METHOD)(__func__)(": ")(e.what()).Flush();

            return false;
        }
    }
    auto Close(const opentxs::network::asio::Endpoint& endpoint) noexcept
        -> bool
    {
        try {
            auto lock = Lock{lock_};
            map_.at(endpoint.str()).Stop();

            return true;
        } catch (...) {

            return false;
        }
    }
    auto Stop() noexcept -> void
    {
        auto lock = Lock{lock_};

        for (auto& [name, acceptor] : map_) { acceptor.Stop(); }

        map_.clear();
    }

    Imp(internal::Asio& parent, boost::asio::io_context& context) noexcept
        : parent_(parent)
        , context_(context)
        , lock_()
        , map_()
    {
    }

    ~Imp() { Stop(); }

private:
    internal::Asio& parent_;
    boost::asio::io_context& context_;
    mutable std::mutex lock_;
    std::map<std::string, Acceptor> map_;

    Imp() = delete;
    Imp(const Imp&) = delete;
    Imp(Imp&&) = delete;
    auto operator=(const Imp&) -> Imp& = delete;
    auto operator=(Imp&&) -> Imp& = delete;
};

Acceptors::Acceptors(
    internal::Asio& parent,
    boost::asio::io_context& context) noexcept
    : imp_(std::make_unique<Imp>(parent, context).release())
{
}

auto Acceptors::Close(const opentxs::network::asio::Endpoint& endpoint) noexcept
    -> bool
{
    return imp_->Close(endpoint);
}

auto Acceptors::Start(
    const opentxs::network::asio::Endpoint& endpoint,
    Callback cb) noexcept -> bool
{
    return imp_->Start(endpoint, std::move(cb));
}

auto Acceptors::Stop() noexcept -> void { imp_->Stop(); }

Acceptors::~Acceptors()
{
    if (nullptr != imp_) {
        delete imp_;
        imp_ = nullptr;
    }
}
}  // namespace opentxs::api::network::asio
