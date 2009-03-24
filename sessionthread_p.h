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

#ifndef KIMAP_SESSIONTHREAD_P_H
#define KIMAP_SESSIONTHREAD_P_H

#include <QtCore/QThread>
#include <QtCore/QMutex>
#include <QtCore/QWaitCondition>

class QIODevice;

namespace KIMAP {

class ImapStreamParser;
class Message;

class SessionThread : public QThread
{
  Q_OBJECT

  public:
    explicit SessionThread( QObject *parent = 0 );
    ~SessionThread();

    void setDevice( QIODevice *device );

    void sendCommand( const QByteArray &command );
    void run();

  signals:
    void responseReceived(const KIMAP::Message &response);

  private slots:
    void requestResponse();

  private:
    QIODevice *m_device;
    ImapStreamParser *m_stream;

    QMutex m_mutex;
    QWaitCondition m_cond;
    bool m_quit;
};

}

#endif
