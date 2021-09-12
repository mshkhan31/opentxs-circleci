// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"    // IWYU pragma: associated
#include "1_Internal.hpp"  // IWYU pragma: associated
#include "blockchain/p2p/bitcoin/message/Version.hpp"  // IWYU pragma: associated

#include <boost/asio.hpp>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <limits>
#include <utility>

#include "blockchain/p2p/bitcoin/Header.hpp"
#include "blockchain/p2p/bitcoin/Message.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"

//#define OT_METHOD "
// opentxs::blockchain::p2p::bitcoin::message::implementation::Version::"

namespace opentxs::factory
{
auto BitcoinP2PVersion(
    const api::Core& api,
    std::unique_ptr<blockchain::p2p::bitcoin::Header> pHeader,
    const blockchain::p2p::bitcoin::ProtocolVersion,
    const void* payload,
    const std::size_t size)
    -> blockchain::p2p::bitcoin::message::internal::Version*
{
    namespace bitcoin = blockchain::p2p::bitcoin;
    using ReturnType = bitcoin::message::implementation::Version;

    if (false == bool(pHeader)) {
        LogOutput("opentxs::factory::")(__func__)(": Invalid header").Flush();

        return nullptr;
    }

    const auto& header = *pHeader;
    auto expectedSize = sizeof(ReturnType::BitcoinFormat_1);

    if (expectedSize > size) {
        LogOutput("opentxs::factory::")(__func__)(
            ": Size below minimum for version 1")
            .Flush();

        return nullptr;
    }

    auto* it{static_cast<const std::byte*>(payload)};
    ReturnType::BitcoinFormat_1 raw1{};
    std::memcpy(reinterpret_cast<std::byte*>(&raw1), it, sizeof(raw1));
    it += sizeof(raw1);
    bitcoin::ProtocolVersion version{raw1.version_.value()};
    auto services = bitcoin::GetServices(raw1.services_.value());
    auto localServices{services};
    auto timestamp = Clock::from_time_t(raw1.timestamp_.value());
    auto remoteServices = bitcoin::GetServices(raw1.remote_.services_.value());
    tcp::endpoint remoteAddress{
        ip::make_address_v6(raw1.remote_.address_), raw1.remote_.port_.value()};
    tcp::endpoint localAddress{};
    bitcoin::Nonce nonce{};
    std::string userAgent{};
    blockchain::block::Height height{};
    bool relay{true};

    if (106 <= version) {
        expectedSize += (1 + sizeof(ReturnType::BitcoinFormat_106));

        if (expectedSize > size) {
            LogOutput("opentxs::factory::")(__func__)(
                ": Size below minimum for version 106")
                .Flush();

            return nullptr;
        }

        ReturnType::BitcoinFormat_106 raw2{};
        std::memcpy(reinterpret_cast<std::byte*>(&raw2), it, sizeof(raw2));
        it += sizeof(raw2);
        localServices = bitcoin::GetServices(raw2.local_.services_.value());
        localAddress = {
            ip::make_address_v6(raw2.local_.address_),
            raw2.local_.port_.value()};
        nonce = raw2.nonce_.value();
        std::size_t uaSize{0};

        if (std::byte{0} == *it) {
            it += 1;
        } else {
            const auto csBytes = bitcoin::CompactSize::CalculateSize(*it);
            expectedSize += csBytes;

            if (expectedSize > size) {
                LogOutput("opentxs::factory::")(__func__)(
                    ": CompactSize incomplete")
                    .Flush();

                return nullptr;
            }

            if (0 == csBytes) {
                uaSize = std::to_integer<std::uint8_t>(*it);
                it += 1;
            } else {
                it += 1;
                bitcoin::CompactSize::Bytes bytes{it, it + csBytes};
                uaSize = bitcoin::CompactSize(bytes).Value();
                it += csBytes;
            }

            expectedSize += uaSize;

            if (expectedSize > size) {
                LogOutput("opentxs::factory::")(__func__)(
                    ": User agent string incomplete")
                    .Flush();

                return nullptr;
            }

            userAgent = {reinterpret_cast<const char*>(it), uaSize};
            it += uaSize;
        }
    }

    if (209 <= version) {
        expectedSize += sizeof(ReturnType::BitcoinFormat_209);

        if (expectedSize > size) {
            LogOutput("opentxs::factory::")(__func__)(
                ": Required height field is missing")
                .Flush();

            return nullptr;
        }

        ReturnType::BitcoinFormat_209 raw3{};
        std::memcpy(reinterpret_cast<std::byte*>(&raw3), it, sizeof(raw3));
        it += sizeof(raw3);

        height = raw3.height_.value();
    }

    if (70001 <= version) {
        expectedSize += 1;

        if (expectedSize == size) {
            auto value = std::to_integer<std::uint8_t>(*it);
            relay = (0 == value) ? false : true;
        }
    }

    const auto chain = header.Network();

    return new ReturnType(
        api,
        std::move(pHeader),
        version,
        localAddress,
        remoteAddress,
        TranslateServices(chain, version, services),
        TranslateServices(chain, version, localServices),
        TranslateServices(chain, version, remoteServices),
        nonce,
        userAgent,
        height,
        relay,
        timestamp);
}

auto BitcoinP2PVersion(
    const api::Core& api,
    const blockchain::Type network,
    const blockchain::p2p::Network style,
    const std::int32_t version,
    const std::set<blockchain::p2p::Service>& localServices,
    const std::string& localAddress,
    const std::uint16_t localPort,
    const std::set<blockchain::p2p::Service>& remoteServices,
    const std::string& remoteAddress,
    const std::uint16_t remotePort,
    const std::uint64_t nonce,
    const std::string& userAgent,
    const blockchain::block::Height height,
    const bool relay) -> blockchain::p2p::bitcoin::message::internal::Version*
{
    namespace bitcoin = blockchain::p2p::bitcoin;
    using ReturnType = bitcoin::message::implementation::Version;
    tcp::endpoint local{};
    tcp::endpoint remote{};

    if (blockchain::p2p::Network::zmq == style) {
        local =
            tcp::endpoint(ip::make_address_v6("::FFFF:7f0e:5801"), localPort);
        remote =
            tcp::endpoint(ip::make_address_v6("::FFFF:7f0e:5802"), remotePort);
    } else {
        try {
            local = tcp::endpoint(ip::make_address_v6(localAddress), localPort);
        } catch (...) {
            LogOutput("opentxs::factory::")(__func__)(
                ": Invalid local address: ")(localAddress)
                .Flush();

            OT_FAIL;
        }

        try {
            remote =
                tcp::endpoint(ip::make_address_v6(remoteAddress), remotePort);
        } catch (...) {
            LogOutput("opentxs::factory::")(__func__)(
                ": Invalid remote address: ")(remoteAddress)
                .Flush();

            OT_FAIL;
        }
    }

    return new ReturnType(
        api,
        network,
        version,
        local,
        remote,
        localServices,
        localServices,
        remoteServices,
        nonce,
        userAgent,
        height,
        relay);
}
}  // namespace opentxs::factory

namespace opentxs::blockchain::p2p::bitcoin::message::implementation
{
Version::Version(
    const api::Core& api,
    const blockchain::Type network,
    const bitcoin::ProtocolVersion version,
    const tcp::endpoint localAddress,
    const tcp::endpoint remoteAddress,
    const std::set<blockchain::p2p::Service>& services,
    const std::set<blockchain::p2p::Service>& localServices,
    const std::set<blockchain::p2p::Service>& remoteServices,
    const bitcoin::Nonce nonce,
    const std::string& userAgent,
    const block::Height height,
    const bool relay,
    const Time time) noexcept
    : Message(api, network, bitcoin::Command::version)
    , version_(version)
    , local_address_(localAddress)
    , remote_address_(remoteAddress)
    , services_(services)
    , local_services_(localServices)
    , remote_services_(remoteServices)
    , nonce_(nonce)
    , user_agent_(userAgent)
    , height_(height)
    , relay_(relay)
    , timestamp_(time)
{
    init_hash();
}

Version::Version(
    const api::Core& api,
    std::unique_ptr<Header> header,
    const bitcoin::ProtocolVersion version,
    const tcp::endpoint localAddress,
    const tcp::endpoint remoteAddress,
    const std::set<blockchain::p2p::Service>& services,
    const std::set<blockchain::p2p::Service>& localServices,
    const std::set<blockchain::p2p::Service>& remoteServices,
    const bitcoin::Nonce nonce,
    const std::string& userAgent,
    const block::Height height,
    const bool relay,
    const Time time) noexcept
    : Message(api, std::move(header))
    , version_(version)
    , local_address_(localAddress)
    , remote_address_(remoteAddress)
    , services_(services)
    , local_services_(localServices)
    , remote_services_(remoteServices)
    , nonce_(nonce)
    , user_agent_(userAgent)
    , height_(height)
    , relay_(relay)
    , timestamp_(time)
{
}

Version::BitcoinFormat_1::BitcoinFormat_1() noexcept
    : version_()
    , services_()
    , timestamp_()
    , remote_()
{
    static_assert(46 == sizeof(BitcoinFormat_1));
}

Version::BitcoinFormat_1::BitcoinFormat_1(
    const bitcoin::ProtocolVersion version,
    const std::set<bitcoin::Service>& localServices,
    const std::set<bitcoin::Service>& remoteServices,
    const tcp::endpoint& remoteAddress,
    const Time time) noexcept
    : version_(version)
    , services_(GetServiceBytes(localServices))
    , timestamp_(Clock::to_time_t(time))
    , remote_(remoteServices, remoteAddress)
{
    static_assert(46 == sizeof(BitcoinFormat_1));
}

Version::BitcoinFormat_106::BitcoinFormat_106() noexcept
    : local_()
    , nonce_()
{
    static_assert(34 == sizeof(BitcoinFormat_106));
}

Version::BitcoinFormat_106::BitcoinFormat_106(
    const std::set<bitcoin::Service>& services,
    const tcp::endpoint address,
    const bitcoin::Nonce nonce) noexcept
    : local_(services, address)
    , nonce_(nonce)
{
    static_assert(34 == sizeof(BitcoinFormat_106));
}

Version::BitcoinFormat_209::BitcoinFormat_209() noexcept
    : height_()
{
    static_assert(4 == sizeof(BitcoinFormat_209));
}

Version::BitcoinFormat_209::BitcoinFormat_209(
    const block::Height height) noexcept
    : height_(static_cast<std::uint32_t>(height))
{
    static_assert(4 == sizeof(BitcoinFormat_209));
    static_assert(sizeof(height_) == sizeof(std::uint32_t));

    OT_ASSERT(std::numeric_limits<std::uint32_t>::max() >= height);
}

auto Version::payload() const noexcept -> OTData
{
    BitcoinFormat_1 raw1(
        version_,
        TranslateServices(header_->Network(), version_, services_),
        TranslateServices(header_->Network(), version_, remote_services_),
        remote_address_,
        timestamp_);
    auto output = Data::Factory(&raw1, sizeof(raw1));

    if (106 <= version_) {
        BitcoinFormat_106 raw2(
            TranslateServices(header_->Network(), version_, local_services_),
            local_address_,
            nonce_);
        output->Concatenate(&raw2, sizeof(raw2));
        output += BitcoinString(user_agent_);
    }

    if (209 <= version_) {
        BitcoinFormat_209 raw3(height_);
        output->Concatenate(&raw3, sizeof(raw3));
    }

    if (70001 <= version_) {
        const auto relay = (relay_) ? std::byte{1} : std::byte{0};
        output->Concatenate(&relay, sizeof(relay));
    }

    return output;
}
}  // namespace opentxs::blockchain::p2p::bitcoin::message::implementation
