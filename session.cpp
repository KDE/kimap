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

#include "session.h"
#include "session_p.h"
#include "job.h"
#include "message_p.h"
#include "sessionthread_p.h"

#include <QtCore/QDebug>
#include <QtCore/QTimer>

using namespace KIMAP;

Session::Session( const QString &hostName, quint16 port, QObject *parent)
  : QObject(parent), d(new SessionPrivate(this))
{
  d->hostName = hostName;
  d->port = port;
  d->state = Disconnected;
  d->jobRunning = false;

  d->socket = new SessionSocket;
  //TODO: connects to catch errors

  d->thread = new SessionThread( this );
  d->thread->setDevice(d->socket);
  connect( d->thread, SIGNAL(responseReceived(KIMAP::Message)),
           this, SLOT(responseReceived(KIMAP::Message)) );

  d->reconnect();
}

QString Session::hostName() const
{
  return d->hostName;
}

quint16 Session::port() const
{
  return d->port;
}

Session::State Session::state() const
{
  return d->state;
}

SessionPrivate::SessionPrivate( Session *session )
  : q(session)
{
}

void SessionPrivate::reconnect()
{
  if ( socket->state() != SessionSocket::ConnectedState &&
       socket->state() != SessionSocket::ConnectingState ) {
    socket->connectToHost(hostName, port);
  }
}

void SessionPrivate::addJob(Job *job)
{
  queue.append(job);

  QObject::connect( job, SIGNAL(result(KJob*)), q, SLOT(jobDone(KJob*)) );
  QObject::connect( job, SIGNAL(destroyed(QObject*)), q, SLOT(jobDestroyed(QObject*)) );

  startNext();
}

void SessionPrivate::startNext()
{
  QTimer::singleShot( 0, q, SLOT(doStartNext()) );
}

void SessionPrivate::doStartNext()
{
  if ( queue.isEmpty() || jobRunning || state==Session::Disconnected )
    return;

  jobRunning = true;

  currentJob = queue.dequeue();
  currentJob->doStart();
}

void SessionPrivate::jobDone( KJob *job )
{
  Q_ASSERT( job == currentJob );

  jobRunning = false;
  currentJob = 0;
  startNext();
}

void SessionPrivate::jobDestroyed( QObject *job )
{
  queue.removeAll( static_cast<KIMAP::Job*>( job ) );
  if ( currentJob == job )
    currentJob = 0;
}

void SessionPrivate::responseReceived( const Message &response )
{
  QByteArray code;
  QByteArray command;

  if ( response.content.size()>=2 ) {
    code = response.content[1].toString();
  }

  if ( response.content.size()>=3 ) {
    command = response.content[2].toString();
  }

  switch ( state ) {
  case Session::Disconnected:
    if ( code=="OK" ) {
      state = Session::NotAuthenticated;
      startNext();
    } else if ( code=="PREAUTH" ) {
      state = Session::Authenticated;
      startNext();
    } else {
      socket->close();
      QTimer::singleShot( 1000, q, SLOT( reconnect() ) );
    }
    return;
  case Session::NotAuthenticated:
    if ( code=="OK" && ( command=="LOGIN" || command=="AUTHENTICATE" ) ) {
      state = Session::Authenticated;
    }
    break;
  case Session::Authenticated:
    if ( code=="OK" && ( command=="SELECT" || command=="EXAMINE" ) ) {
      state = Session::Selected;
    }
    break;
  case Session::Selected:
    if ( ( code=="OK" && command=="CLOSE" )
      || ( code!="OK" && ( command=="SELECT" || command=="EXAMINE" ) ) ) {
      state = Session::Authenticated;
    }
    break;
  }

  // If a job is running forward it the response
  if ( currentJob!=0 ) {
    currentJob->doHandleResponse( response );
  } else {
    qWarning() << "A message was received from the server with no job to handle it";
  }
}

void SessionPrivate::sendCommand( const QByteArray &command )
{
  socket->write(command);
}

#include "session.moc"
