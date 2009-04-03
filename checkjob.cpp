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

#include "checkjob.h"

#include <KDE/KLocale>

#include "job_p.h"
#include "message_p.h"
#include "session_p.h"

namespace KIMAP
{
  class CheckJobPrivate : public JobPrivate
  {
    public:
      CheckJobPrivate( Session *session, const QString& name ) : JobPrivate(session, name) { }
      ~CheckJobPrivate() { }
  };
}

using namespace KIMAP;

CheckJob::CheckJob( Session *session )
  : Job( *new CheckJobPrivate(session, i18n("Check")) )
{
}

CheckJob::~CheckJob()
{
}

void CheckJob::doStart()
{
  Q_D(CheckJob);
  d->tag = d->sessionInternal()->sendCommand( "CHECK" );
}

#include "checkjob.moc"
