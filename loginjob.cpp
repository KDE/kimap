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

namespace KIMAP
{
  class LoginJobPrivate : public JobPrivate
  {
    public:
     enum AuthState {
        StartTls = 0,
        Login
      };

      LoginJobPrivate( Session *session, const QString& name ) : JobPrivate(session, name), encryptionMode(LoginJob::Unencrypted), authState(Login) { }
      ~LoginJobPrivate() { }

      QString userName;
      QString password;

      LoginJob::EncryptionMode encryptionMode;

      AuthState authState;
  };
}

using namespace KIMAP;

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
    d->tag = d->sessionInternal()->sendCommand( "LOGIN", d->userName.toUtf8()+' '+d->password.toUtf8() );
  } else if (d->encryptionMode == TlsV1) {
    d->authState = LoginJobPrivate::StartTls;
    d->tag = d->sessionInternal()->sendCommand( "STARTTLS" );
  }
}

void LoginJob::doHandleResponse( const Message &response )
{
  Q_D(LoginJob);

  QString commandName = i18n("Login");

  if (d->authState == LoginJobPrivate::StartTls) {
    commandName = i18n("StartTls");
  }

  if ( !response.content.isEmpty()
       && response.content.first().toString() == d->tag ) {
    if ( response.content.size() < 2 ) {
      setErrorText( i18n("%1 failed, malformed reply from the server").arg(commandName) );
  } else
    if ( response.content[1].toString() != "OK" ) {
      setError( UserDefinedError );
      setErrorText( i18n("%1 failed, server replied: %2", commandName, response.toString().constData()) );
  } else
    if ( response.content[1].toString() == "OK" && d->authState == LoginJobPrivate::StartTls) {
      d->sessionInternal()->startTls();
      return;
    }
  }
  emitResult();
}

void LoginJob::tlsResponse(bool response)
{
  Q_D(LoginJob);
    
  if (response) {
    d->authState = LoginJobPrivate::Login;
    d->tag = d->sessionInternal()->sendCommand( "LOGIN", d->userName.toUtf8()+' '+d->password.toUtf8() );
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

void LoginJob::connectionLost()
{
  Q_D(LoginJob);

  //don't emit the result if the connection was lost before getting the tls result, as it can mean
  //the TLS handshake failed and the socket was reconnected in normal mode
  if (d->authState == LoginJobPrivate::Login) {
    emitResult();
  }
    
}


#include "loginjob.moc"
