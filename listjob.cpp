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
#include "rfccodecs.h"
#include "session_p.h"

uint qHash( const QStringList &descriptor )
{
  QString toHash;

  foreach (const QString &part, descriptor ) {
    toHash+= part;
  }

  return qHash(toHash);
}

namespace KIMAP
{
  class ListJobPrivate : public JobPrivate
  {
    public:
      ListJobPrivate( Session *session, const QString& name ) : JobPrivate(session, name), includeUnsubscribed(false) { }
      ~ListJobPrivate() { }

      bool includeUnsubscribed;
      QByteArray command;
      QList<QStringList> descriptors;
      QHash< QStringList, QList<QByteArray> > flags;
  };
}

using namespace KIMAP;

ListJob::ListJob( Session *session )
  : Job( *new ListJobPrivate(session, i18n("List")) )
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

QList<QStringList> ListJob::mailBoxes() const
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

  if (handleErrorReplies(response) == NotHandled) {
    if ( response.content.size() >= 5
           && response.content[1].toString()==d->command ) {
      QList<QByteArray> flags = response.content[2].toList();
      QByteArray separator = response.content[3].toString();
      Q_ASSERT(separator.size()==1);
      QByteArray fullName;
      for ( int i=4; i<response.content.size(); i++ ) {
        fullName+= response.content[i].toString()+' ';
      }
      fullName.chop( 1 );

      fullName = decodeImapFolderName( fullName );

      QStringList mailBoxDescriptor;
      mailBoxDescriptor << QString::fromUtf8( separator );
      foreach ( const QByteArray &part, fullName.split( separator[0] ) ) {
        mailBoxDescriptor << QString::fromUtf8( part );
      }

      d->descriptors << mailBoxDescriptor;
      d->flags[mailBoxDescriptor] = flags;
      emit mailBoxReceived( mailBoxDescriptor, flags );
    }
  }
}

#include "listjob.moc"
