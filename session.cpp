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
#include "sessionuiproxy.h"

#include <QtCore/QDebug>
#include <QtCore/QTimer>

#include <KDE/KMessageBox>
#include <KDE/KLocale>

#include "job.h"
#include "message_p.h"
#include "sessionthread_p.h"

Q_DECLARE_METATYPE(KTcpSocket::SslVersion)
Q_DECLARE_METATYPE(QSslSocket::SslMode)
static const int _kimap_sslVersionId = qRegisterMetaType<KTcpSocket::SslVersion>();

using namespace KIMAP;

Session::Session( const QString &hostName, quint16 port, QObject *parent)
  : QObject(parent), d(new SessionPrivate(this))
{
  d->state = Disconnected;
  d->jobRunning = false;

  d->thread = new SessionThread(hostName, port, this);
  connect(d->thread, SIGNAL(encryptionNegotiationResult(bool)), this, SIGNAL(encryptionNegotiationResult(bool)));
  connect(d->thread, SIGNAL(sslError(const KSslErrorUiData&)), this, SLOT(handleSslError(const KSslErrorUiData&)));
  connect(this, SIGNAL(sslErrorHandlerResponse(bool)), d->thread, SLOT(sslErrorHandlerResponse(bool)));

  d->thread->start();
}

Session::~Session()
{
  delete d->thread;
}

void Session::setUiProxy(SessionUiProxy *proxy)
{
  d->uiProxy = proxy;
}

QString Session::hostName() const
{
  return d->thread->hostName();
}

quint16 Session::port() const
{
  return d->thread->port();
}

Session::State Session::state() const
{
  return d->state;
}

void SessionPrivate::handleSslError(const KSslErrorUiData& errorData) {
  if (uiProxy && uiProxy->ignoreSslError(errorData)) {
     emit q->sslErrorHandlerResponse(true);
   } else {
     emit q->sslErrorHandlerResponse(false);
   }
}

SessionPrivate::SessionPrivate( Session *session )
  : q(session),
    uiProxy(0),
    currentJob(0),
    tagCount(0)
{
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
  if ( queue.isEmpty() || jobRunning || state==Session::Disconnected ) {
    return;
  }

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
  QByteArray tag;
  QByteArray code;

  if ( response.content.size()>=1 ) {
    tag = response.content[0].toString();
  }

  if ( response.content.size()>=2 ) {
    code = response.content[1].toString();
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
      thread->closeSocket();
      QTimer::singleShot( 1000, thread, SLOT( reconnect() ) );
    }
    return;
  case Session::NotAuthenticated:
    if ( code=="OK" && tag==authTag ) {
      state = Session::Authenticated;
    }
    break;
  case Session::Authenticated:
    if ( code=="OK" && tag==selectTag ) {
      state = Session::Selected;
      currentMailBox = upcomingMailBox;
    }
    break;
  case Session::Selected:
    if ( ( code=="OK" && tag==closeTag )
      || ( code!="OK" && tag==selectTag) ) {
      state = Session::Authenticated;
      currentMailBox = QByteArray();
    } else if ( code=="OK" && tag==selectTag ) {
      currentMailBox = upcomingMailBox;
    }
    break;
  }

  if (tag==authTag) authTag.clear();
  if (tag==selectTag) selectTag.clear();
  if (tag==closeTag) closeTag.clear();

  // If a job is running forward it the response
  if ( currentJob!=0 ) {
    currentJob->doHandleResponse( response );
  } else {
    qWarning() << "A message was received from the server with no job to handle it";
  }
}

QByteArray SessionPrivate::sendCommand( const QByteArray &command, const QByteArray &args )
{
  QByteArray tag = "A" + QByteArray::number(++tagCount).rightJustified(6, '0');

  QByteArray payload = tag+' '+command;
  if ( !args.isEmpty() ) {
    payload+= ' '+args;
  }
  payload+="\r\n";

  thread->sendData(payload);

  if ( command=="LOGIN" || command=="AUTHENTICATE" ) {
    authTag = tag;
  } else if ( command=="SELECT" || command=="EXAMINE" ) {
    selectTag = tag;
    upcomingMailBox = args;
    upcomingMailBox.remove( 0, 1 );
    upcomingMailBox.chop( 1 );
  } else if ( command=="CLOSE" ) {
    closeTag = tag;
  }

  return tag;
}

void SessionPrivate::sendData( const QByteArray &data )
{
  thread->sendData(data+"\r\n");
}

void SessionPrivate::socketConnected()
{
  state = Session::NotAuthenticated;
  startNext();
}

void SessionPrivate::socketDisconnected()
{
  state = Session::Disconnected;
  thread->closeSocket();

  if ( currentJob ) {
    currentJob->connectionLost();
  }
}

void SessionPrivate::socketError()
{
  //qWarning() << "Socket error occurred:" << socket->errorString();
  socketDisconnected();
}

void SessionPrivate::startSsl(const KTcpSocket::SslVersion &version)
{
  QMetaObject::invokeMethod( thread, "startSsl", Qt::QueuedConnection, Q_ARG(KTcpSocket::SslVersion, version) );
}

QByteArray SessionPrivate::selectedMailBox() const
{
  return currentMailBox;
}

#include "session.moc"
