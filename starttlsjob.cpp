/*
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

#include "starttlsjob.h"

#include <KDE/KLocale>
#include <KDE/KDebug>

#include "job_p.h"
#include "message_p.h"
#include "session_p.h"

namespace KIMAP
{
  class StartTlsJobPrivate : public JobPrivate
  {
    public:
      StartTlsJobPrivate( Session *session, const QString& name ) : JobPrivate(session, name) { }
      ~StartTlsJobPrivate() { }

      QList< int > items;
  };
}

using namespace KIMAP;

StartTlsJob::StartTlsJob( Session *session )
  : Job( *new StartTlsJobPrivate(session, i18n("StartTLS")) )
{
}

StartTlsJob::~StartTlsJob()
{
}

void StartTlsJob::doStart()
{
  Q_D(StartTlsJob);
  d->tag = d->sessionInternal()->sendCommand( "STARTTLS" );
}

void StartTlsJob::doHandleResponse( const Message &response )
{
  Q_D(StartTlsJob);

  if ( !response.content.isEmpty()
       && response.content.first().toString() == d->tag ) {
    if ( response.content.size() < 2 ) {
      setErrorText( i18n("%1 failed, malformed reply from the server").arg(d->m_name) );
    } else
    if ( response.content[1].toString() != "OK" ) {
      setError( UserDefinedError );
      setErrorText( i18n("%1 failed, server replied: %2", d->m_name, response.toString().constData()) );
    } else
    if ( response.content[1].toString() == "OK" ) {
      //TODO: implement TLS layer commands
    }
    emitResult();
  }
}

QList< int > StartTlsJob::deletedItems() const
{
  Q_D(const StartTlsJob);
  return d->items;
}


#include "starttlsjob.moc"
