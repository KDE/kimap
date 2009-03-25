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

#include "sessionthread_p.h"

#include <QtCore/QDebug>
#include <QtCore/QTimer>

#include "imapstreamparser.h"
#include "message_p.h"
#include "session.h"

using namespace KIMAP;

Q_DECLARE_METATYPE(QAbstractSocket::SocketError)
static const int _kimap_socketErrorTypeId = qRegisterMetaType<QAbstractSocket::SocketError>();

SessionThread::SessionThread( const QString &hostName, quint16 port, Session *parent )
  : QThread(), m_hostName(hostName), m_port(port),
    m_session(parent), m_socket(0), m_stream(0)
{
  // Yeah, sounds weird, but QThread object is linked to the parent
  // thread not to itself, and I'm too lazy to introduce yet another
  // internal QObject
  moveToThread(this);
}

SessionThread::~SessionThread()
{
  quit();
  wait();
}

void SessionThread::sendData( const QByteArray &payload )
{
  QMutexLocker locker(&m_mutex);

  m_dataQueue.enqueue( payload );
  QTimer::singleShot( 0, this, SLOT( writeDataQueue() ) );
}

void SessionThread::writeDataQueue()
{
  QMutexLocker locker(&m_mutex);

  while ( !m_dataQueue.isEmpty() ) {
    m_socket->write( m_dataQueue.dequeue() );
  }
}

void SessionThread::readMessage()
{
  QMutexLocker locker(&m_mutex);

  if ( m_stream->availableDataSize()==0 ) {
    return;
  }

  Message message;
  QList<Message::Part> *payload = &message.content;

  try {
    while ( !m_stream->atCommandEnd() ) {
      if ( m_stream->hasString() ) {
        *payload << Message::Part(m_stream->readString());
      } else if ( m_stream->hasList() ) {
        *payload << Message::Part(m_stream->readParenthesizedList());
      } else if ( m_stream->hasResponseCode() ) {
        payload = &message.responseCode;
      } else if ( m_stream->atResponseCodeEnd() ) {
        payload = &message.content;
      } else if ( m_stream->hasLiteral() ) {
        QByteArray literal;
        while ( !m_stream->atLiteralEnd() ) {
          literal+= m_stream->readLiteralPart();
        }
        *payload << Message::Part(literal);
      }
    }

    emit responseReceived(message);

  } catch (KIMAP::ImapParserException e) {
    qWarning() << "The stream parser raised an exception:" << e.what();
  }

  if ( m_stream->availableDataSize()>1 ) {
    QTimer::singleShot( 0, this, SLOT( readMessage() ) );
  }

}

void SessionThread::closeSocket()
{
  QMutexLocker locker(&m_mutex);

  QMetaObject::invokeMethod( m_socket, "close" );
}

void SessionThread::reconnect()
{
  QMutexLocker locker(&m_mutex);

  if ( m_socket->state() != SessionSocket::ConnectedState &&
       m_socket->state() != SessionSocket::ConnectingState ) {
    m_socket->connectToHost(m_hostName, m_port);
  }
}

void SessionThread::run()
{
  m_socket = new SessionSocket;
  m_stream = new ImapStreamParser( m_socket );
  connect( m_socket, SIGNAL(readyRead()),
           this, SLOT(readMessage()), Qt::QueuedConnection );

  connect( m_socket, SIGNAL(disconnected()),
           m_session, SLOT(socketDisconnected()) );
  connect( m_socket, SIGNAL(error(QAbstractSocket::SocketError)),
           m_session, SLOT(socketError()) );

  connect( this, SIGNAL(responseReceived(KIMAP::Message)),
           m_session, SLOT(responseReceived(KIMAP::Message)) );

  QTimer::singleShot( 0, this, SLOT( reconnect() ) );
  exec();

  delete m_stream;
  delete m_socket;
}

#include "sessionthread_p.moc"

