// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CORE_MESSAGE_HPP
#define OPENTXS_CORE_MESSAGE_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <irrxml/irrXML.hpp>
#include <cstdint>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <unordered_map>
#include <utility>

#include "opentxs/Types.hpp"
#include "opentxs/core/Armored.hpp"
#include "opentxs/core/Contract.hpp"
#include "opentxs/core/NumList.hpp"
#include "opentxs/core/String.hpp"

namespace opentxs
{
namespace api
{
namespace implementation
{
class Factory;
}  // namespace implementation

class Core;
}  // namespace api

namespace identity
{
class Nym;
}  // namespace identity

namespace otx
{
namespace context
{
class Base;
class Server;
}  // namespace context
}  // namespace otx

class Message;
class PasswordPrompt;
class Tag;
}  // namespace opentxs

namespace opentxs
{
class OTMessageStrategy
{
public:
    virtual auto processXml(Message& message, irr::io::IrrXMLReader*& xml)
        -> std::int32_t = 0;
    virtual void writeXml(Message& message, Tag& parent) = 0;
    virtual ~OTMessageStrategy();

    void processXmlSuccess(Message& m, irr::io::IrrXMLReader*& xml);
};

class OTMessageStrategyManager
{
public:
    auto findStrategy(std::string name) -> OTMessageStrategy*
    {
        auto strategy = mapping.find(name);
        if (strategy == mapping.end()) return nullptr;
        return strategy->second.get();
    }
    void registerStrategy(std::string name, OTMessageStrategy* strategy)
    {
        mapping[name] = std::unique_ptr<OTMessageStrategy>(strategy);
    }

    OTMessageStrategyManager()
        : mapping()
    {
    }

private:
    std::unordered_map<std::string, std::unique_ptr<OTMessageStrategy>> mapping;
};

class OPENTXS_EXPORT Message final : public Contract
{
protected:
    auto ProcessXMLNode(irr::io::IrrXMLReader*& xml) -> std::int32_t final;

    void UpdateContents(const PasswordPrompt& reason) final;

    bool m_bIsSigned{false};

private:
    friend api::implementation::Factory;

    using TypeMap = std::map<MessageType, std::string>;
    using ReverseTypeMap = std::map<std::string, MessageType>;

    static const TypeMap message_names_;
    static const ReverseTypeMap message_types_;
    static const std::map<MessageType, MessageType> reply_message_;

    static auto make_reverse_map() -> ReverseTypeMap;
    static auto reply_command(const MessageType& type) -> MessageType;

    Message(const api::Core& api);

    auto updateContentsByType(Tag& parent) -> bool;

    auto processXmlNodeAckReplies(Message& m, irr::io::IrrXMLReader*& xml)
        -> std::int32_t;
    auto processXmlNodeAcknowledgedReplies(
        Message& m,
        irr::io::IrrXMLReader*& xml) -> std::int32_t;
    auto processXmlNodeNotaryMessage(Message& m, irr::io::IrrXMLReader*& xml)
        -> std::int32_t;

public:
    static auto Command(const MessageType type) -> std::string;
    static auto Type(const std::string& type) -> MessageType;
    static auto ReplyCommand(const MessageType type) -> std::string;

    ~Message() final;

    auto VerifyContractID() const -> bool final;

    auto SignContract(const identity::Nym& theNym, const PasswordPrompt& reason)
        -> bool final;
    auto VerifySignature(const identity::Nym& theNym) const -> bool final;

    auto HarvestTransactionNumbers(
        otx::context::Server& context,
        bool bHarvestingForRetry,     // false until positively asserted.
        bool bReplyWasSuccess,        // false until positively asserted.
        bool bReplyWasFailure,        // false until positively asserted.
        bool bTransactionWasSuccess,  // false until positively asserted.
        bool bTransactionWasFailure) const -> bool;  // false until positively
                                                     // asserted.

    // So the message can get the list of numbers from the Nym, before sending,
    // that should be listed as acknowledged that the server reply has already
    // been seen for those request numbers.
    void SetAcknowledgments(const otx::context::Base& context);
    void SetAcknowledgments(const std::set<RequestNumber>& numbers);

    static void registerStrategy(std::string name, OTMessageStrategy* strategy);

    OTString m_strCommand;   // perhaps @register is the string for "reply to
                             // register" a-ha
    OTString m_strNotaryID;  // This is sent with every message for security
                             // reasons.
    OTString m_strNymID;  // The hash of the user's public key... or x509 cert.
    OTString m_strNymboxHash;  // Sometimes in a server reply as FYI, sometimes
                               // in user message for validation purposes.
    OTString m_strInboxHash;   // Sometimes in a server reply as FYI, sometimes
                               // in user message for validation purposes.
    OTString m_strOutboxHash;  // Sometimes in a server reply as FYI, sometimes
                               // in user message for validation purposes.
    OTString m_strNymID2;  // If the user requests public key of another user.
                           // ALSO used for MARKET ID sometimes.
    OTString m_strNymPublicKey;  // The user's public key... or x509 cert.
    OTString m_strInstrumentDefinitionID;  // The hash of the contract for
                                           // whatever
                                           // digital
                                           // asset is referenced.
    OTString m_strAcctID;                  // The unique ID of an asset account.
    OTString m_strType;                    // .
    OTString m_strRequestNum;  // Every user has a request number. This prevents
                               // messages from
                               // being intercepted and repeated by attackers.

    OTArmored m_ascInReferenceTo;  // If the server responds to a user
                                   // command, he sends
    // it back to the user here in ascii armored format.
    OTArmored m_ascPayload;  // If the reply needs to include a payload (such
                             // as a new account
    // or a message envelope or request from another user etc) then
    // it can be put here in ascii-armored format.
    OTArmored m_ascPayload2;  // Sometimes one payload just isn't enough.
    OTArmored m_ascPayload3;  // Sometimes two payload just isn't enough.

    // This list of request numbers is stored for optimization, so client/server
    // can communicate about
    // which messages have been received, and can avoid certain downloads, such
    // as replyNotice Box Receipts.
    //
    NumList m_AcknowledgedReplies;  // Client request: list of server replies
                                    // client has already seen.
    // Server reply:   list of client-acknowledged replies (so client knows that
    // server knows.)

    std::int64_t m_lNewRequestNum{0};  // If you are SENDING a message, you set
                                       // m_strRequestNum. (For all msgs.)
    // Server Reply for all messages copies that same number into
    // m_strRequestNum;
    // But if this is a SERVER REPLY to the "getRequestNumber" MESSAGE, the
    // "request number" expected in that reply is stored HERE in
    // m_lNewRequestNum;
    std::int64_t m_lDepth{0};  // For Market-related messages... (Plus for usage
                               // credits.) Also used by getBoxReceipt
    std::int64_t m_lTransactionNum{0};  // For Market-related messages... Also
                                        // used by getBoxReceipt

    std::uint8_t enum_{0};
    std::uint32_t enum2_{0};

    bool m_bSuccess{false};  // When the server replies to the client, this may
                             // be true or false
    bool m_bBool{false};  // Some commands need to send a bool. This variable is
                          // for those.
    std::int64_t m_lTime{0};  // Timestamp when the message was signed.

    static OTMessageStrategyManager messageStrategyManager;
};

class RegisterStrategy
{
public:
    RegisterStrategy(std::string name, OTMessageStrategy* strategy)
    {
        Message::registerStrategy(name, strategy);
    }
};

}  // namespace opentxs

#endif
