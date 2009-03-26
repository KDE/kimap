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

#include "capabilitiesjob.h"

#include <KDE/KLocale>

#include "job_p.h"
#include "message_p.h"
#include "session_p.h"

namespace KIMAP
{
  class CapabilitiesJobPrivate : public JobPrivate
  {
    public:
      CapabilitiesJobPrivate( Session *session ) : JobPrivate(session) { }
      ~CapabilitiesJobPrivate() { }

      QStringList capabilities;
      QByteArray tag;
  };
}

using namespace KIMAP;

CapabilitiesJob::CapabilitiesJob( Session *session )
  : Job( *new CapabilitiesJobPrivate(session) )
{

}

CapabilitiesJob::~CapabilitiesJob()
{
}

QStringList CapabilitiesJob::capabilities() const
{
  Q_D(const CapabilitiesJob);
  return d->capabilities;
}

void CapabilitiesJob::doStart()
{
  Q_D(CapabilitiesJob);
  d->tag = d->sessionInternal()->sendCommand( "CAPABILITY" );
}

void CapabilitiesJob::doHandleResponse( const Message &response )
{
  Q_D(CapabilitiesJob);

  if ( !response.content.isEmpty()
    && response.content.first().toString()==d->tag ) {
    if ( response.content.size() < 2 ) {
      setErrorText( i18n("Capabilities query failed, malformed reply from the server") );
    } else if ( response.content[1].toString()!="OK" ) {
      setError( UserDefinedError );
      setErrorText( i18n("Capabilities query failed, server replied: %1", response.toString().constData()) );
    }

    emitResult();
  } else if ( response.content.size() >= 2
           && response.content[1].toString()=="CAPABILITY" ) {
    for (int i=2; i<response.content.size(); ++i) {
      d->capabilities << response.content[i].toString();
    }
    emit capabilitiesReceived(d->capabilities);
  }
}

#include "capabilitiesjob.moc"
