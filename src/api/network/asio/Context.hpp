// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

namespace boost
{
namespace asio
{
class io_context;
}  // namespace asio
}  // namespace boost

namespace opentxs::api::network::asio
{
class Context
{
public:
    operator const boost::asio::io_context&() const noexcept
    {
        return const_cast<Context&>(*this).operator boost::asio::io_context&();
    }
    auto get() const noexcept -> const boost::asio::io_context&
    {
        return const_cast<Context&>(*this).get();
    }

    operator boost::asio::io_context&() noexcept;
    auto get() noexcept -> boost::asio::io_context& { return *this; }
    auto Init(unsigned int threads) noexcept -> bool;
    auto Stop() noexcept -> void;

    Context() noexcept;

    ~Context();

private:
    struct Imp;

    Imp* imp_;

    Context(const Context&) = delete;
    Context(Context&&) = delete;
    Context& operator=(const Context&) = delete;
    Context& operator=(Context&&) = delete;
};
}  // namespace opentxs::api::network::asio
