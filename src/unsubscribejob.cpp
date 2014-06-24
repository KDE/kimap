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

#include "unsubscribejob.h"

#include <KDE/KLocalizedString>

#include "job_p.h"
#include "message_p.h"
#include "session_p.h"
#include "rfccodecs.h"

namespace KIMAP
{
  class UnsubscribeJobPrivate : public JobPrivate
  {
    public:
      UnsubscribeJobPrivate( Session *session, const QString& name ) : JobPrivate( session, name ) { }
      ~UnsubscribeJobPrivate() { }

      QString mailBox;
  };
}

using namespace KIMAP;

UnsubscribeJob::UnsubscribeJob( Session *session )
  : Job( *new UnsubscribeJobPrivate( session, i18n( "Unsubscribe" ) ) )
{
}

UnsubscribeJob::~UnsubscribeJob()
{
}

void UnsubscribeJob::doStart()
{
  Q_D( UnsubscribeJob );
  d->tags << d->sessionInternal()->sendCommand( "UNSUBSCRIBE", '\"' + KIMAP::encodeImapFolderName( d->mailBox.toUtf8() ) + '\"' );
}

void UnsubscribeJob::setMailBox( const QString &mailBox )
{
  Q_D( UnsubscribeJob );
  d->mailBox = mailBox;
}

QString UnsubscribeJob::mailBox() const
{
  Q_D( const UnsubscribeJob );
  return d->mailBox;
}
