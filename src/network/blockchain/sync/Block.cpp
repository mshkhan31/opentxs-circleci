// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                               // IWYU pragma: associated
#include "1_Internal.hpp"                             // IWYU pragma: associated
#include "opentxs/network/blockchain/sync/Block.hpp"  // IWYU pragma: associated

#include <memory>
#include <set>
#include <stdexcept>
#include <string>
#include <string_view>

#include "Proto.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/protobuf/BlockchainP2PSync.pb.h"

namespace opentxs::network::blockchain::sync
{

struct Block::Imp {
    static constexpr auto default_version_ = VersionNumber{1};

    const opentxs::blockchain::Type chain_;
    const opentxs::blockchain::block::Height height_;
    const opentxs::blockchain::filter::Type type_;
    const std::uint32_t count_;
    const Space header_;
    const Space filter_;

    Imp(opentxs::blockchain::Type chain,
        opentxs::blockchain::block::Height height,
        opentxs::blockchain::filter::Type type,
        std::uint32_t count,
        ReadView header,
        ReadView filter) noexcept(false)
        : chain_(chain)
        , height_(height)
        , type_(type)
        , count_(count)
        , header_(space(header))
        , filter_(space(filter))
    {
        if (0 == opentxs::blockchain::DefinedChains().count(chain_)) {
            throw std::runtime_error{"invalid chain"};
        }

        if (0 == header_.size()) { throw std::runtime_error{"invalid header"}; }

        if (0 == filter_.size()) { throw std::runtime_error{"invalid filter"}; }
    }

private:
    Imp() noexcept;
    Imp(const Imp&) = delete;
    Imp(Imp&&) = delete;
    auto operator=(const Imp&) -> Imp& = delete;
    auto operator=(Imp&&) -> Imp& = delete;
};

Block::Block(const proto::BlockchainP2PSync& in) noexcept(false)
    : Block(
          static_cast<opentxs::blockchain::Type>(in.chain()),
          in.height(),
          static_cast<opentxs::blockchain::filter::Type>(in.filter_type()),
          in.filter_element_count(),
          in.header(),
          in.filter())
{
}

Block::Block(
    opentxs::blockchain::Type chain,
    opentxs::blockchain::block::Height height,
    opentxs::blockchain::filter::Type type,
    std::uint32_t count,
    ReadView header,
    ReadView filter) noexcept(false)
    : imp_(std::make_unique<Imp>(chain, height, type, count, header, filter)
               .release())
{
}

Block::Block(Block&& rhs) noexcept
    : imp_(rhs.imp_)
{
    rhs.imp_ = nullptr;
}

auto Block::Chain() const noexcept -> opentxs::blockchain::Type
{
    return imp_->chain_;
}

auto Block::Filter() const noexcept -> ReadView
{
    return reader(imp_->filter_);
}

auto Block::FilterElements() const noexcept -> std::uint32_t
{
    return imp_->count_;
}

auto Block::FilterType() const noexcept -> opentxs::blockchain::filter::Type
{
    return imp_->type_;
}

auto Block::Header() const noexcept -> ReadView
{
    return reader(imp_->header_);
}

auto Block::Height() const noexcept -> opentxs::blockchain::block::Height
{
    return imp_->height_;
}

auto Block::Serialize(proto::BlockchainP2PSync& dest) const noexcept -> bool
{
    const auto header = reader(imp_->header_);
    const auto filter = reader(imp_->filter_);
    dest.set_version(Imp::default_version_);
    dest.set_chain(static_cast<std::uint32_t>(imp_->chain_));
    dest.set_height(imp_->height_);
    dest.set_header(header.data(), header.size());
    dest.set_filter_type(static_cast<std::uint32_t>(imp_->type_));
    dest.set_filter_element_count(imp_->count_);
    dest.set_filter(filter.data(), filter.size());

    return true;
}

auto Block::Serialize(AllocateOutput dest) const noexcept -> bool
{
    const auto proto = [&] {
        auto out = proto::BlockchainP2PSync{};
        Serialize(out);

        return out;
    }();

    return proto::write(proto, dest);
}

Block::~Block() { std::unique_ptr<Imp>(imp_).reset(); }
}  // namespace opentxs::network::blockchain::sync
