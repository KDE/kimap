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
#include "sessionuiproxy.h"

#include <ktcpsocket.h>

#include <QtCore/QObject>
#include <QtCore/QQueue>
#include <QtCore/QString>
#include <QtCore/QTimer>

class KJob;

namespace KIMAP
{

class Job;
struct Message;
class SessionLogger;
class SessionThread;

class KIMAP_EXPORT SessionPrivate : public QObject
{
    Q_OBJECT

    friend class Session;

public:
    explicit SessionPrivate(Session *session);
    virtual ~SessionPrivate();

    void addJob(Job *job);
    QByteArray sendCommand(const QByteArray &command, const QByteArray &args = QByteArray());
    void startSsl(const KTcpSocket::SslVersion &version);
    void sendData(const QByteArray &data);

    KTcpSocket::SslVersion negotiatedEncryption() const;

    void setSocketTimeout(int ms);
    int socketTimeout() const;

Q_SIGNALS:
    void encryptionNegotiationResult(bool);

private Q_SLOTS:
    void onEncryptionNegotiationResult(bool isEncrypted, KTcpSocket::SslVersion sslVersion);
    void onSocketTimeout();

    void doStartNext();
    void jobDone(KJob *);
    void jobDestroyed(QObject *);
    void responseReceived(const KIMAP::Message &);

    void socketConnected();
    void socketDisconnected();
    void socketError(KTcpSocket::Error);
    void socketActivity();

    void handleSslError(const KSslErrorUiData &errorData);

private:
    void startNext();
    void clearJobQueue();
    void setState(Session::State state);

    void startSocketTimer();
    void stopSocketTimer();
    void restartSocketTimer();

    Session *const q;

    bool isSocketConnected;
    Session::State state;

    SessionLogger *logger;
    SessionThread *thread;
    SessionUiProxy::Ptr uiProxy;

    bool jobRunning;
    Job *currentJob;
    QQueue<Job *> queue;

    QByteArray authTag;
    QByteArray selectTag;
    QByteArray closeTag;

    QString userName;
    QByteArray greeting;
    QByteArray currentMailBox;
    QByteArray upcomingMailBox;
    quint16 tagCount;

    KTcpSocket::SslVersion sslVersion;

    int socketTimerInterval;
    QTimer socketTimer;
};

}

#endif
