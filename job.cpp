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

#include "job.h"
#include "job_p.h"
#include "session_p.h"

#include <KDE/KLocale>

using namespace KIMAP;

Job::Job( Session *session )
  : d_ptr(new JobPrivate(session))
{

}

Job::Job( JobPrivate &dd )
  : d_ptr(&dd)
{

}

Job::~Job()
{
  delete d_ptr;
}

void Job::start()
{
  Q_D(Job);
  d->sessionInternal()->addJob(this);
}

void Job::doHandleResponse(const Message &/*response*/)
{
}

void Job::connectionLost()
{
  setError( KJob::UserDefinedError );
  setErrorText( i18n("Connection to server lost") );
  emitResult();
}

#include "job.moc"
