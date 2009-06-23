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

#include <ktcpsocket.h>

#include <QtCore/QObject>
#include <QtCore/QQueue>
#include <QtCore/QString>

class KJob;

namespace KIMAP {

class Job;
struct Message;
class SessionThread;
class SessionUiProxy;

class SessionPrivate : public QObject
{
  Q_OBJECT

  friend class Session;

  public:
    SessionPrivate( Session *session );

    void addJob(Job *job);
    QByteArray sendCommand( const QByteArray &command, const QByteArray &args = QByteArray() );
    void startSsl(const KTcpSocket::SslVersion &version);
    void sendData( const QByteArray &data );

    QString selectedMailBox() const;

    void handleSslError( const KSslErrorUiData &errorData );

  Q_SIGNALS:
    void encryptionNegotiationResult(bool);

  private:
    void reconnect();

    void startNext();
    void doStartNext();
    void jobDone( KJob *job );
    void jobDestroyed( QObject *job );
    void responseReceived( const KIMAP::Message &response );

    void socketConnected();
    void socketDisconnected();
    void socketError();

    Session *const q;

    Session::State state;

    SessionThread *thread;
    SessionUiProxy *uiProxy;

    bool jobRunning;
    Job *currentJob;
    QQueue<Job*> queue;

    QByteArray authTag;
    QByteArray selectTag;
    QByteArray closeTag;

    QByteArray currentMailBox;
    QByteArray upcomingMailBox;
    quint16 tagCount;
};

}

#endif
