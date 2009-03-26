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

#include "listjob.h"

#include <KDE/KLocale>

#include "job_p.h"
#include "message_p.h"
#include "session_p.h"

namespace KIMAP
{
  class ListJobPrivate : public JobPrivate
  {
    public:
      ListJobPrivate( Session *session ) : JobPrivate(session), includeUnsubscribed(false) { }
      ~ListJobPrivate() { }

      bool includeUnsubscribed;
      QByteArray tag;
      QByteArray command;
      QList< QList<QByteArray> > descriptors;
  };
}

using namespace KIMAP;

ListJob::ListJob( Session *session )
  : Job( *new ListJobPrivate(session) )
{

}

ListJob::~ListJob()
{
}

void ListJob::setIncludeUnsubscribed( bool include )
{
  Q_D(ListJob);
  d->includeUnsubscribed = include;
}

bool ListJob::isIncludeUnsubscribed() const
{
  Q_D(const ListJob);
  return d->includeUnsubscribed;
}

QList< QList<QByteArray> > ListJob::mailBoxes() const
{
  Q_D(const ListJob);
  return d->descriptors;
}

void ListJob::doStart()
{
  Q_D(ListJob);

  d->command = "LSUB";
  if (d->includeUnsubscribed) {
    d->command = "LIST";
  }

  d->tag = d->sessionInternal()->sendCommand( d->command, "\"\" *" );
}

void ListJob::doHandleResponse( const Message &response )
{
  Q_D(ListJob);

  if ( !response.content.isEmpty()
    && response.content.first().toString()==d->tag ) {
    if ( response.content.size() < 2 ) {
      setErrorText( i18n("List failed, malformed reply from the server") );
    } else if ( response.content[1].toString()!="OK" ) {
      setError( UserDefinedError );
      setErrorText( i18n("List failed, server replied: %1", response.toString().constData()) );
    }

    emitResult();
  } else if ( response.content.size() == 5
           && response.content[1].toString()==d->command ) {
    QByteArray separator = response.content[3].toString();
    Q_ASSERT(separator.size()==1);
    QByteArray fullName = response.content[4].toString();

    QList<QByteArray> mailBoxDescriptor;
    mailBoxDescriptor << separator;
    mailBoxDescriptor << fullName.split(separator[0]);

    d->descriptors << mailBoxDescriptor;
    emit mailBoxReceived( mailBoxDescriptor );
  }
}

#include "listjob.moc"
