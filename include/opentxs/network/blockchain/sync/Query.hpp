// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_NETWORK_BLOCKCHAIN_SYNC_QUERY_HPP
#define OPENTXS_NETWORK_BLOCKCHAIN_SYNC_QUERY_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <vector>

#include "opentxs/network/blockchain/sync/Base.hpp"

namespace opentxs
{
namespace network
{
namespace blockchain
{
namespace sync
{
class OPENTXS_EXPORT Query final : public Base
{
public:
    Query(int) noexcept;
    OPENTXS_NO_EXPORT Query() noexcept;

    ~Query() final;

private:
    Query(const Query&) = delete;
    Query(Query&&) = delete;
    auto operator=(const Query&) -> Query& = delete;
    auto operator=(Query&&) -> Query& = delete;
};
}  // namespace sync
}  // namespace blockchain
}  // namespace network
}  // namespace opentxs
#endif
