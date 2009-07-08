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

#include "storejob.h"

#include <KDE/KDebug>
#include <KDE/KLocale>

#include "job_p.h"
#include "message_p.h"
#include "session_p.h"

namespace KIMAP
{
  class StoreJobPrivate : public JobPrivate
  {
    public:
      StoreJobPrivate( Session *session, const QString& name ) : JobPrivate( session, name ) { }
      ~StoreJobPrivate() { }

      ImapSet set;
      bool uidBased;
      StoreJob::StoreMode mode;
      MessageFlags flags;

      QMap<int, MessageFlags> resultingFlags;
  };
}

using namespace KIMAP;

StoreJob::StoreJob( Session *session )
  : Job( *new StoreJobPrivate(session, i18n("Store")) )
{
  Q_D(StoreJob);
  d->mode = SetFlags;
}

StoreJob::~StoreJob()
{
}

void StoreJob::setSequenceSet( const ImapSet &set )
{
  Q_D(StoreJob);
  d->set = set;
}

ImapSet StoreJob::sequenceSet() const
{
  Q_D(const StoreJob);
  return d->set;
}

void StoreJob::setUidBased(bool uidBased)
{
  Q_D(StoreJob);
  d->uidBased = uidBased;
}

bool StoreJob::isUidBased() const
{
  Q_D(const StoreJob);
  return d->uidBased;
}

void StoreJob::setFlags( const MessageFlags &flags )
{
  Q_D(StoreJob);
  d->flags = flags;
}

MessageFlags StoreJob::flags() const
{
  Q_D(const StoreJob);
  return d->flags;
}

void StoreJob::setMode( StoreMode mode )
{
  Q_D(StoreJob);
  d->mode = mode;
}

StoreJob::StoreMode StoreJob::mode() const
{
  Q_D(const StoreJob);
  return d->mode;
}

QMap<int, MessageFlags> StoreJob::resultingFlags() const
{
  Q_D(const StoreJob);
  return d->resultingFlags;
}

void StoreJob::doStart()
{
  Q_D(StoreJob);

  QByteArray parameters = d->set.toImapSequenceSet()+' ';

  switch ( d->mode ) {
  case SetFlags:
    parameters+= "FLAGS";
    break;
  case AppendFlags:
    parameters+= "+FLAGS";
    break;
  case RemoveFlags:
    parameters+= "-FLAGS";
    break;
  }

  parameters+=" (";
  foreach ( const QByteArray &flag, d->flags ) {
    parameters+=flag+' ';
  }
  if (!d->flags.isEmpty()) parameters.chop(1);
  parameters+=')';

  qDebug("%s", parameters.constData());

  QByteArray command = "STORE";
  if ( d->uidBased ) {
    command = "UID "+command;
  }

  d->tag = d->sessionInternal()->sendCommand( command, parameters );
}

void StoreJob::handleResponse( const Message &response )
{
  Q_D(StoreJob);

  if (handleErrorReplies(response) == NotHandled ) {
    if ( response.content.size() == 4
      && response.content[2].toString()=="FETCH"
      && response.content[3].type()==Message::Part::List ) {

      int id = response.content[1].toString().toInt();
      qint64 uid = 0;
      bool uidFound = false;
      QList<QByteArray> resultingFlags;

      QList<QByteArray> content = response.content[3].toList();

      for ( QList<QByteArray>::ConstIterator it = content.constBegin();
            it!=content.constEnd(); ++it ) {
        QByteArray str = *it;
        ++it;

        if ( str=="FLAGS" ) {
          if ( (*it).startsWith('(') && (*it).endsWith(')') ) {
            QByteArray str = *it;
            str.chop(1);
            str.remove(0, 1);
            resultingFlags = str.split(' ');
          } else {
            resultingFlags << *it;
          }
        } else if ( str=="UID" ) {
          uid = it->toLongLong(&uidFound);
        }
      }

      if ( !d->uidBased ) {
        d->resultingFlags[id] = resultingFlags;
      } else if ( uidFound ) {
        d->resultingFlags[uid] = resultingFlags;
      } else {
        kWarning() << "We asked for UID but the server didn't give it back, resultingFlags not stored.";
      }
    }
  }
}

#include "storejob.moc"
