// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_SHAREDPIMPL_HPP
#define OPENTXS_SHAREDPIMPL_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <cstdlib>
#include <iostream>
#include <memory>

namespace opentxs
{
template <class C>
class OPENTXS_EXPORT SharedPimpl
{
public:
    explicit SharedPimpl(const std::shared_ptr<const C>& in) noexcept
        : pimpl_(in)
    {
        if (false == bool(pimpl_)) {
            std::cout << stack_trace() << '\n';
            abort();
        }
    }
    SharedPimpl(const SharedPimpl& rhs) noexcept = default;
    SharedPimpl(SharedPimpl&& rhs) noexcept = default;
    auto operator=(const SharedPimpl& rhs) noexcept -> SharedPimpl& = default;
    auto operator=(SharedPimpl&& rhs) noexcept -> SharedPimpl& = default;

    operator const C&() const noexcept { return *pimpl_; }

    auto operator->() const -> const C* { return pimpl_.get(); }

    template <typename Type>
    auto as() noexcept -> SharedPimpl<Type>
    {
        return SharedPimpl<Type>{std::static_pointer_cast<const Type>(pimpl_)};
    }
    template <typename Type>
    auto dynamic() noexcept(false) -> SharedPimpl<Type>
    {
        auto pointer = std::dynamic_pointer_cast<const Type>(pimpl_);

        if (pointer) {
            return SharedPimpl<Type>{std::move(pointer)};
        } else {
            throw std::runtime_error("Invalid dynamic cast");
        }
    }
    auto get() const noexcept -> const C& { return *pimpl_; }

    ~SharedPimpl() = default;

private:
    std::shared_ptr<const C> pimpl_{nullptr};

    SharedPimpl() = delete;
};  // class SharedPimpl
}  // namespace opentxs
#endif
