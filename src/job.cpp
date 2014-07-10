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
#include "message_p.h"
#include "session_p.h"

#include <KLocalizedString>
#include <QDebug>

using namespace KIMAP;

Job::Job( Session *session )
  : KJob( session ), d_ptr( new JobPrivate( session, i18n( "Job" ) ) )
{
}

Job::Job( JobPrivate &dd )
  : KJob( dd.m_session ), d_ptr( &dd )
{
}

Job::~Job()
{
  delete d_ptr;
}

Session *Job::session() const
{
  Q_D( const Job );
  return d->m_session;
}

void Job::start()
{
  Q_D( Job );
  d->sessionInternal()->addJob( this );
}

void Job::handleResponse(const Message &response)
{
  handleErrorReplies( response );
}

void Job::connectionLost()
{
  setError( KJob::UserDefinedError );
  setErrorText( i18n( "Connection to server lost." ) );
  emitResult();
}

void Job::setSocketError(KTcpSocket::Error error)
{
  Q_D( Job );
  d->m_socketError = error;
}

Job::HandlerResponse Job::handleErrorReplies(const Message &response)
{
  Q_D( Job );
//   qDebug() << response.toString();

  if ( !response.content.isEmpty() &&
       d->tags.contains( response.content.first().toString() ) ) {
    if ( response.content.size() < 2 ) {
      setErrorText( i18n( "%1 failed, malformed reply from the server.", d->m_name ) );
    } else if ( response.content[1].toString() != "OK" ) {
      setError( UserDefinedError );
      setErrorText( i18n( "%1 failed, server replied: %2", d->m_name, QLatin1String(response.toString().constData()) ) );
    }
    d->tags.removeAll( response.content.first().toString() );
    if ( d->tags.isEmpty() ) { // Only emit result when the last command returned
      emitResult();
    }
    return Handled;
  }

  return NotHandled;
}
