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
#include <QtCore/QIODevice>
#include "imapstreamparser.h"
#include "message_p.h"

using namespace KIMAP;

SessionThread::SessionThread( QObject *parent )
  : QThread(parent), m_device(0), m_stream(0), m_quit(false)
{

}

SessionThread::~SessionThread()
{
  m_quit = true;
  m_cond.wakeOne();
  wait();
  delete m_stream;
  delete m_device;
}

void SessionThread::requestResponse()
{
  QMutexLocker locker(&m_mutex);

  if ( !isRunning() ) {
    start();
  } else {
    m_cond.wakeOne();
  }
}

void SessionThread::run()
{
  while ( !m_quit ) {
    QMutexLocker locker(&m_mutex);

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

    if ( m_stream->availableDataSize()==0 ) {
      m_cond.wait(&m_mutex);
    }
  }
}

void SessionThread::setDevice( QIODevice *device )
{
  QMutexLocker locker(&m_mutex);
  delete m_stream;
  delete m_device;

  m_device = device;

  if ( m_device!=0 ) {
    m_device->setParent(0);

    m_stream = new ImapStreamParser( device );
    connect( m_device, SIGNAL(readyRead()), this, SLOT(requestResponse()) );
  } else {
    m_stream = 0;
  }
}

#include "sessionthread_p.moc"

