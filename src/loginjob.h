/*
    SPDX-FileCopyrightText: 2009 Kevin Ottens <ervin@kde.org>
    SPDX-FileCopyrightText: 2009 Andras Mantia <amantia@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "kimap_export.h"

#include "job.h"

namespace KIMAP
{
class Session;
struct Response;
class LoginJobPrivate;

class KIMAP_EXPORT LoginJob : public Job
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(LoginJob)

    friend class SessionPrivate;

public:
    enum EncryptionMode {
        Unencrypted = 0,
        SSLorTLS, /*!< Use SSL/TLS encryption, KIMAP will automatically negoatiate
                       the best supported encryption protocol. */
        STARTTLS /*!< Use STARTTLS to upgrade an initially plaintext connection to
                      encrypted connection. KIMAP will automatically negoatiate
                      the best supported encryption protocol. */
    };

    enum AuthenticationMode { ClearText = 0, Login, Plain, CramMD5, DigestMD5, NTLM, GSSAPI, Anonymous, XOAuth2 };

    enum ErrorCode {
        ERR_COULD_NOT_CONNECT = KJob::UserDefinedError + 23 // same as in kio
    };

    explicit LoginJob(Session *session);
    ~LoginJob() override;

    Q_REQUIRED_RESULT QString userName() const;
    void setUserName(const QString &userName);

    /**
     * Get the authorization identity.
     * @since 4.10
     */
    Q_REQUIRED_RESULT QString authorizationName() const;

    /**
     * Set the authorization identity.
     *
     * If set, proxy-authentication according to RFC4616 will be used.
     *
     * Note that this feature only works with the "PLAIN" AuthenticationMode.
     *
     * The @param authorizationName will be used together with the password() to get authenticated as userName() by the authorization of the provided
     * credentials. This allows to login as a user using the admin credentials and the users name.
     * @since 4.10
     */
    void setAuthorizationName(const QString &authorizationName);

    Q_REQUIRED_RESULT QString password() const;
    void setPassword(const QString &password);

    /**
     * Returns the server greeting, in case of a successful login.
     * If the login wasn't successful, this method returns an empty string. Use errorString() to
     * get the error message in this case.
     *
     * Note that the content of this response is not defined by the IMAP protocol and is
     * implementation-dependent.
     * @since 4.7
     */
    Q_REQUIRED_RESULT QString serverGreeting() const;

    /**
     * Set the encryption mode for the connection. In case an encryption mode is set, the caller
     * MUST check the encryptionMode() result after executing the job, to see if the connection is
     * encrypted or not (e.g handshaking failed).
     * @param mode the encryption mode, see EncryptionModes
     */
    void setEncryptionMode(EncryptionMode mode);

    /**
      Get the encryption mode.
      @return the currently active encryption mode
    */
    Q_REQUIRED_RESULT EncryptionMode encryptionMode();

    void setAuthenticationMode(AuthenticationMode mode);

protected:
    void doStart() override;
    void handleResponse(const Response &response) override;
    void connectionLost() override;

private:
    Q_PRIVATE_SLOT(d_func(), void sslResponse(bool))
};

}

