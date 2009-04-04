/*
    Copyright (c) 2009 Kevin Ottens <ervin@kde.org>

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

#include "job_p.h"
#include "message_p.h"
#include "session_p.h"

#ifndef HAVE_LIBSASL2
#define HAVE_LIBSASL2
#endif
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

      LoginJobPrivate( Session *session, const QString& name ) : JobPrivate(session, name), encryptionMode(LoginJob::Unencrypted),  authState(Login), plainLoginDisabled(false) {
        conn = 0;
        client_interact = 0;
      }
      ~LoginJobPrivate() { }
      bool sasl_interact();

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
  : Job( *new LoginJobPrivate(session, i18n("Login")) )
{  
  connect(session, SIGNAL(tlsNegotiationResult(bool)), this, SLOT(tlsResponse(bool)));
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
  if (d->encryptionMode == Unencrypted ) {
    if (d->authMode.isEmpty()) {
      d->tag = d->sessionInternal()->sendCommand( "LOGIN", d->userName.toUtf8()+' '+d->password.toUtf8() );
    } else {
      if (!startAuthentication()) {
        emitResult();
      }
    }
  } else if (d->encryptionMode == TlsV1) {
    d->authState = LoginJobPrivate::StartTls;
    d->tag = d->sessionInternal()->sendCommand( "STARTTLS" );
  }
}

void LoginJob::doHandleResponse( const Message &response )
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
      setErrorText( i18n("%1 failed, malformed reply from the server").arg(commandName) );
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
            d->tag = d->sessionInternal()->sendCommand( "LOGIN", d->userName.toUtf8()+' '+d->password.toUtf8() );
          }
        }

        //find the selected SASL authentication method
        Q_FOREACH(QString capability, d->capabilities) {
          if (capability.startsWith("AUTH=")) {
            QString authType = capability.mid(5);
            if (authType == d->authMode) {
                if (!startAuthentication()) {
                  emitResult(); //problem, we're done
                }
            }
          }
        }
      } else if (d->authState == LoginJobPrivate::StartTls) {
        d->sessionInternal()->startTls();
      } else {
        emitResult(); //got an OK, command done
      }
    }
  } else if ( response.content.size() >= 2 ) {
    if ( d->authState == LoginJobPrivate::Authenticate ) {
      if (!answerChallenge(response.content[1].toString())) {
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
        setErrorText( i18n("Login failed, authentication mode %1 is not supported by the server.").arg(d->authMode) );
        d->authState = LoginJobPrivate::Login; //just to treat the upcoming OK correctly
      }
    }
  }
}

bool LoginJob::startAuthentication()
{
  Q_D(LoginJob);

  //SASL authentication
  if (!initSASL()) {
      setError( UserDefinedError );
      setErrorText( i18n("Login failed, client cannot initalize the SASL library.") );
      return false;
  }

  d->authState = LoginJobPrivate::Authenticate;
  const char *out = 0;
  uint outlen = 0;
  const char *mechusing = 0;
  
  int result = sasl_client_new( "imap", d->m_session->hostName().toLatin1(), 0, 0, callbacks, 0, &d->conn );
  if ( result != SASL_OK ) {
    kDebug() <<"sasl_client_new failed with:" << result;
    setError( UserDefinedError );
    setErrorText( QString::fromUtf8( sasl_errdetail( d->conn ) ) );
    return false;
  }
  
  do {
    result = sasl_client_start(d->conn, d->authMode.toLatin1(), &d->client_interact,d->capabilities.contains("SASL-IR") ? &out : 0, &outlen, &mechusing);

    if ( result == SASL_INTERACT ) {
      if ( !d->sasl_interact() ) {
        sasl_dispose( &d->conn );
        setError( UserDefinedError ); //TODO: check up the actual error
        return false;
      }
    }
  } while ( result == SASL_INTERACT );

  if ( result != SASL_CONTINUE && result != SASL_OK ) {
    kDebug() <<"sasl_client_start failed with:" << result;
    setError( UserDefinedError );
    setErrorText( QString::fromUtf8( sasl_errdetail( d->conn ) ) );
    sasl_dispose( &d->conn );
    return false;
  }

  QByteArray tmp = QByteArray::fromRawData( out, outlen );
  QByteArray challenge = tmp.toBase64();

  d->tag = d->sessionInternal()->sendCommand( "AUTHENTICATE", d->authMode.toLatin1() + ' ' + challenge );

  return true;
}

bool LoginJob::answerChallenge(const QByteArray &data)
{
  Q_D(LoginJob);
  
  QByteArray challenge = data;
  int result = -1;
  const char *out = 0;
  uint outlen = 0;
  do {
    result = sasl_client_step(d->conn, challenge.isEmpty() ? 0 : challenge.data(),
                              challenge.size(),
                              &d->client_interact,
                              &out, &outlen);

    if (result == SASL_INTERACT) {
      if ( !d->sasl_interact() ) {
        setError( UserDefinedError ); //TODO: check up the actual error
        sasl_dispose( &d->conn );
        return false;
      }
    }
  } while ( result == SASL_INTERACT );

  if ( result != SASL_CONTINUE && result != SASL_OK ) {
    kDebug() <<"sasl_client_step failed with:" << result;
    setError( UserDefinedError ); //TODO: check up the actual error
    setErrorText( QString::fromUtf8( sasl_errdetail( d->conn ) ) );
    sasl_dispose( &d->conn );
    return false;
  }

  QByteArray tmp = QByteArray::fromRawData( out, outlen );
  challenge = tmp.toBase64();

  d->sessionInternal()->sendData( challenge );

  return true;
}

void LoginJob::tlsResponse(bool response)
{
  Q_D(LoginJob);
    
  if (response) {
    d->authState = LoginJobPrivate::Capability;
    d->tag = d->sessionInternal()->sendCommand( "CAPABILITY" );
  } else {
    setError( UserDefinedError );
    setErrorText( i18n("Login failed, TLS negotiation failed." ));
    d->encryptionMode = Unencrypted;
    emitResult();
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
