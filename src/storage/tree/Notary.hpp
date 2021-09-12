// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstdint>
#include <map>
#include <memory>
#include <string>

#include "Proto.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/api/Editor.hpp"
#include "opentxs/api/storage/Storage.hpp"
#include "opentxs/protobuf/SpentTokenList.pb.h"
#include "opentxs/protobuf/StorageNotary.pb.h"
#include "storage/tree/Node.hpp"

namespace opentxs
{
namespace api
{
namespace storage
{
class Driver;
}  // namespace storage
}  // namespace api

namespace identifier
{
class UnitDefinition;
}  // namespace identifier

namespace storage
{
class Tree;
}  // namespace storage
}  // namespace opentxs

namespace opentxs::storage
{
class Notary final : public Node
{
public:
    using MintSeries = std::uint64_t;

#if OT_CASH
    auto CheckSpent(
        const identifier::UnitDefinition& unit,
        const MintSeries series,
        const std::string& key) const -> bool;

    auto MarkSpent(
        const identifier::UnitDefinition& unit,
        const MintSeries series,
        const std::string& key) -> bool;
#endif

    ~Notary() final = default;

private:
    friend Tree;
    using SeriesMap = std::map<MintSeries, std::string>;
    using UnitMap = std::map<std::string, SeriesMap>;

    std::string id_;

#if OT_CASH
    mutable UnitMap mint_map_;

    auto create_list(
        const std::string& unitID,
        const MintSeries series,
        std::shared_ptr<proto::SpentTokenList>& output) const -> std::string;
    auto get_or_create_list(
        const Lock& lock,
        const std::string& unitID,
        const MintSeries series) const -> proto::SpentTokenList;
#endif
    auto save(const Lock& lock) const -> bool final;
    auto serialize() const -> proto::StorageNotary;

    void init(const std::string& hash) final;

    Notary(
        const opentxs::api::storage::Driver& storage,
        const std::string& key,
        const std::string& id);
    Notary() = delete;
    Notary(const Notary&) = delete;
    Notary(Notary&&) = delete;
    auto operator=(const Notary&) -> Notary = delete;
    auto operator=(Notary&&) -> Notary = delete;
};
}  // namespace opentxs::storage
