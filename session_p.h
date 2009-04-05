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

#ifndef KIMAP_SESSION_P_H
#define KIMAP_SESSION_P_H

#include "session.h"
#include <QtCore/QQueue>
#include <QtCore/QString>

class KJob;

namespace KIMAP {

class Job;
class Message;
class SessionThread;

class SessionPrivate
{
  friend class Session;

  public:
    SessionPrivate( Session *session );

    void addJob(Job *job);
    QByteArray sendCommand( const QByteArray &command, const QByteArray &args = QByteArray() );
    void startTls();
    void sendData( const QByteArray &data );

    QByteArray selectedMailBox() const;

  private:
    void reconnect();

    void startNext();
    void doStartNext();
    void jobDone( KJob *job );
    void jobDestroyed( QObject *job );
    void responseReceived( const KIMAP::Message &response );

    void socketDisconnected();
    void socketError();

    Session *const q;

    Session::State state;

    SessionThread *thread;

    bool jobRunning;
    Job *currentJob;
    QQueue<Job*> queue;

    QByteArray authTag;
    QByteArray selectTag;
    QByteArray closeTag;

    QByteArray currentMailBox;
    QByteArray upcomingMailBox;
};

}

#endif
