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

#include "statusjob.h"

#include <KDE/KLocale>

#include "job_p.h"
#include "message_p.h"
#include "session_p.h"
#include "rfccodecs.h"

namespace KIMAP
{
  class StatusJobPrivate : public JobPrivate
  {
    public:
      StatusJobPrivate( Session *session, const QString& name )
        : JobPrivate(session, name), messageCount(-1),
          uidValidity(-1), nextUid(-1) { }
      ~StatusJobPrivate() { }

      QString mailBox;

      int messageCount;
      qint64 uidValidity;
      int nextUid;
  };
}

using namespace KIMAP;

StatusJob::StatusJob( Session *session )
  : Job( *new StatusJobPrivate(session, i18n("Status")) )
{
}

StatusJob::~StatusJob()
{
}

void StatusJob::setMailBox( const QString &mailBox )
{
  Q_D(StatusJob);
  d->mailBox = mailBox;
}

QString StatusJob::mailBox() const
{
  Q_D(const StatusJob);
  return d->mailBox;
}

int StatusJob::messageCount() const
{
  Q_D(const StatusJob);
  return d->messageCount;
}

qint64 StatusJob::uidValidity() const
{
  Q_D(const StatusJob);
  return d->uidValidity;
}

int StatusJob::nextUid() const
{
  Q_D(const StatusJob);
  return d->nextUid;
}

void StatusJob::doStart()
{
  Q_D(StatusJob);

  d->tag = d->sessionInternal()->sendCommand( "STATUS", '\"'+KIMAP::encodeImapFolderName( d->mailBox.toUtf8() )+"\" (MESSAGES UIDVALIDITY UIDNEXT)" );
}

void StatusJob::doHandleResponse( const Message &response )
{
  Q_D(StatusJob);

  if ( handleErrorReplies(response) == NotHandled) {
      if ( response.content.size() >= 4 ) {
        QByteArray code = response.content[1].toString();
        QString mailBox = QString::fromUtf8( response.content[2].toString() );

        if ( code=="STATUS" && mailBox==d->mailBox ) {
          QList<QByteArray> data = response.content[3].toList();

          for ( QList<QByteArray>::ConstIterator it = data.constBegin();
                it!=data.constEnd(); ++it ) {
            if ( (*it)=="MESSAGES" ) {
              ++it;
              d->messageCount = it->toInt();
            } else if ( (*it)=="UIDVALIDITY" ) {
              ++it;
              d->uidValidity = it->toLongLong();
            } else if ( (*it)=="UIDNEXT" ) {
              ++it;
              d->nextUid = it->toInt();
            }
          }

          emit status( mailBox, d->messageCount, d->uidValidity, d->nextUid );
        }
      } else {
        qDebug("%s", response.toString().constData());
      }
  }
}

#include "statusjob.moc"
