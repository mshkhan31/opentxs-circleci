// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CORE_CRYPTO_OTSIGNEDFILE_HPP
#define OPENTXS_CORE_CRYPTO_OTSIGNEDFILE_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <irrxml/irrXML.hpp>
#include <cstdint>

#include "opentxs/core/Contract.hpp"
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

class PasswordPrompt;

class OPENTXS_EXPORT OTSignedFile : public Contract
{
public:
    auto LoadFile() -> bool;
    auto SaveFile() -> bool;
    auto VerifyFile() -> bool;  // Returns true or false, whether actual
                                // subdir/file matches purported subdir/file.
    // (You should still verify the signature on it as well, if you are doing
    // this.)
    void SetFilename(const String& LOCAL_SUBDIR, const String& FILE_NAME);
    auto GetFilePayload() -> String&;
    void SetFilePayload(const String& strArg);
    auto GetSignerNymID() -> String&;
    void SetSignerNymID(const String& strArg);
    ~OTSignedFile() override;
    void Release() override;
    void Release_SignedFile();
    void UpdateContents(const PasswordPrompt& reason) override;

protected:
    OTString m_strSignedFilePayload;  // This class exists to wrap another and
                                      // save it in signed form.
    // The "payload" (the wrapped contents) are stored in this member.

    OTString m_strLocalDir;  // The local subdirectory where the file is, such
                             // as "nyms" or "certs"
    OTString m_strSignedFilename;  // The file stores its own name. Later, when
                                   // loading it back up, you can
    // see that the name matches internally, and that the signature matches,
    // therefore, no one has switched the file or meddled with its contents.

    OTString m_strPurportedLocalDir;  // This is the subdirectory according to
                                      // the file.
    OTString m_strPurportedFilename;  // This is the filename according to the
                                      // file.

    OTString m_strSignerNymID;  // Optional. Here in case you ever
                                // want to use it.

    // THOUGHT: What if someone switched the file for an older version of
    // itself? Seems to me that he could
    // make the server accept the file, in that case. Like maybe an account file
    // with a higher balance?
    // Similarly, what if someone erased a spent token file? Then the software
    // would accept it as a new
    // token once again. Also, the cash account would be deducted twice for the
    // same token, which means it
    // would no longer contain enough to cover all the tokens...
    // Therefore it seems to me that, even with the files signed, there are
    // still attacks possible when
    // the attacker has write/erase access to the filesystem. I'd like to make
    // it impervious even to that.

    auto ProcessXMLNode(irr::io::IrrXMLReader*& xml) -> std::int32_t override;

private:  // Private prevents erroneous use by other classes.
    friend api::implementation::Factory;

    using ot_super = Contract;

    // These assume SetFilename() was already called,
    // or at least one of the constructors that uses it.
    //
    explicit OTSignedFile(const api::Core& core);
    explicit OTSignedFile(
        const api::Core& core,
        const String& LOCAL_SUBDIR,
        const String& FILE_NAME);
    explicit OTSignedFile(
        const api::Core& core,
        const char* LOCAL_SUBDIR,
        const String& FILE_NAME);
    explicit OTSignedFile(
        const api::Core& core,
        const char* LOCAL_SUBDIR,
        const char* FILE_NAME);

    OTSignedFile() = delete;
};
}  // namespace opentxs
#endif
