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

#include "job_p.h"
#include "message_p.h"
#include "session_p.h"

namespace KIMAP
{
  class LoginJobPrivate : public JobPrivate
  {
    public:
      LoginJobPrivate( Session *session ) : JobPrivate(session) { }
      ~LoginJobPrivate() { }

      QString userName;
      QString password;
  };
}

using namespace KIMAP;

LoginJob::LoginJob( Session *session )
  : Job( *new LoginJobPrivate(session) )
{

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
  d->sessionInternal()->sendCommand(
    QString( "A001 LOGIN %1 %2\r\n" )
    .arg(d->userName)
    .arg(d->password)
    .toUtf8()
  );
}

void LoginJob::doHandleResponse( const Message &response )
{
  if ( !response.content.isEmpty()
    && response.content.first().toString()=="A001" ) {
    if ( response.content.size() < 2 ) {
      setErrorText( i18n("Login failed, malformed reply from the server") );
    } else if ( response.content[1].toString()!="OK" ) {
      QString status;
      for ( QList<Message::Part>::ConstIterator it = response.content.begin()+1;
            it!=response.content.end(); ++it ) {
        if (it!=response.content.begin()+1) {
          status+=' ';
        }
        status+=(*it).toString();
      }
      setError( UserDefinedError );
      setErrorText( i18n("Login failed, server replied: %1", status) );
    }

    emitResult();
  }
}

#include "loginjob.moc"
