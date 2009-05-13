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

#include "loginjob.h"

#include <KDE/KLocale>
#include <KDE/KDebug>
#include <ktcpsocket.h>

#include "job_p.h"
#include "message_p.h"
#include "session_p.h"
#include "rfccodecs.h"

#include "common.h"

extern "C" {
#include <sasl/sasl.h>
}

static sasl_callback_t callbacks[] = {
    { SASL_CB_ECHOPROMPT, NULL, NULL },
    { SASL_CB_NOECHOPROMPT, NULL, NULL },
    { SASL_CB_GETREALM, NULL, NULL },
    { SASL_CB_USER, NULL, NULL },
    { SASL_CB_AUTHNAME, NULL, NULL },
    { SASL_CB_PASS, NULL, NULL },
    { SASL_CB_CANON_USER, NULL, NULL },
    { SASL_CB_LIST_END, NULL, NULL }
};

namespace KIMAP
{
  class LoginJobPrivate : public JobPrivate
  {
    public:
     enum AuthState {
        StartTls = 0,
        Capability,
        Login,
        Authenticate
      };

      LoginJobPrivate( LoginJob *job, Session *session, const QString& name ) : JobPrivate(session, name), q(job), encryptionMode(LoginJob::Unencrypted),  authState(Login), plainLoginDisabled(false) {
        conn = 0;
        client_interact = 0;
      }
      ~LoginJobPrivate() { }
      bool sasl_interact();

      bool startAuthentication();
      bool answerChallenge(const QByteArray &data);
      void sslResponse(bool response);

      LoginJob *q;

      QString userName;
      QString password;

      LoginJob::EncryptionMode encryptionMode;
      QString authMode;
      AuthState authState;
      QStringList capabilities;
      bool plainLoginDisabled;

      sasl_conn_t *conn;
      sasl_interact_t *client_interact;
  };
}

using namespace KIMAP;

bool LoginJobPrivate::sasl_interact()
{
  kDebug() <<"sasl_interact";
  sasl_interact_t *interact = client_interact;

  //some mechanisms do not require username && pass, so it doesn't need a popup
  //window for getting this info
  for ( ; interact->id != SASL_CB_LIST_END; interact++ ) {
    if ( interact->id == SASL_CB_AUTHNAME ||
         interact->id == SASL_CB_PASS ) {
      //TODO: dialog for use name??
      break;
    }
  }

  interact = client_interact;
  while( interact->id != SASL_CB_LIST_END ) {
    kDebug() <<"SASL_INTERACT id:" << interact->id;
    switch( interact->id ) {
      case SASL_CB_USER:
      case SASL_CB_AUTHNAME:
        kDebug() <<"SASL_CB_[USER|AUTHNAME]: '" << userName <<"'";
        interact->result = strdup( userName.toUtf8() );
        interact->len = strlen( (const char *) interact->result );
        break;
      case SASL_CB_PASS:
        kDebug() <<"SASL_CB_PASS: [hidden]";
        interact->result = strdup( password.toUtf8() );
        interact->len = strlen( (const char *) interact->result );
        break;
      default:
        interact->result = 0;
        interact->len = 0;
        break;
    }
    interact++;
  }
  return true;
}


LoginJob::LoginJob( Session *session )
  : Job( *new LoginJobPrivate(this, session, i18n("Login")) )
{
  Q_D(LoginJob);
  connect(d->sessionInternal(), SIGNAL(encryptionNegotiationResult(bool)), this, SLOT(sslResponse(bool)));
}

LoginJob::~LoginJob()
{
}

QString LoginJob::userName() const
{
  Q_D(const LoginJob);
  return d->userName;
}

void LoginJob::setUserName( const QString &userName )
{
  Q_D(LoginJob);
  d->userName = userName;
}

QString LoginJob::password() const
{
  Q_D(const LoginJob);
  return d->password;
}

void LoginJob::setPassword( const QString &password )
{
  Q_D(LoginJob);
  d->password = password;
}

void LoginJob::doStart()
{
  Q_D(LoginJob);
  if (d->encryptionMode == SslV2 || d->encryptionMode == SslV3 || d->encryptionMode == SslV3_1 || d->encryptionMode == AnySslVersion) {
    KTcpSocket::SslVersion version = KTcpSocket::SslV2;
    if (d->encryptionMode == SslV3)
      version = KTcpSocket::SslV3;
    if (d->encryptionMode == SslV3_1)
      version = KTcpSocket::SslV3_1;
    if (d->encryptionMode == AnySslVersion)
      version = KTcpSocket::AnySslVersion;
    d->sessionInternal()->startSsl(version);
  } else  if (d->encryptionMode == Unencrypted ) {
    if (d->authMode.isEmpty()) {
      d->tag = d->sessionInternal()->sendCommand( "LOGIN",
                                                  quoteIMAP( d->userName ).toUtf8()
                                                 +' '
                                                 +quoteIMAP(d->password ).toUtf8() );
    } else {
      if (!d->startAuthentication()) {
        emitResult();
      }
    }
  } else if (d->encryptionMode == TlsV1) {
    d->authState = LoginJobPrivate::StartTls;
    d->tag = d->sessionInternal()->sendCommand( "STARTTLS" );
  }
}

void LoginJob::handleResponse( const Message &response )
{
  Q_D(LoginJob);

  //set the actual command name for standard responses
  QString commandName = i18n("Login");
  if (d->authState == LoginJobPrivate::Capability) {
    commandName = i18n("Capability");
  } else if (d->authState == LoginJobPrivate::StartTls) {
    commandName = i18n("StartTls");
  }

  if ( !response.content.isEmpty()
       && response.content.first().toString() == d->tag ) {
    if ( response.content.size() < 2 ) {
      setErrorText( i18n("%1 failed, malformed reply from the server.", commandName) );
      emitResult();
    } else if ( response.content[1].toString() != "OK" ) {
        //server replied with NO or BAD for SASL authentication
        if (d->authState == LoginJobPrivate::Authenticate) {
          sasl_dispose( &d->conn );
        }

        setError( UserDefinedError );
        setErrorText( i18n("%1 failed, server replied: %2", commandName, response.toString().constData()) );
        emitResult();
    } else if ( response.content[1].toString() == "OK")    {
      if (d->authState == LoginJobPrivate::Authenticate) {
        sasl_dispose( &d->conn ); //SASL authentication done
        emitResult();
      } else if (d->authState == LoginJobPrivate::Capability) {

        //cleartext login, if enabled
        if (d->authMode.isEmpty()) {
          if (d->plainLoginDisabled) {
            setError( UserDefinedError );
            setErrorText( i18n("Login failed, plain login is disabled by the server.") );
            emitResult();
          } else {
            d->authState = LoginJobPrivate::Login;
            d->tag = d->sessionInternal()->sendCommand( "LOGIN",
                                                        quoteIMAP( d->userName ).toUtf8()
                                                       +' '
                                                       +quoteIMAP( d->password ).toUtf8() );
          }
        }

        //find the selected SASL authentication method
        Q_FOREACH(QString capability, d->capabilities) {
          if (capability.startsWith("AUTH=")) {
            QString authType = capability.mid(5);
            if (authType == d->authMode) {
                if (!d->startAuthentication()) {
                  emitResult(); //problem, we're done
                }
            }
          }
        }
      } else if (d->authState == LoginJobPrivate::StartTls) {
        d->sessionInternal()->startSsl(KTcpSocket::TlsV1);
      } else {
        emitResult(); //got an OK, command done
      }
    }
  } else if ( response.content.size() >= 2 ) {
    if ( d->authState == LoginJobPrivate::Authenticate ) {
      if (!d->answerChallenge(response.content[1].toString())) {
        emitResult(); //error, we're done
      }
    } else if ( response.content[1].toString()=="CAPABILITY" ) {
      bool authModeSupported = d->authMode.isEmpty();
      for (int i = 2; i < response.content.size(); ++i) {
        QString capability = response.content[i].toString();
        d->capabilities << capability;
        if (capability == "LOGINDISABLED") {
          d->plainLoginDisabled = true;
        }
        QString authMode = capability.mid(5);
        if (authMode == d->authMode) {
          authModeSupported = true;
        }
      }
      kDebug() << "Capabilities after STARTTLS: " << d->capabilities;
      if (!authModeSupported) {
        setError( UserDefinedError );
        setErrorText( i18n("Login failed, authentication mode %1 is not supported by the server.", d->authMode) );
        d->authState = LoginJobPrivate::Login; //just to treat the upcoming OK correctly
      }
    }
  }
}

bool LoginJobPrivate::startAuthentication()
{
  //SASL authentication
  if (!initSASL()) {
    q->setError( LoginJob::UserDefinedError );
    q->setErrorText( i18n("Login failed, client cannot initialize the SASL library.") );
    return false;
  }

  authState = LoginJobPrivate::Authenticate;
  const char *out = 0;
  uint outlen = 0;
  const char *mechusing = 0;

  int result = sasl_client_new( "imap", m_session->hostName().toLatin1(), 0, 0, callbacks, 0, &conn );
  if ( result != SASL_OK ) {
    kDebug() <<"sasl_client_new failed with:" << result;
    q->setError( LoginJob::UserDefinedError );
    q->setErrorText( QString::fromUtf8( sasl_errdetail( conn ) ) );
    return false;
  }

  do {
    result = sasl_client_start(conn, authMode.toLatin1(), &client_interact, capabilities.contains("SASL-IR") ? &out : 0, &outlen, &mechusing);

    if ( result == SASL_INTERACT ) {
      if ( !sasl_interact() ) {
        sasl_dispose( &conn );
        q->setError( LoginJob::UserDefinedError ); //TODO: check up the actual error
        return false;
      }
    }
  } while ( result == SASL_INTERACT );

  if ( result != SASL_CONTINUE && result != SASL_OK ) {
    kDebug() <<"sasl_client_start failed with:" << result;
    q->setError( LoginJob::UserDefinedError );
    q->setErrorText( QString::fromUtf8( sasl_errdetail( conn ) ) );
    sasl_dispose( &conn );
    return false;
  }

  QByteArray tmp = QByteArray::fromRawData( out, outlen );
  QByteArray challenge = tmp.toBase64();

  tag = sessionInternal()->sendCommand( "AUTHENTICATE", authMode.toLatin1() + ' ' + challenge );

  return true;
}

bool LoginJobPrivate::answerChallenge(const QByteArray &data)
{
  QByteArray challenge = data;
  int result = -1;
  const char *out = 0;
  uint outlen = 0;
  do {
    result = sasl_client_step(conn, challenge.isEmpty() ? 0 : challenge.data(),
                              challenge.size(),
                              &client_interact,
                              &out, &outlen);

    if (result == SASL_INTERACT) {
      if ( !sasl_interact() ) {
        q->setError( LoginJob::UserDefinedError ); //TODO: check up the actual error
        sasl_dispose( &conn );
        return false;
      }
    }
  } while ( result == SASL_INTERACT );

  if ( result != SASL_CONTINUE && result != SASL_OK ) {
    kDebug() <<"sasl_client_step failed with:" << result;
    q->setError( LoginJob::UserDefinedError ); //TODO: check up the actual error
    q->setErrorText( QString::fromUtf8( sasl_errdetail( conn ) ) );
    sasl_dispose( &conn );
    return false;
  }

  QByteArray tmp = QByteArray::fromRawData( out, outlen );
  challenge = tmp.toBase64();

  sessionInternal()->sendData( challenge );

  return true;
}

void LoginJobPrivate::sslResponse(bool response)
{
  if (response) {
    authState = LoginJobPrivate::Capability;
    tag = sessionInternal()->sendCommand( "CAPABILITY" );
  } else {
    q->setError( LoginJob::UserDefinedError );
    q->setErrorText( i18n("Login failed, TLS negotiation failed." ));
    encryptionMode = LoginJob::Unencrypted;
    q->emitResult();
  }
}

void LoginJob::setEncryptionMode(EncryptionMode mode)
{
  Q_D(LoginJob);
  d->encryptionMode = mode;
}

LoginJob::EncryptionMode LoginJob::encryptionMode()
{
  Q_D(LoginJob);
  return d->encryptionMode;
}

void LoginJob::setAuthenticationMode(AuthenticationMode mode)
{
  Q_D(LoginJob);
  switch (mode)
  {
    case ClearText: d->authMode = "";
      break;
    case Login: d->authMode = "LOGIN";
      break;
    case Plain: d->authMode = "PLAIN";
      break;
    case CramMD5: d->authMode = "CRAM-MD5";
      break;
    case DigestMD5: d->authMode = "DIGEST-MD5";
      break;
    case GSSAPI: d->authMode = "GSSAPI";
      break;
    case Anonymous: d->authMode = "ANONYMOUS";
      break;
    default:
      d->authMode = "";
  }
}

void LoginJob::connectionLost()
{
  Q_D(LoginJob);

  //don't emit the result if the connection was lost before getting the tls result, as it can mean
  //the TLS handshake failed and the socket was reconnected in normal mode
  if (d->authState != LoginJobPrivate::StartTls) {
    emitResult();
  }

}


#include "loginjob.moc"
