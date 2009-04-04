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

#include "expungejob.h"

#include <KDE/KLocale>
#include <KDE/KDebug>

#include "job_p.h"
#include "message_p.h"
#include "session_p.h"

namespace KIMAP
{
  class ExpungeJobPrivate : public JobPrivate
  {
    public:
      ExpungeJobPrivate( Session *session, const QString& name ) : JobPrivate(session, name) { }
      ~ExpungeJobPrivate() { }

      QList< int > items;
  };
}

using namespace KIMAP;

ExpungeJob::ExpungeJob( Session *session )
  : Job( *new ExpungeJobPrivate(session, i18n("Expunge")) )
{
}

ExpungeJob::~ExpungeJob()
{
}

void ExpungeJob::doStart()
{
  Q_D(ExpungeJob);
  d->tag = d->sessionInternal()->sendCommand( "EXPUNGE" );
}

void ExpungeJob::doHandleResponse( const Message &response )
{
  Q_D(ExpungeJob);

  if (handleErrorReplies(response) == NotHandled) {
    if ( response.content.size() >= 2 ) {
        QByteArray code = response.content[2].toString();
        if  (code == "EXPUNGE") {
          QByteArray s = response.content[1].toString();
          bool ok = true;
          int id = s.toInt(&ok);
          if (ok) {
            d->items.append(id);
          }
          //TODO error handling
          return;
        }
    }
    kDebug() << "Unhandled response: " << response.toString().constData();
  }
}

QList< int > ExpungeJob::deletedItems() const
{
  Q_D(const ExpungeJob);
  return d->items;
}


#include "expungejob.moc"
