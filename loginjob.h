/*
    Copyright (c) 2009 Kevin Ottens <ervin@kde.org>
    Copyright (c) 2009 Andras Mantia <amantia@kde.org>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

#ifndef KIMAP_LOGINJOB_H
#define KIMAP_LOGINJOB_H

#include "kimap_export.h"

#include "job.h"

namespace KIMAP {

class Session;
struct Message;
class LoginJobPrivate;

class KIMAP_EXPORT LoginJob : public Job
{
  Q_OBJECT
  Q_DECLARE_PRIVATE(LoginJob)

  friend class SessionPrivate;

  public:
    enum EncryptionMode {
        Unencrypted = 0,
        TlsV1,
        SslV2,
        SslV3,
        SslV3_1,
        AnySslVersion
    };

    enum AuthenticationMode {
        ClearText = 0,
        Login,
        Plain,
        CramMD5,
        DigestMD5,
        NTLM,
        GSSAPI,
        Anonymous
    };

    explicit LoginJob( Session *session );
    virtual ~LoginJob();

    QString userName() const;
    void setUserName( const QString &userName );

    QString password() const;
    void setPassword( const QString &password );

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
    EncryptionMode encryptionMode();

    void setAuthenticationMode(AuthenticationMode mode);

  protected:
    virtual void doStart();
    virtual void handleResponse( const Message &response );
    virtual void connectionLost();

  private:
    Q_PRIVATE_SLOT( d_func(), void sslResponse(bool) )
};

}

#endif
