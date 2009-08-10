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

#include "idlejob.h"

#include <QtCore/QTimer>
#include <KDE/KLocale>

#include "job_p.h"
#include "message_p.h"
#include "session_p.h"

namespace KIMAP
{
  class IdleJobPrivate : public JobPrivate
  {
    public:
      IdleJobPrivate( IdleJob *job, Session *session, const QString& name )
        : JobPrivate( session, name ), q(job),
          messageCount( -1 ), recentCount( -1 ),
          lastMessageCount( -1 ), lastRecentCount( -1 ) { }
      ~IdleJobPrivate() { }

      IdleJob * const q;

      int messageCount;
      int recentCount;

      int lastMessageCount;
      int lastRecentCount;
  };
}

using namespace KIMAP;

IdleJob::IdleJob( Session *session )
  : Job( *new IdleJobPrivate(this, session, i18nc("name of the idle job", "Idle")) )
{
}

IdleJob::~IdleJob()
{
}

void KIMAP::IdleJob::stop()
{
  Q_D(IdleJob);
  d->sessionInternal()->sendData( "DONE" );
}

void IdleJob::doStart()
{
  Q_D(IdleJob);
  d->tag = d->sessionInternal()->sendCommand( "IDLE" );
}

void IdleJob::handleResponse( const Message &response )
{
  Q_D(IdleJob);

  if (handleErrorReplies(response) == NotHandled ) {
    if ( response.content.size() > 0 && response.content[0].toString()=="+" ) {
      // Got the continuation all is fine
      return;

    } else if ( response.content[2].toString()=="EXISTS" ) {
      d->messageCount = response.content[1].toString().toInt();
    } else if ( response.content[2].toString()=="RECENT" ) {
      d->recentCount = response.content[1].toString().toInt();
    }

    if ( d->messageCount>=0 && d->recentCount>=0 ) {
      emit mailBoxStats(this, d->sessionInternal()->selectedMailBox(),
                        d->messageCount, d->recentCount);

      d->lastMessageCount = d->messageCount;
      d->lastRecentCount = d->recentCount;

      d->messageCount = -1;
      d->recentCount = -1;
    }
  }
}

QString KIMAP::IdleJob::lastMailBox() const
{
  Q_D(const IdleJob);
  return d->sessionInternal()->selectedMailBox();
}

int KIMAP::IdleJob::lastMessageCount() const
{
  Q_D(const IdleJob);
  return d->lastMessageCount;
}

int KIMAP::IdleJob::lastRecentCount() const
{
  Q_D(const IdleJob);
  return d->lastRecentCount;
}

#include "idlejob.moc"
