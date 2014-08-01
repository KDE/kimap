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

#include <QtCore/QMutex>
#include <QtCore/QQueue>

#include <ktcpsocket.h>

typedef KTcpSocket SessionSocket;

namespace KIMAP
{

class ImapStreamParser;
struct Message;

class SessionThread : public QObject
{
    Q_OBJECT

public:
    explicit SessionThread(const QString &hostName, quint16 port);
    ~SessionThread();

    inline QString hostName()
    {
        return m_hostName;
    }
    inline quint16 port()
    {
        return m_port;
    }

    void sendData(const QByteArray &payload);

public Q_SLOTS:
    void closeSocket();
    void startSsl(KTcpSocket::SslVersion version);
    void sslErrorHandlerResponse(bool result);

Q_SIGNALS:
    void socketConnected();
    void socketDisconnected();
    void socketActivity();
    void socketError(KTcpSocket::Error);
    void responseReceived(const KIMAP::Message &response);
    void encryptionNegotiationResult(bool, KTcpSocket::SslVersion);
    void sslError(const KSslErrorUiData &);

private Q_SLOTS:
    void reconnect();
    void threadInit();
    void threadQuit();
    void readMessage();
    void writeDataQueue();
    void sslConnected();
    void doCloseSocket();
    void slotSocketError(KTcpSocket::Error);
    void slotSocketDisconnected();
    void doStartSsl(KTcpSocket::SslVersion);
    void doSslErrorHandlerResponse(bool result);

private:
    QString m_hostName;
    quint16 m_port;

    SessionSocket *m_socket;
    ImapStreamParser *m_stream;

    QQueue<QByteArray> m_dataQueue;

    // Protects m_dataQueue
    QMutex m_mutex;

    bool m_encryptedMode;
};

}

#endif
