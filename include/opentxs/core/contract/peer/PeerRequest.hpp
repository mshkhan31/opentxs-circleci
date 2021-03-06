// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CORE_CONTRACT_PEER_PEERREQUEST_HPP
#define OPENTXS_CORE_CONTRACT_PEER_PEERREQUEST_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <string>

#include "opentxs/SharedPimpl.hpp"
#include "opentxs/core/contract/Signable.hpp"
#include "opentxs/core/contract/peer/Types.hpp"

namespace opentxs
{
namespace contract
{
namespace peer
{
namespace request
{
class Bailment;
class BailmentNotice;
class Connection;
class Outbailment;
class StoreSecret;
}  // namespace request

class Request;
}  // namespace peer
}  // namespace contract

namespace identifier
{
class Nym;
class Server;
}  // namespace identifier

namespace proto
{
class PeerRequest;
}  // namespace proto

using OTPeerRequest = SharedPimpl<contract::peer::Request>;
}  // namespace opentxs

namespace opentxs
{
namespace contract
{
namespace peer
{
class OPENTXS_EXPORT Request : virtual public opentxs::contract::Signable
{
public:
    using SerializedType = proto::PeerRequest;

    virtual auto asBailment() const noexcept -> const request::Bailment& = 0;
    virtual auto asBailmentNotice() const noexcept
        -> const request::BailmentNotice& = 0;
    virtual auto asConnection() const noexcept
        -> const request::Connection& = 0;
    virtual auto asOutbailment() const noexcept
        -> const request::Outbailment& = 0;
    virtual auto asStoreSecret() const noexcept
        -> const request::StoreSecret& = 0;

    using Signable::Serialize;
    OPENTXS_NO_EXPORT virtual auto Serialize(SerializedType&) const -> bool = 0;
    virtual auto Initiator() const -> const identifier::Nym& = 0;
    virtual auto Recipient() const -> const identifier::Nym& = 0;
    virtual auto Server() const -> const identifier::Server& = 0;
    virtual auto Type() const -> PeerRequestType = 0;

    ~Request() override = default;

protected:
    Request() noexcept = default;

private:
    friend OTPeerRequest;

#ifndef _WIN32
    auto clone() const noexcept -> Request* override = 0;
#endif

    Request(const Request&) = delete;
    Request(Request&&) = delete;
    auto operator=(const Request&) -> Request& = delete;
    auto operator=(Request&&) -> Request& = delete;
};
}  // namespace peer
}  // namespace contract
}  // namespace opentxs
#endif
