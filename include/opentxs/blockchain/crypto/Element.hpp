// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_BLOCKCHAIN_CRYPTO_ELEMENT_HPP
#define OPENTXS_BLOCKCHAIN_CRYPTO_ELEMENT_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include "opentxs/Types.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/identifier/Nym.hpp"

namespace opentxs
{
namespace blockchain
{
namespace crypto
{
namespace internal
{
struct Element;
}  // namespace internal

class Subaccount;
}  // namespace crypto
}  // namespace blockchain

class PasswordPrompt;
}  // namespace opentxs

namespace opentxs
{
namespace blockchain
{
namespace crypto
{
class OPENTXS_EXPORT Element
{
public:
    using Txids = std::vector<opentxs::blockchain::block::pTxid>;

    virtual auto Address(const AddressStyle format) const noexcept
        -> std::string = 0;
    virtual auto Confirmed() const noexcept -> Txids = 0;
    virtual auto Contact() const noexcept -> OTIdentifier = 0;
    virtual auto Index() const noexcept -> Bip32Index = 0;
    virtual auto Internal() const noexcept -> internal::Element& = 0;
    virtual auto Key() const noexcept -> ECKey = 0;
    virtual auto KeyID() const noexcept -> crypto::Key = 0;
    virtual auto Label() const noexcept -> std::string = 0;
    virtual auto LastActivity() const noexcept -> Time = 0;
    virtual auto Parent() const noexcept -> const Subaccount& = 0;
    virtual auto PrivateKey(const PasswordPrompt& reason) const noexcept
        -> ECKey = 0;
    virtual auto PubkeyHash() const noexcept -> OTData = 0;
    virtual auto Subchain() const noexcept -> crypto::Subchain = 0;
    virtual auto Unconfirmed() const noexcept -> Txids = 0;

    OPENTXS_NO_EXPORT virtual ~Element() = default;

protected:
    Element() noexcept = default;

private:
    Element(const Element&) = delete;
    Element(Element&&) = delete;
    auto operator=(const Element&) -> Element& = delete;
    auto operator=(Element&&) -> Element& = delete;
};
}  // namespace crypto
}  // namespace blockchain
}  // namespace opentxs
#endif  // OPENTXS_BLOCKCHAIN_CRYPTO_BALANCENODE_HPP
