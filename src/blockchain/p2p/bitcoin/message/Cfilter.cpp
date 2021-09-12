// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"    // IWYU pragma: associated
#include "1_Internal.hpp"  // IWYU pragma: associated
#include "blockchain/p2p/bitcoin/message/Cfilter.hpp"  // IWYU pragma: associated

#include <cstddef>
#include <cstring>
#include <iterator>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

#include "blockchain/p2p/bitcoin/Header.hpp"
#include "blockchain/p2p/bitcoin/Message.hpp"
#include "internal/blockchain/Blockchain.hpp"
#include "internal/blockchain/p2p/bitcoin/Bitcoin.hpp"
#include "internal/blockchain/p2p/bitcoin/message/Message.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/blockchain/node/FilterOracle.hpp"
#include "opentxs/blockchain/p2p/Types.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"
#include "opentxs/network/blockchain/bitcoin/CompactSize.hpp"

// #define OT_METHOD "opentxs::blockchain::p2p::bitcoin::message::Cfilter::"

namespace opentxs::factory
{
auto BitcoinP2PCfilter(
    const api::Core& api,
    std::unique_ptr<blockchain::p2p::bitcoin::Header> pHeader,
    const blockchain::p2p::bitcoin::ProtocolVersion version,
    const void* payload,
    const std::size_t size)
    -> blockchain::p2p::bitcoin::message::internal::Cfilter*
{
    namespace bitcoin = blockchain::p2p::bitcoin;
    using ReturnType = bitcoin::message::implementation::Cfilter;

    if (false == bool(pHeader)) {
        LogOutput("opentxs::factory::")(__func__)(": Invalid header").Flush();

        return nullptr;
    }

    const auto& header = *pHeader;
    auto raw = ReturnType::BitcoinFormat{};
    auto expectedSize = sizeof(raw);

    if (expectedSize > size) {
        LogOutput("opentxs::factory::")(__func__)(": Payload too short (begin)")
            .Flush();

        return nullptr;
    }

    auto* it{static_cast<const std::byte*>(payload)};
    std::memcpy(reinterpret_cast<std::byte*>(&raw), it, sizeof(raw));
    std::advance(it, sizeof(raw));
    expectedSize += 1;

    if (expectedSize > size) {
        LogOutput("opentxs::factory::")(__func__)(
            ": Payload too short (compactsize)")
            .Flush();

        return nullptr;
    }

    auto filterSize = std::size_t{0};
    const auto haveSize = network::blockchain::bitcoin::DecodeSize(
        it, expectedSize, size, filterSize);

    if (false == haveSize) {
        LogOutput(__func__)(": CompactSize incomplete").Flush();

        return nullptr;
    }

    if ((expectedSize + filterSize) > size) {
        LogOutput("opentxs::factory::")(__func__)(
            ": Payload too short (filter)")
            .Flush();

        return nullptr;
    }

    const auto filterType = raw.Type(header.Network());

    try {
        const auto [elementCount, filterBytes] =
            blockchain::internal::DecodeSerializedCfilter(
                ReadView{reinterpret_cast<const char*>(it), filterSize});
        const auto start =
            reinterpret_cast<const std::byte*>(filterBytes.data());
        const auto end = start + filterBytes.size();

        return new ReturnType(
            api,
            std::move(pHeader),
            filterType,
            raw.Hash(),
            elementCount,
            Space{start, end});
    } catch (const std::exception& e) {
        LogOutput("opentxs::factory::")(__func__)(": ")(e.what()).Flush();

        return nullptr;
    }
}

auto BitcoinP2PCfilter(
    const api::Core& api,
    const blockchain::Type network,
    const blockchain::filter::Type type,
    const blockchain::block::Hash& hash,
    const blockchain::node::GCS& filter)
    -> blockchain::p2p::bitcoin::message::internal::Cfilter*
{
    namespace bitcoin = blockchain::p2p::bitcoin;
    using ReturnType = bitcoin::message::implementation::Cfilter;

    return new ReturnType(
        api, network, type, hash, filter.ElementCount(), filter.Compressed());
}
}  // namespace opentxs::factory

namespace opentxs::blockchain::p2p::bitcoin::message::implementation
{
Cfilter::Cfilter(
    const api::Core& api,
    const blockchain::Type network,
    const filter::Type type,
    const block::Hash& hash,
    const std::uint32_t count,
    const Space& compressed) noexcept
    : Message(api, network, bitcoin::Command::cfilter)
    , type_(type)
    , hash_(hash)
    , count_(count)
    , filter_(compressed)
    , params_(blockchain::internal::GetFilterParams(type_))
{
    init_hash();
}

Cfilter::Cfilter(
    const api::Core& api,
    std::unique_ptr<Header> header,
    const filter::Type type,
    const block::Hash& hash,
    const std::uint32_t count,
    Space&& compressed) noexcept
    : Message(api, std::move(header))
    , type_(type)
    , hash_(hash)
    , count_(count)
    , filter_(std::move(compressed))
    , params_(blockchain::internal::GetFilterParams(type_))
{
}

auto Cfilter::payload() const noexcept -> OTData
{
    try {
        auto raw = BitcoinFormat{header().Network(), type_, hash_};
        auto output = Data::Factory(&raw, sizeof(raw));
        const auto filter = [&] {
            auto output = CompactSize(count_).Encode();
            output.insert(output.end(), filter_.begin(), filter_.end());

            return output;
        }();
        const auto payload = [&] {
            auto output = CompactSize(filter.size()).Encode();
            output.insert(output.end(), filter.begin(), filter.end());

            return output;
        }();
        output->Concatenate(payload.data(), payload.size());

        return output;
    } catch (...) {
        return Data::Factory();
    }
}
}  // namespace opentxs::blockchain::p2p::bitcoin::message::implementation
