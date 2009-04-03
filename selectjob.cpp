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

#include "selectjob.h"

#include <KDE/KLocale>

#include "job_p.h"
#include "message_p.h"
#include "session_p.h"

namespace KIMAP
{
  class SelectJobPrivate : public JobPrivate
  {
    public:
      SelectJobPrivate( Session *session, const QString& name )
        : JobPrivate(session, name), readOnly(false), messageCount(-1), recentCount(-1),
          firstUnseenIndex(-1), uidValidity(-1), nextUid(-1) { }
      ~SelectJobPrivate() { }

      QByteArray mailBox;
      bool readOnly;

      QList<QByteArray> flags;
      QList<QByteArray> permanentFlags;
      int messageCount;
      int recentCount;
      int firstUnseenIndex;
      int uidValidity;
      int nextUid;
  };
}

using namespace KIMAP;

SelectJob::SelectJob( Session *session )
  : Job( *new SelectJobPrivate(session, i18n("Select")) )
{
}

SelectJob::~SelectJob()
{
}

void SelectJob::setMailBox( const QByteArray &mailBox )
{
  Q_D(SelectJob);
  d->mailBox = mailBox;
}

QByteArray SelectJob::mailBox() const
{
  Q_D(const SelectJob);
  return d->mailBox;
}

void SelectJob::setOpenReadOnly( bool readOnly )
{
  Q_D(SelectJob);
  d->readOnly = readOnly;
}

bool SelectJob::isOpenReadOnly() const
{
  Q_D(const SelectJob);
  return d->readOnly;
}

QList<QByteArray> SelectJob::flags() const
{
  Q_D(const SelectJob);
  return d->flags;
}

QList<QByteArray> SelectJob::permanentFlags() const
{
  Q_D(const SelectJob);
  return d->permanentFlags;
}

int SelectJob::messageCount() const
{
  Q_D(const SelectJob);
  return d->messageCount;
}

int SelectJob::recentCount() const
{
  Q_D(const SelectJob);
  return d->recentCount;
}

int SelectJob::firstUnseenIndex() const
{
  Q_D(const SelectJob);
  return d->firstUnseenIndex;
}

int SelectJob::uidValidity() const
{
  Q_D(const SelectJob);
  return d->uidValidity;
}

int SelectJob::nextUid() const
{
  Q_D(const SelectJob);
  return d->nextUid;
}

void SelectJob::doStart()
{
  Q_D(SelectJob);

  QByteArray command = "SELECT";
  if ( d->readOnly ) {
    command = "EXAMINE";
  }

  d->tag = d->sessionInternal()->sendCommand( command, '\"'+d->mailBox+'\"' );
}

void SelectJob::doHandleResponse( const Message &response )
{
  Q_D(SelectJob);

  if ( handleErrorReplies(response) == NotHandled) {
      if ( response.content.size() >= 2 ) {
        QByteArray code = response.content[1].toString();

        if ( code=="OK" ) {
          if ( response.responseCode.size() < 2 ) return;

          code = response.responseCode[0].toString();

          if ( code=="PERMANENTFLAGS" ) {
            d->permanentFlags = response.responseCode[1].toList();
          } else {
            bool isInt;
            int value = response.responseCode[1].toString().toInt(&isInt);
            if ( !isInt ) return;

            if ( code=="UNSEEN" ) {
              d->firstUnseenIndex = value;
            } else if ( code=="UIDVALIDITY" ) {
              d->uidValidity = value;
            } else if ( code=="UIDNEXT" ) {
              d->nextUid = value;
            }
          }
        } else if ( code=="FLAGS" ) {
          d->flags = response.content[2].toList();
        } else {
          bool isInt;
          int value = response.content[1].toString().toInt(&isInt);
          if ( !isInt || response.content.size()<3 ) return;

          code = response.content[2].toString();
          if ( code=="EXISTS" ) {
            d->messageCount = value;
          } else if ( code=="RECENT" ) {
            d->recentCount = value;
          }
        }
      } else {
        qDebug("%s", response.toString().constData());
      }
  }
}

#include "selectjob.moc"
