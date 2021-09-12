// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <boost/multiprecision/cpp_bin_float.hpp>
#include <boost/multiprecision/cpp_int.hpp>
#include <string>

#include "opentxs/blockchain/Work.hpp"

namespace opentxs
{
class Factory;
}  // namespace opentxs

namespace bmp = boost::multiprecision;

namespace opentxs::blockchain::implementation
{
class Work final : public blockchain::Work
{
public:
    using Type = bmp::cpp_bin_float_double;

    auto operator==(const blockchain::Work& rhs) const noexcept -> bool final;
    auto operator!=(const blockchain::Work& rhs) const noexcept -> bool final;
    auto operator<(const blockchain::Work& rhs) const noexcept -> bool final;
    auto operator<=(const blockchain::Work& rhs) const noexcept -> bool final;
    auto operator>(const blockchain::Work& rhs) const noexcept -> bool final;
    auto operator>=(const blockchain::Work& rhs) const noexcept -> bool final;
    auto operator+(const blockchain::Work& rhs) const noexcept -> OTWork final;

    auto asHex() const noexcept -> std::string final;
    auto Decimal() const noexcept -> std::string final { return data_.str(); }

    Work(Type&& data) noexcept;
    Work() noexcept;
    Work(const Work& rhs) noexcept;

    ~Work() final = default;

private:
    friend opentxs::Factory;

    Type data_;

    auto clone() const noexcept -> Work* final { return new Work(*this); }

    Work(Work&& rhs) = delete;
    auto operator=(const Work& rhs) -> Work& = delete;
    auto operator=(Work&& rhs) -> Work& = delete;
};
}  // namespace opentxs::blockchain::implementation
