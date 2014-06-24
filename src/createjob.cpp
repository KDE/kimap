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

#include "createjob.h"

#include <KDE/KLocalizedString>

#include "job_p.h"
#include "message_p.h"
#include "rfccodecs.h"
#include "session_p.h"

namespace KIMAP
{
  class CreateJobPrivate : public JobPrivate
  {
    public:
      CreateJobPrivate( Session *session, const QString& name ) : JobPrivate( session, name ) { }
      ~CreateJobPrivate() { }

      QString mailBox;
  };
}

using namespace KIMAP;

CreateJob::CreateJob( Session *session )
  : Job( *new CreateJobPrivate( session, i18n( "Create" ) ) )
{
}

CreateJob::~CreateJob()
{
}

void CreateJob::doStart()
{
  Q_D( CreateJob );
  d->tags << d->sessionInternal()->sendCommand( "CREATE", '\"' + KIMAP::encodeImapFolderName( d->mailBox.toUtf8() ) + '\"' );
}

void CreateJob::setMailBox( const QString &mailBox )
{
  Q_D( CreateJob );
  d->mailBox = mailBox;
}

QString CreateJob::mailBox() const
{
  Q_D( const CreateJob );
  return d->mailBox;
}
