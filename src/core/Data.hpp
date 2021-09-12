// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include "opentxs/Bytes.hpp"
#include "opentxs/core/Data.hpp"

namespace opentxs
{
class Armored;
}  // namespace opentxs

namespace opentxs::implementation
{
class Data : virtual public opentxs::Data
{
public:
    auto operator==(const opentxs::Data& rhs) const noexcept -> bool final;
    auto operator!=(const opentxs::Data& rhs) const noexcept -> bool final;
    auto operator<(const opentxs::Data& rhs) const noexcept -> bool final;
    auto operator>(const opentxs::Data& rhs) const noexcept -> bool final;
    auto operator<=(const opentxs::Data& rhs) const noexcept -> bool final;
    auto operator>=(const opentxs::Data& rhs) const noexcept -> bool final;
    auto operator+=(const opentxs::Data& rhs) -> Data& final;
    auto operator+=(const std::uint8_t rhs) -> Data& final;
    auto operator+=(const std::uint16_t rhs) -> Data& final;
    auto operator+=(const std::uint32_t rhs) -> Data& final;
    auto operator+=(const std::uint64_t rhs) -> Data& final;

    auto asHex() const -> std::string final;
    auto at(const std::size_t position) const -> const std::byte& final
    {
        return reinterpret_cast<const std::byte&>(data_.at(position));
    }
    auto begin() const -> const_iterator final
    {
        return const_iterator(this, 0);
    }
    auto Bytes() const noexcept -> ReadView final
    {
        return ReadView{
            reinterpret_cast<const char*>(data_.data()), data_.size()};
    }
    auto cbegin() const -> const_iterator final
    {
        return const_iterator(this, 0);
    }
    auto cend() const -> const_iterator final
    {
        return const_iterator(this, data_.size());
    }
    auto empty() const -> bool final { return data_.empty(); }
    auto data() const -> const void* final { return data_.data(); }
    auto end() const -> const_iterator final
    {
        return const_iterator(this, data_.size());
    }
    auto Extract(
        const std::size_t amount,
        opentxs::Data& output,
        const std::size_t pos) const -> bool final;
    auto Extract(std::uint8_t& output, const std::size_t pos) const
        -> bool final;
    auto Extract(std::uint16_t& output, const std::size_t pos) const
        -> bool final;
    auto Extract(std::uint32_t& output, const std::size_t pos) const
        -> bool final;
    auto Extract(std::uint64_t& output, const std::size_t pos) const
        -> bool final;
    auto IsEmpty() const -> bool final { return empty(); }
    auto IsNull() const -> bool final;
    auto GetPointer() const -> const void* final { return data_.data(); }
    auto GetSize() const -> std::size_t final { return size(); }
    auto size() const -> std::size_t final { return data_.size(); }

    void Assign(const opentxs::Data& source) final;
    void Assign(ReadView source) final { Assign(source.data(), source.size()); }
    void Assign(const void* data, const std::size_t& size) final;
    auto at(const std::size_t position) -> std::byte& final
    {
        return reinterpret_cast<std::byte&>(data_.at(position));
    }
    auto begin() -> iterator final { return iterator(this, 0); }
    void Concatenate(const ReadView data) final
    {
        Concatenate(data.data(), data.size());
    }
    void Concatenate(const void* data, const std::size_t& size) final;
    auto data() -> void* final { return data_.data(); }
    auto DecodeHex(const std::string& hex) -> bool final;
    auto end() -> iterator final { return iterator(this, data_.size()); }
    auto Randomize(const std::size_t& size) -> bool final;
    void Release() final;
    void resize(const std::size_t size) final { data_.resize(size); }
    void SetSize(const std::size_t size) final;
    auto str() const -> std::string override;
    void swap(opentxs::Data&& rhs) final;
    auto WriteInto() noexcept -> AllocateOutput final;
    void zeroMemory() final;

    ~Data() override = default;

protected:
    using Vector = std::vector<std::uint8_t>;

    Vector data_;

    void Initialize();

    Data() noexcept;
    explicit Data(const void* data, std::size_t size) noexcept;
    explicit Data(const Armored& source) noexcept;
    explicit Data(const Vector& sourceVector) noexcept;
    explicit Data(const std::vector<std::byte>& sourceVector) noexcept;

private:
    friend opentxs::Data;

    auto clone() const -> Data* override { return new Data(this->data_); }

    auto check_sub(const std::size_t pos, const std::size_t target) const
        -> bool;
    auto concatenate(const Vector& data) -> void;
    auto spaceship(const opentxs::Data& rhs) const noexcept -> int;

    Data(const Data& rhs) = delete;
    Data(Data&& rhs) = delete;
    auto operator=(const Data& rhs) -> Data& = delete;
    auto operator=(Data&& rhs) -> Data& = delete;
};
}  // namespace opentxs::implementation
