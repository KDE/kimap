/*
    Copyright (c) 2009 Kevin Ottens <ervin@kde.org>

    Copyright (c) 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
    Author: Kevin Ottens <kevin@kdab.com>

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

#include <KDebug>
#include <KDE/KLocale>

#include "job.h"
#include "loginjob.h"
#include "message_p.h"
#include "sessionlogger_p.h"
#include "sessionthread_p.h"
#include "rfccodecs.h"

Q_DECLARE_METATYPE( KTcpSocket::SslVersion )
Q_DECLARE_METATYPE( QSslSocket::SslMode )
static const int _kimap_sslVersionId = qRegisterMetaType<KTcpSocket::SslVersion>();

using namespace KIMAP;

Session::Session( const QString &hostName, quint16 port, QObject *parent)
  : QObject( parent ), d( new SessionPrivate( this ) )
{
  if ( !qgetenv( "KIMAP_LOGFILE" ).isEmpty() ) {
    d->logger = new SessionLogger;
  }

  d->isSocketConnected = false;
  d->state = Disconnected;
  d->jobRunning = false;

  d->thread = new SessionThread( hostName, port, this );
  connect( d->thread, SIGNAL(encryptionNegotiationResult(bool,KTcpSocket::SslVersion)),
           d, SLOT(onEncryptionNegotiationResult(bool,KTcpSocket::SslVersion)) );
  connect( d->thread, SIGNAL(sslError(KSslErrorUiData)), this, SLOT(handleSslError(KSslErrorUiData)) );

  d->startSocketTimer();
  d->thread->start();
}

Session::~Session()
{
  delete d->thread;
}

void Session::setUiProxy(SessionUiProxy::Ptr proxy)
{
  d->uiProxy = proxy;
}

void Session::setUiProxy(SessionUiProxy *proxy)
{
  setUiProxy( SessionUiProxy::Ptr( proxy ) );
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

QString Session::userName() const
{
    return d->userName;
}

QByteArray Session::serverGreeting() const
{
  return d->greeting;
}

int Session::jobQueueSize() const
{
  return d->queue.size() + ( d->jobRunning ? 1 : 0 );
}

void KIMAP::Session::close()
{
  d->thread->closeSocket();
}

void SessionPrivate::handleSslError(const KSslErrorUiData& errorData)
{
  if ( uiProxy && uiProxy->ignoreSslError( errorData ) ) {
    QMetaObject::invokeMethod( thread, "sslErrorHandlerResponse", Q_ARG( bool, true ) );
  } else {
    QMetaObject::invokeMethod( thread, "sslErrorHandlerResponse", Q_ARG( bool, false ) );
  }
}

SessionPrivate::SessionPrivate( Session *session )
  : QObject( session ),
    q( session ),
    state( Session::Disconnected ),
    logger( 0 ),
    currentJob( 0 ),
    tagCount( 0 ),
    sslVersion( KTcpSocket::UnknownSslVersion ),
    socketTimerInterval( 30000 ) // By default timeouts on 30s
{
}

SessionPrivate::~SessionPrivate()
{
  delete logger;
}

void SessionPrivate::addJob(Job *job)
{
  queue.append( job );
  emit q->jobQueueSizeChanged( q->jobQueueSize() );

  QObject::connect( job, SIGNAL(result(KJob*)), q, SLOT(jobDone(KJob*)) );
  QObject::connect( job, SIGNAL(destroyed(QObject*)), q, SLOT(jobDestroyed(QObject*)) );

  if ( state!=Session::Disconnected ) {
    startNext();
  }
}

void SessionPrivate::startNext()
{
  QTimer::singleShot( 0, q, SLOT(doStartNext()) );
}

void SessionPrivate::doStartNext()
{
  if ( queue.isEmpty() || jobRunning || !isSocketConnected ) {
    return;
  }

  startSocketTimer();
  jobRunning = true;

  currentJob = queue.dequeue();
  currentJob->doStart();
}

void SessionPrivate::jobDone( KJob *job )
{
  Q_UNUSED( job );
  Q_ASSERT( job == currentJob );

  // If we're in disconnected state it's because we ended up
  // here because the inactivity timer triggered, so no need to
  // stop it (it is single shot)
  if ( state!=Session::Disconnected ) {
    stopSocketTimer();
  }

  jobRunning = false;
  currentJob = 0;
  emit q->jobQueueSizeChanged( q->jobQueueSize() );
  startNext();
}

void SessionPrivate::jobDestroyed( QObject *job )
{
  queue.removeAll( static_cast<KIMAP::Job*>( job ) );
  if ( currentJob == job ) {
    currentJob = 0;
  }
}

void SessionPrivate::responseReceived( const Message &response )
{
  if ( logger && ( state == Session::Authenticated || state == Session::Selected ) ) {
    logger->dataReceived( response.toString() );
  }

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
    if ( socketTimer.isActive() ) {
      stopSocketTimer();
    }
    if ( code == "OK" ) {
      setState( Session::NotAuthenticated );

      Message simplified = response;
      simplified.content.removeFirst(); // Strip the tag
      simplified.content.removeFirst(); // Strip the code
      greeting = simplified.toString().trimmed(); // Save the server greeting

      startNext();
    } else if ( code == "PREAUTH" ) {
      setState( Session::Authenticated );

      Message simplified = response;
      simplified.content.removeFirst(); // Strip the tag
      simplified.content.removeFirst(); // Strip the code
      greeting = simplified.toString().trimmed(); // Save the server greeting

      startNext();
    } else {
      thread->closeSocket();
    }
    return;
  case Session::NotAuthenticated:
    if ( code == "OK" && tag == authTag ) {
      setState( Session::Authenticated );
    }
    break;
  case Session::Authenticated:
    if ( code == "OK" && tag == selectTag ) {
      setState( Session::Selected );
      currentMailBox = upcomingMailBox;
    }
    break;
  case Session::Selected:
    if ( ( code == "OK" && tag == closeTag ) ||
         ( code != "OK" && tag == selectTag ) ) {
      setState( Session::Authenticated );
      currentMailBox = QByteArray();
    } else if ( code == "OK" && tag == selectTag ) {
      currentMailBox = upcomingMailBox;
    }
    break;
  }

  if ( tag == authTag ) {
    authTag.clear();
  }
  if ( tag == selectTag ) {
    selectTag.clear();
  }
  if ( tag == closeTag ) {
    closeTag.clear();
  }

  // If a job is running forward it the response
  if ( currentJob != 0 ) {
    restartSocketTimer();
    currentJob->handleResponse( response );
  } else {
    qWarning() << "A message was received from the server with no job to handle it:"
               << response.toString()
               << '(' + response.toString().toHex() + ')';
  }
}

void SessionPrivate::setState(Session::State s)
{
  if ( s != state ) {
    Session::State oldState = state;
    state = s;
    emit q->stateChanged( state, oldState );
  }
}

QByteArray SessionPrivate::sendCommand( const QByteArray &command, const QByteArray &args )
{
  QByteArray tag = 'A' + QByteArray::number( ++tagCount ).rightJustified( 6, '0' );

  QByteArray payload = tag + ' ' + command;
  if ( !args.isEmpty() ) {
    payload += ' ' + args;
  }

  sendData( payload );

  if ( command == "LOGIN" || command == "AUTHENTICATE" ) {
    authTag = tag;
  } else if ( command == "SELECT" || command == "EXAMINE" ) {
    selectTag = tag;
    upcomingMailBox = args;
    upcomingMailBox.remove( 0, 1 );
    upcomingMailBox.chop( 1 );
    upcomingMailBox = KIMAP::decodeImapFolderName( upcomingMailBox );
  } else if ( command == "CLOSE" ) {
    closeTag = tag;
  }
  return tag;
}

void SessionPrivate::sendData( const QByteArray &data )
{
  restartSocketTimer();

  if ( logger && ( state == Session::Authenticated || state == Session::Selected ) ) {
    logger->dataSent( data );
  }

  thread->sendData( data + "\r\n" );
}

void SessionPrivate::socketConnected()
{
  stopSocketTimer();
  isSocketConnected = true;

  bool willUseSsl = false;
  if ( !queue.isEmpty() ) {
    KIMAP::LoginJob *login = qobject_cast<KIMAP::LoginJob*>( queue.first() );
    if ( login ) {
      willUseSsl = ( login->encryptionMode() == KIMAP::LoginJob::SslV2 ) ||
                   ( login->encryptionMode() == KIMAP::LoginJob::SslV3 ) ||
                   ( login->encryptionMode() == KIMAP::LoginJob::SslV3_1 ) ||
                   ( login->encryptionMode() == KIMAP::LoginJob::AnySslVersion );

      userName = login->userName();
    }
  }

  if ( state == Session::Disconnected && willUseSsl ) {
    startNext();
  } else {
    startSocketTimer();
  }
}

void SessionPrivate::socketDisconnected()
{
  if ( socketTimer.isActive() ) {
    stopSocketTimer();
  }

  if ( logger && ( state == Session::Authenticated || state == Session::Selected ) ) {
    logger->disconnectionOccured();
  }

  if ( state != Session::Disconnected ) {
    setState( Session::Disconnected );
    emit q->connectionLost();
  } else {
    emit q->connectionFailed();
  }

  isSocketConnected = false;

  clearJobQueue();
}

void SessionPrivate::socketActivity()
{
  restartSocketTimer();
}

void SessionPrivate::socketError()
{
  if ( socketTimer.isActive() ) {
    stopSocketTimer();
  }

  if ( isSocketConnected ) {
    thread->closeSocket();
  } else {
    emit q->connectionFailed();
    emit q->connectionLost();    // KDE5: Remove this. We shouldn't emit connectionLost() if we weren't connected in the first place
    clearJobQueue();
  }
}

void SessionPrivate::clearJobQueue()
{
  if ( currentJob ) {
    currentJob->connectionLost();
  } else if ( !queue.isEmpty() ) {
    currentJob = queue.takeFirst();
    currentJob->connectionLost();
  }

  QQueue<Job*> queueCopy = queue; // copy because jobDestroyed calls removeAll
  qDeleteAll(queueCopy);
  queue.clear();
  emit q->jobQueueSizeChanged( 0 );
}

void SessionPrivate::startSsl(const KTcpSocket::SslVersion &version)
{
  QMetaObject::invokeMethod( thread, "startSsl", Qt::QueuedConnection, Q_ARG( KTcpSocket::SslVersion, version ) );
}

QString Session::selectedMailBox() const
{
  return QString::fromUtf8( d->currentMailBox );
}

void SessionPrivate::onEncryptionNegotiationResult(bool isEncrypted, KTcpSocket::SslVersion version)
{
  if ( isEncrypted ) {
    sslVersion = version;
  } else {
    sslVersion = KTcpSocket::UnknownSslVersion;
  }
  emit encryptionNegotiationResult( isEncrypted );
}

KTcpSocket::SslVersion SessionPrivate::negotiatedEncryption() const
{
  return sslVersion;
}

void SessionPrivate::setSocketTimeout( int ms )
{
  bool timerActive = socketTimer.isActive();

  if ( timerActive ) {
    stopSocketTimer();
  }

  socketTimerInterval = ms;

  if ( timerActive ) {
    startSocketTimer();
  }
}

int SessionPrivate::socketTimeout() const
{
  return socketTimerInterval;
}

void SessionPrivate::startSocketTimer()
{
  if ( socketTimerInterval < 0 ) {
    return;
  }
  Q_ASSERT( !socketTimer.isActive() );

  connect( &socketTimer, SIGNAL(timeout()),
           this, SLOT(onSocketTimeout()) );

  socketTimer.setSingleShot( true );
  socketTimer.start( socketTimerInterval );
}

void SessionPrivate::stopSocketTimer()
{
  if ( socketTimerInterval < 0 ) {
    return;
  }

  socketTimer.stop();

  disconnect( &socketTimer, SIGNAL(timeout()),
              this, SLOT(onSocketTimeout()) );
}

void SessionPrivate::restartSocketTimer()
{
  if ( socketTimer.isActive() ) {
    stopSocketTimer();
  }
  startSocketTimer();
}

void SessionPrivate::onSocketTimeout()
{
  kDebug() << "Socket timeout!";
  thread->closeSocket();
}

void Session::setTimeout( int timeout )
{
  d->setSocketTimeout( timeout * 1000 );
}

#include "moc_session.cpp"
#include "moc_session_p.cpp"
