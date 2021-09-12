// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                         // IWYU pragma: associated
#include "1_Internal.hpp"                       // IWYU pragma: associated
#include "opentxs/rpc/request/SendPayment.hpp"  // IWYU pragma: associated
#include "rpc/request/Base.hpp"                 // IWYU pragma: associated

#include <memory>

#include "internal/rpc/RPC.hpp"
#include "opentxs/protobuf/RPCCommand.pb.h"
#include "opentxs/protobuf/SendPayment.pb.h"
#include "opentxs/rpc/CommandType.hpp"
#include "opentxs/rpc/PaymentType.hpp"

namespace opentxs::rpc::request::implementation
{
struct SendPayment final : public Base::Imp {
    const VersionNumber send_payment_version_;
    const rpc::PaymentType payment_type_;
    const std::string contact_;
    const std::string source_;
    const std::string destination_;
    const std::string memo_;
    const opentxs::Amount amount_;

    auto asSendPayment() const noexcept -> const request::SendPayment& final
    {
        return static_cast<const request::SendPayment&>(*parent_);
    }
    auto serialize(proto::RPCCommand& dest) const noexcept -> bool final
    {
        if (Imp::serialize(dest)) {
            auto& payment = *dest.mutable_sendpayment();
            payment.set_version(send_payment_version_);
            payment.set_type(translate(payment_type_));
            payment.set_contact(contact_);
            payment.set_sourceaccount(source_);
            payment.set_destinationaccount(destination_);
            payment.set_memo(memo_);
            payment.set_amount(amount_);

            return true;
        }

        return false;
    }

    SendPayment(
        const request::SendPayment* parent,
        VersionNumber version,
        Base::SessionIndex session,
        rpc::PaymentType type,
        opentxs::Amount amount,
        const std::string& sourceAccount,
        const std::string& recipientContact,
        const std::string& destinationAccount,
        const std::string& memo,
        const Base::AssociateNyms& nyms) noexcept(false)
        : Imp(parent, CommandType::send_payment, version, session, nyms)
        , send_payment_version_(1u)
        , payment_type_(type)
        , contact_(recipientContact)
        , source_(sourceAccount)
        , destination_(destinationAccount)
        , memo_(memo)
        , amount_(amount)
    {
        check_session();
    }
    SendPayment(
        const request::SendPayment* parent,
        const proto::RPCCommand& in) noexcept(false)
        : Imp(parent, in)
        , send_payment_version_(in.sendpayment().version())
        , payment_type_(translate(in.sendpayment().type()))
        , contact_(in.sendpayment().contact())
        , source_(in.sendpayment().sourceaccount())
        , destination_(in.sendpayment().destinationaccount())
        , memo_(in.sendpayment().memo())
        , amount_(in.sendpayment().amount())
    {
        check_session();
    }

    ~SendPayment() final = default;

private:
    SendPayment() = delete;
    SendPayment(const SendPayment&) = delete;
    SendPayment(SendPayment&&) = delete;
    auto operator=(const SendPayment&) -> SendPayment& = delete;
    auto operator=(SendPayment&&) -> SendPayment& = delete;
};
}  // namespace opentxs::rpc::request::implementation

namespace opentxs::rpc::request
{
SendPayment::SendPayment(
    SessionIndex session,
    rpc::PaymentType type,
    const std::string& sourceAccount,
    const std::string& recipientContact,
    opentxs::Amount amount,
    const std::string& memo,
    const AssociateNyms& nyms)
    : Base(std::make_unique<implementation::SendPayment>(
          this,
          DefaultVersion(),
          session,
          type,
          amount,
          sourceAccount,
          recipientContact,
          "",
          memo,
          nyms))
{
}

SendPayment::SendPayment(
    SessionIndex session,
    const std::string& sourceAccount,
    const std::string& recipientContact,
    const std::string& destinationAccount,
    opentxs::Amount amount,
    const std::string& memo,
    const AssociateNyms& nyms)
    : Base(std::make_unique<implementation::SendPayment>(
          this,
          DefaultVersion(),
          session,
          PaymentType::transfer,
          amount,
          sourceAccount,
          recipientContact,
          destinationAccount,
          memo,
          nyms))
{
}

SendPayment::SendPayment(
    SessionIndex session,
    const std::string& sourceAccount,
    const std::string& destinationAddress,
    opentxs::Amount amount,
    const std::string& recipientContact,
    const std::string& memo,
    const AssociateNyms& nyms)
    : Base(std::make_unique<implementation::SendPayment>(
          this,
          DefaultVersion(),
          session,
          PaymentType::blockchain,
          amount,
          sourceAccount,
          recipientContact,
          destinationAddress,
          memo,
          nyms))
{
}

SendPayment::SendPayment(const proto::RPCCommand& in) noexcept(false)
    : Base(std::make_unique<implementation::SendPayment>(this, in))
{
}

SendPayment::SendPayment() noexcept
    : Base(std::make_unique<implementation::SendPayment>(
          this,
          0,
          -1,
          PaymentType::error,
          0,
          "",
          "",
          "",
          "",
          AssociateNyms{}))
{
}

auto SendPayment::Amount() const noexcept -> opentxs::Amount
{
    return static_cast<const implementation::SendPayment&>(*imp_).amount_;
}

auto SendPayment::DefaultVersion() noexcept -> VersionNumber { return 3u; }

auto SendPayment::DestinationAccount() const noexcept -> const std::string&
{
    return static_cast<const implementation::SendPayment&>(*imp_).destination_;
}

auto SendPayment::Memo() const noexcept -> const std::string&
{
    return static_cast<const implementation::SendPayment&>(*imp_).memo_;
}

auto SendPayment::PaymentType() const noexcept -> rpc::PaymentType
{
    return static_cast<const implementation::SendPayment&>(*imp_).payment_type_;
}

auto SendPayment::RecipientContact() const noexcept -> const std::string&
{
    return static_cast<const implementation::SendPayment&>(*imp_).contact_;
}

auto SendPayment::SourceAccount() const noexcept -> const std::string&
{
    return static_cast<const implementation::SendPayment&>(*imp_).source_;
}

SendPayment::~SendPayment() = default;
}  // namespace opentxs::rpc::request
