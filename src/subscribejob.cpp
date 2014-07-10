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

#include "subscribejob.h"

#include <KLocalizedString>

#include "job_p.h"
#include "message_p.h"
#include "session_p.h"
#include "rfccodecs.h"

namespace KIMAP
{
  class SubscribeJobPrivate : public JobPrivate
  {
    public:
      SubscribeJobPrivate( Session *session, const QString& name ) : JobPrivate( session, name ) { }
      ~SubscribeJobPrivate() { }

      QString mailBox;
  };
}

using namespace KIMAP;

SubscribeJob::SubscribeJob( Session *session )
  : Job( *new SubscribeJobPrivate( session, i18n( "Subscribe" ) ) )
{
}

SubscribeJob::~SubscribeJob()
{
}

void SubscribeJob::doStart()
{
  Q_D( SubscribeJob );
  d->tags << d->sessionInternal()->sendCommand( "SUBSCRIBE", '\"' + KIMAP::encodeImapFolderName( d->mailBox.toUtf8() ) + '\"' );
}

void SubscribeJob::setMailBox( const QString &mailBox )
{
  Q_D( SubscribeJob );
  d->mailBox = mailBox;
}

QString SubscribeJob::mailBox() const
{
  Q_D( const SubscribeJob );
  return d->mailBox;
}
