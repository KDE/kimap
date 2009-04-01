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

#include "deletejob.h"

#include <KDE/KLocale>
#include <KDE/KDebug>

#include "job_p.h"
#include "message_p.h"
#include "session_p.h"

namespace KIMAP
{
  class DeleteJobPrivate : public JobPrivate
  {
    public:
      DeleteJobPrivate( Session *session ) : JobPrivate(session) { }
      ~DeleteJobPrivate() { }

      QByteArray tag;
  };
}

using namespace KIMAP;

DeleteJob::DeleteJob( Session *session )
  : Job( *new DeleteJobPrivate(session) )
{

}

DeleteJob::~DeleteJob()
{
}

void DeleteJob::doStart()
{
  Q_D(DeleteJob);
  d->tag = d->sessionInternal()->sendCommand( "DELETE" );
}

void DeleteJob::doHandleResponse( const Message &response )
{
  Q_D(DeleteJob);

  if ( !response.content.isEmpty()
    && response.content.first().toString()==d->tag ) {
    if ( response.content.size() < 2 ) {
      setErrorText( i18n("Delete failed, malformed reply from the server") );
    } else
    if ( response.content[1].toString()=="NO" ) {
      setError( UserDefinedError );
      setErrorText( i18n("Delete failed, can't delete mailbox. Server replied: %1", response.toString().constData()) );
    } else
    if ( response.content[1].toString()!="OK" ) {
      setError( UserDefinedError );
      setErrorText( i18n("Delete failed, server replied: %1", response.toString().constData()) );
    }
    emitResult();
  }
}

void DeleteJob::connectionLost()
{
  emitResult();
}


#include "deletejob.moc"
