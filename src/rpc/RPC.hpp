// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/rpc/AccountEventType.hpp"
// IWYU pragma: no_include "opentxs/rpc/ResponseCode.hpp"

#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
#include <iosfwd>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <tuple>
#include <vector>

#include "Proto.hpp"
#include "internal/rpc/RPC.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/api/client/OTX.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Lockable.hpp"
#include "opentxs/core/Message.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/network/zeromq/ListenCallback.hpp"
#include "opentxs/network/zeromq/socket/Publish.hpp"
#include "opentxs/network/zeromq/socket/Pull.hpp"
#include "opentxs/network/zeromq/socket/Subscribe.hpp"
#include "opentxs/protobuf/RPCEnums.pb.h"
#include "opentxs/protobuf/RPCResponse.pb.h"
#include "opentxs/rpc/Types.hpp"
#include "opentxs/rpc/request/Base.hpp"
#include "opentxs/rpc/response/Base.hpp"

namespace opentxs
{
namespace api
{
namespace client
{
class Manager;
}  // namespace client

namespace server
{
class Manager;
}  // namespace server

class Context;
class Core;
}  // namespace api

namespace identifier
{
class Server;
class UnitDefinition;
}  // namespace identifier

namespace network
{
namespace zeromq
{
class Message;
}  // namespace zeromq
}  // namespace network

namespace proto
{
class APIArgument;
class RPCCommand;
class RPCResponse;
class TaskComplete;
}  // namespace proto

namespace rpc
{
namespace request
{
class SendPayment;
}  // namespace request

class AccountData;
}  // namespace rpc

class Factory;
class Identifier;
class Options;
}  // namespace opentxs

namespace zmq = opentxs::network::zeromq;

namespace opentxs::rpc::implementation
{
class RPC final : virtual public rpc::internal::RPC, Lockable
{
public:
    auto Process(const proto::RPCCommand& command) const
        -> proto::RPCResponse final;
    auto Process(const request::Base& command) const
        -> std::unique_ptr<response::Base> final;

    ~RPC() final;

private:
    friend opentxs::Factory;

    using Args = const ::google::protobuf::RepeatedPtrField<
        ::opentxs::proto::APIArgument>;
    using TaskID = std::string;
    using Future = api::client::OTX::Future;
    using Result = api::client::OTX::Result;
    using Finish =
        std::function<void(const Result& result, proto::TaskComplete& output)>;
    using TaskData = std::tuple<Future, Finish, OTNymID>;

    const api::Context& ot_;
    mutable std::mutex task_lock_;
    mutable std::map<TaskID, TaskData> queued_tasks_;
    const OTZMQListenCallback task_callback_;
    const OTZMQListenCallback push_callback_;
    const OTZMQPullSocket push_receiver_;
    const OTZMQPublishSocket rpc_publisher_;
    const OTZMQSubscribeSocket task_subscriber_;

    static void add_output_status(
        proto::RPCResponse& output,
        proto::RPCResponseCode code);
    static void add_output_status(
        proto::TaskComplete& output,
        proto::RPCResponseCode code);
    static void add_output_identifier(
        const std::string& id,
        proto::RPCResponse& output);
    static void add_output_identifier(
        const std::string& id,
        proto::TaskComplete& output);
    static void add_output_task(
        proto::RPCResponse& output,
        const std::string& taskid);
    static auto get_account_event_type(
        StorageBox storagebox,
        Amount amount) noexcept -> rpc::AccountEventType;
    static auto get_args(const Args& serialized) -> Options;
    static auto get_index(std::int32_t instance) -> std::size_t;
    static auto init(const proto::RPCCommand& command) -> proto::RPCResponse;
    static auto invalid_command(const proto::RPCCommand& command)
        -> proto::RPCResponse;

    auto accept_pending_payments(const proto::RPCCommand& command) const
        -> proto::RPCResponse;
    auto add_claim(const proto::RPCCommand& command) const
        -> proto::RPCResponse;
    auto add_contact(const proto::RPCCommand& command) const
        -> proto::RPCResponse;
    auto client_session(const request::Base& command) const noexcept(false)
        -> const api::client::Manager&;
    auto create_account(const proto::RPCCommand& command) const
        -> proto::RPCResponse;
    auto create_compatible_account(const proto::RPCCommand& command) const
        -> proto::RPCResponse;
    auto create_issuer_account(const proto::RPCCommand& command) const
        -> proto::RPCResponse;
    auto create_nym(const proto::RPCCommand& command) const
        -> proto::RPCResponse;
    auto create_unit_definition(const proto::RPCCommand& command) const
        -> proto::RPCResponse;
    auto delete_claim(const proto::RPCCommand& command) const
        -> proto::RPCResponse;
    void evaluate_deposit_payment(
        const api::client::Manager& client,
        const api::client::OTX::Result& result,
        proto::TaskComplete& output) const;
    void evaluate_move_funds(
        const api::client::Manager& client,
        const api::client::OTX::Result& result,
        proto::RPCResponse& output) const;
    template <typename T>
    void evaluate_register_account(
        const api::client::OTX::Result& result,
        T& output) const;
    template <typename T>
    void evaluate_register_nym(
        const api::client::OTX::Result& result,
        T& output) const;
    auto evaluate_send_payment_cheque(
        const api::client::OTX::Result& result,
        proto::TaskComplete& output) const noexcept -> void;
    auto evaluate_send_payment_transfer(
        const api::client::Manager& api,
        const api::client::OTX::Result& result,
        proto::TaskComplete& output) const noexcept -> void;
    auto evaluate_transaction_reply(
        const api::client::Manager& api,
        const Message& reply) const noexcept -> bool;
    template <typename T>
    void evaluate_transaction_reply(
        const api::client::Manager& client,
        const Message& reply,
        T& output,
        const proto::RPCResponseCode code =
            proto::RPCRESPONSE_TRANSACTION_FAILED) const;
    auto get_client(std::int32_t instance) const -> const api::client::Manager*;
    auto get_account_activity(const request::Base& command) const
        -> std::unique_ptr<response::Base>;
    auto get_account_balance(const request::Base& command) const noexcept
        -> std::unique_ptr<response::Base>;
    auto get_account_balance_blockchain(
        const request::Base& base,
        const std::size_t index,
        const Identifier& accountID,
        std::vector<AccountData>& balances,
        response::Base::Responses& codes) const noexcept -> void;
    auto get_account_balance_custodial(
        const api::Core& api,
        const std::size_t index,
        const Identifier& accountID,
        std::vector<AccountData>& balances,
        response::Base::Responses& codes) const noexcept -> void;
    auto get_compatible_accounts(const proto::RPCCommand& command) const
        -> proto::RPCResponse;
    auto get_nyms(const proto::RPCCommand& command) const -> proto::RPCResponse;
    auto get_pending_payments(const proto::RPCCommand& command) const
        -> proto::RPCResponse;
    auto get_seeds(const proto::RPCCommand& command) const
        -> proto::RPCResponse;
    auto get_server(std::int32_t instance) const -> const api::server::Manager*;
    auto get_server_admin_nym(const proto::RPCCommand& command) const
        -> proto::RPCResponse;
    auto get_server_contracts(const proto::RPCCommand& command) const
        -> proto::RPCResponse;
    auto get_server_password(const proto::RPCCommand& command) const
        -> proto::RPCResponse;
    auto get_session(std::int32_t instance) const -> const api::Core&;
    auto get_transaction_data(const proto::RPCCommand& command) const
        -> proto::RPCResponse;
    auto get_unit_definitions(const proto::RPCCommand& command) const
        -> proto::RPCResponse;
    auto get_workflow(const proto::RPCCommand& command) const
        -> proto::RPCResponse;
    auto immediate_create_account(
        const api::client::Manager& client,
        const identifier::Nym& owner,
        const identifier::Server& notary,
        const identifier::UnitDefinition& unit) const -> bool;
    auto immediate_register_issuer_account(
        const api::client::Manager& client,
        const identifier::Nym& owner,
        const identifier::Server& notary) const -> bool;
    auto immediate_register_nym(
        const api::client::Manager& client,
        const identifier::Server& notary) const -> bool;
    auto import_seed(const proto::RPCCommand& command) const
        -> proto::RPCResponse;
    auto import_server_contract(const proto::RPCCommand& command) const
        -> proto::RPCResponse;
    auto is_blockchain_account(const request::Base& base, const Identifier& id)
        const noexcept -> bool;
    auto is_client_session(std::int32_t instance) const -> bool;
    auto is_server_session(std::int32_t instance) const -> bool;
    auto is_session_valid(std::int32_t instance) const -> bool;
    auto list_accounts(const request::Base& command) const noexcept
        -> std::unique_ptr<response::Base>;
    auto list_contacts(const proto::RPCCommand& command) const
        -> proto::RPCResponse;
    auto list_client_sessions(const proto::RPCCommand& command) const
        -> proto::RPCResponse;
    auto list_seeds(const proto::RPCCommand& command) const
        -> proto::RPCResponse;
    auto list_nyms(const request::Base& command) const noexcept
        -> std::unique_ptr<response::Base>;
    auto list_server_contracts(const proto::RPCCommand& command) const
        -> proto::RPCResponse;
    auto list_server_sessions(const proto::RPCCommand& command) const
        -> proto::RPCResponse;
    auto list_unit_definitions(const proto::RPCCommand& command) const
        -> proto::RPCResponse;
    auto lookup_account_id(const proto::RPCCommand& command) const
        -> proto::RPCResponse;
    auto move_funds(const proto::RPCCommand& command) const
        -> proto::RPCResponse;
    [[deprecated]] auto queue_task(
        const identifier::Nym& nymID,
        const std::string taskID,
        Finish&& finish,
        Future&& future,
        proto::RPCResponse& output) const -> void;
    auto queue_task(
        const api::Core& api,
        const identifier::Nym& nymID,
        const std::string taskID,
        Finish&& finish,
        Future&& future) const noexcept -> std::string;
    auto register_nym(const proto::RPCCommand& command) const
        -> proto::RPCResponse;
    auto rename_account(const proto::RPCCommand& command) const
        -> proto::RPCResponse;
    auto send_payment(const request::Base& command) const noexcept
        -> std::unique_ptr<response::Base>;
    auto send_payment_blockchain(
        const api::client::Manager& api,
        const request::SendPayment& command) const noexcept
        -> std::unique_ptr<response::Base>;
    auto send_payment_custodial(
        const api::client::Manager& api,
        const request::SendPayment& command) const noexcept
        -> std::unique_ptr<response::Base>;
    auto session(const request::Base& command) const noexcept(false)
        -> const api::Core&;
    auto start_client(const proto::RPCCommand& command) const
        -> proto::RPCResponse;
    auto start_server(const proto::RPCCommand& command) const
        -> proto::RPCResponse;
    auto status(const response::Base::Identifiers& ids) const noexcept
        -> ResponseCode;

    void task_handler(const zmq::Message& message);

    RPC(const api::Context& native);
    RPC() = delete;
    RPC(const RPC&) = delete;
    RPC(RPC&&) = delete;
    auto operator=(const RPC&) -> RPC& = delete;
    auto operator=(RPC&&) -> RPC& = delete;
};
}  // namespace opentxs::rpc::implementation
