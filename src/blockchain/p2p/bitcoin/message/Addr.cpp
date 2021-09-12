// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                             // IWYU pragma: associated
#include "1_Internal.hpp"                           // IWYU pragma: associated
#include "blockchain/p2p/bitcoin/message/Addr.hpp"  // IWYU pragma: associated

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <type_traits>

#include "blockchain/p2p/bitcoin/Header.hpp"
#include "blockchain/p2p/bitcoin/Message.hpp"
#include "internal/blockchain/p2p/P2P.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"
#include "opentxs/network/blockchain/bitcoin/CompactSize.hpp"

//#define OT_METHOD "
// opentxs::blockchain::p2p::bitcoin::message::implementation::Addr::"

namespace opentxs::factory
{
auto BitcoinP2PAddr(
    const api::Core& api,
    std::unique_ptr<blockchain::p2p::bitcoin::Header> pHeader,
    const blockchain::p2p::bitcoin::ProtocolVersion version,
    const void* payload,
    const std::size_t size)
    -> blockchain::p2p::bitcoin::message::internal::Addr*
{
    namespace p2p = blockchain::p2p;
    namespace bitcoin = p2p::bitcoin;
    using ReturnType = bitcoin::message::implementation::Addr;

    if (false == bool(pHeader)) {
        LogOutput("opentxs::factory::")(__func__)(": Invalid header").Flush();

        return nullptr;
    }

    const auto& header = *pHeader;
    auto expectedSize = sizeof(std::byte);

    if (expectedSize > size) {
        LogOutput("opentxs::factory::")(__func__)(
            ": Payload too short (compactsize)")
            .Flush();

        return nullptr;
    }

    auto* it{static_cast<const std::byte*>(payload)};
    std::size_t count{0};
    const bool haveCount =
        network::blockchain::bitcoin::DecodeSize(it, expectedSize, size, count);

    if (false == haveCount) {
        LogOutput(__func__)(": Invalid CompactSizee").Flush();

        return nullptr;
    }

    ReturnType::AddressVector addresses{};
    const auto chain = header.Network();

    if (count > 0) {
        for (std::size_t i{0}; i < count; ++i) {
            const bool timestamp = ReturnType::SerializeTimestamp(version);
            const std::size_t addressSize =
                timestamp ? sizeof(ReturnType::BitcoinFormat_31402)
                          : sizeof(bitcoin::AddressVersion);
            expectedSize += addressSize;

            if (expectedSize > size) {
                LogOutput("opentxs::factory::")(__func__)(
                    ": Address entries incomplete at entry index ")(i)
                    .Flush();

                return nullptr;
            }

            if (timestamp) {
                ReturnType::BitcoinFormat_31402 raw;
                std::memcpy(
                    reinterpret_cast<std::byte*>(&raw), it, sizeof(raw));
                const auto [network, bytes] =
                    ReturnType::ExtractAddress(raw.data_.address_);
                addresses.emplace_back(factory::BlockchainAddress(
                    api,
                    p2p::Protocol::bitcoin,
                    network,
                    bytes,
                    raw.data_.port_.value(),
                    chain,
                    Clock::from_time_t(raw.time_.value()),
                    bitcoin::TranslateServices(
                        chain,
                        version,
                        bitcoin::GetServices(raw.data_.services_.value())),
                    false));
            } else {
                bitcoin::AddressVersion raw;
                std::memcpy(
                    reinterpret_cast<std::byte*>(&raw), it, sizeof(raw));
                const auto [network, bytes] =
                    ReturnType::ExtractAddress(raw.address_);
                addresses.emplace_back(factory::BlockchainAddress(
                    api,
                    p2p::Protocol::bitcoin,
                    network,
                    bytes,
                    raw.port_.value(),
                    chain,
                    Time{},
                    bitcoin::TranslateServices(
                        chain,
                        version,
                        bitcoin::GetServices(raw.services_.value())),
                    false));
            }

            it += addressSize;
        }
    }

    return new ReturnType(
        api, std::move(pHeader), version, std::move(addresses));
}

auto BitcoinP2PAddr(
    const api::Core& api,
    const blockchain::Type network,
    const blockchain::p2p::bitcoin::ProtocolVersion version,
    std::vector<std::unique_ptr<blockchain::p2p::internal::Address>>&&
        addresses) -> blockchain::p2p::bitcoin::message::internal::Addr*
{
    namespace bitcoin = blockchain::p2p::bitcoin;
    using ReturnType = bitcoin::message::implementation::Addr;

    return new ReturnType(api, network, version, std::move(addresses));
}
}  // namespace opentxs::factory

namespace opentxs::blockchain::p2p::bitcoin::message::implementation
{
Addr::Addr(
    const api::Core& api,
    const blockchain::Type network,
    const ProtocolVersion version,
    AddressVector&& addresses) noexcept
    : Message(api, network, bitcoin::Command::addr)
    , version_(version)
    , payload_(std::move(addresses))
{
    init_hash();
}

Addr::Addr(
    const api::Core& api,
    std::unique_ptr<Header> header,
    const ProtocolVersion version,
    AddressVector&& addresses) noexcept
    : Message(api, std::move(header))
    , version_(version)
    , payload_(std::move(addresses))
{
}

Addr::BitcoinFormat_31402::BitcoinFormat_31402(
    const blockchain::Type chain,
    const ProtocolVersion version,
    const p2p::internal::Address& address)
    : time_(
          static_cast<std::uint32_t>(Clock::to_time_t(address.LastConnected())))
    , data_(chain, version, address)
{
}

Addr::BitcoinFormat_31402::BitcoinFormat_31402()
    : time_()
    , data_()
{
}

auto Addr::ExtractAddress(AddressByteField in) noexcept
    -> std::pair<p2p::Network, OTData>
{
    std::pair<p2p::Network, OTData> output{
        Network::ipv6, Data::Factory(in.data(), in.size())};
    auto& [type, bytes] = output;
    auto prefix = Data::Factory();

    if (bytes->Extract(AddressVersion::cjdns_prefix_->size(), prefix) &&
        AddressVersion::cjdns_prefix_ == prefix) {
        type = Network::cjdns;
    } else if (
        bytes->Extract(AddressVersion::ipv4_prefix_->size(), prefix) &&
        AddressVersion::ipv4_prefix_ == prefix) {
        type = Network::ipv4;
        bytes->Extract(
            bytes->size() - AddressVersion::ipv4_prefix_->size(),
            prefix,
            AddressVersion::ipv4_prefix_->size());
        bytes = prefix;

        OT_ASSERT(4 == bytes->size());
    } else if (
        bytes->Extract(AddressVersion::onion_prefix_->size(), prefix) &&
        AddressVersion::onion_prefix_ == prefix) {
        type = Network::onion2;
        bytes->Extract(
            bytes->size() - AddressVersion::onion_prefix_->size(),
            prefix,
            AddressVersion::onion_prefix_->size());
        bytes = prefix;

        OT_ASSERT(10 == bytes->size());
    }

    return output;
}

auto Addr::payload() const noexcept -> OTData
{
    auto output = Data::Factory(CompactSize(payload_.size()).Encode());

    for (const auto& pAddress : payload_) {
        OT_ASSERT(pAddress);

        const auto& address = *pAddress;

        if (SerializeTimestamp()) {
            const BitcoinFormat_31402 raw{
                header_->Network(), version_, address};
            output += Data::Factory(&raw, sizeof(raw));
        } else {
            const AddressVersion raw{header_->Network(), version_, address};
            output += Data::Factory(&raw, sizeof(raw));
        }
    }

    return output;
}

auto Addr::SerializeTimestamp(const ProtocolVersion version) noexcept -> bool
{
    return version >= 31402;
}
}  // namespace opentxs::blockchain::p2p::bitcoin::message::implementation
