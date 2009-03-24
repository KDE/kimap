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

#include "logoutjob.h"

#include <KDE/KLocale>

#include "job_p.h"
#include "message_p.h"
#include "session_p.h"

namespace KIMAP
{
  class LogoutJobPrivate : public JobPrivate
  {
    public:
      LogoutJobPrivate( Session *session ) : JobPrivate(session) { }
      ~LogoutJobPrivate() { }

      QByteArray tag;
  };
}

using namespace KIMAP;

LogoutJob::LogoutJob( Session *session )
  : Job( *new LogoutJobPrivate(session) )
{

}

LogoutJob::~LogoutJob()
{
}

void LogoutJob::doStart()
{
  Q_D(LogoutJob);
  d->tag = d->sessionInternal()->sendCommand( "LOGOUT" );
}

void LogoutJob::doHandleResponse( const Message &response )
{
  Q_D(LogoutJob);

  if ( !response.content.isEmpty()
    && response.content.first().toString()==d->tag ) {
    if ( response.content.size() < 2 ) {
      setErrorText( i18n("Logout failed, malformed reply from the server") );
      emitResult();
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
      setErrorText( i18n("Logout failed, server replied: %1", status) );
      emitResult();
    }
  }
}

void LogoutJob::connectionLost()
{
  emitResult();
}

#include "logoutjob.moc"
