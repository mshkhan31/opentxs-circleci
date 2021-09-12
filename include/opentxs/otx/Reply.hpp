// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_OTX_REPLY_HPP
#define OPENTXS_OTX_REPLY_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <memory>

#include "opentxs/Pimpl.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/core/contract/Signable.hpp"
#include "opentxs/otx/Types.hpp"

namespace opentxs
{
namespace api
{
class Core;
}  // namespace api

namespace otx
{
class Reply;
}  // namespace otx

namespace proto
{
class OTXPush;
class ServerReply;
}  // namespace proto

using OTXReply = Pimpl<otx::Reply>;
}  // namespace opentxs

namespace opentxs
{
namespace otx
{
class OPENTXS_EXPORT Reply : virtual public opentxs::contract::Signable
{
public:
    static const VersionNumber DefaultVersion;
    static const VersionNumber MaxVersion;

    OPENTXS_NO_EXPORT static auto Factory(
        const api::Core& api,
        const Nym_p signer,
        const identifier::Nym& recipient,
        const identifier::Server& server,
        const otx::ServerReplyType type,
        const RequestNumber number,
        const bool success,
        const PasswordPrompt& reason,
        std::shared_ptr<const proto::OTXPush>&& push = {})
        -> Pimpl<opentxs::otx::Reply>;
    static auto Factory(
        const api::Core& api,
        const Nym_p signer,
        const identifier::Nym& recipient,
        const identifier::Server& server,
        const otx::ServerReplyType type,
        const RequestNumber number,
        const bool success,
        const PasswordPrompt& reason,
        opentxs::otx::OTXPushType pushtype,
        const std::string& payload) -> Pimpl<opentxs::otx::Reply>;
    OPENTXS_NO_EXPORT static auto Factory(
        const api::Core& api,
        const proto::ServerReply serialized) -> Pimpl<opentxs::otx::Reply>;
    static auto Factory(const api::Core& api, const ReadView& view)
        -> Pimpl<opentxs::otx::Reply>;

    virtual auto Number() const -> RequestNumber = 0;
    OPENTXS_NO_EXPORT virtual auto Push() const
        -> std::shared_ptr<const proto::OTXPush> = 0;
    virtual auto Recipient() const -> const identifier::Nym& = 0;
    using Signable::Serialize;
    virtual auto Serialize(AllocateOutput destination) const -> bool = 0;
    OPENTXS_NO_EXPORT virtual auto Serialize(
        proto::ServerReply& serialized) const -> bool = 0;
    virtual auto Server() const -> const identifier::Server& = 0;
    virtual auto Success() const -> bool = 0;
    virtual auto Type() const -> otx::ServerReplyType = 0;

    ~Reply() override = default;

protected:
    Reply() = default;

private:
    friend OTXReply;

#ifndef _WIN32
    auto clone() const noexcept -> Reply* override = 0;
#endif

    Reply(const Reply&) = delete;
    Reply(Reply&&) = delete;
    auto operator=(const Reply&) -> Reply& = delete;
    auto operator=(Reply&&) -> Reply& = delete;
};
}  // namespace otx
}  // namespace opentxs
#endif
