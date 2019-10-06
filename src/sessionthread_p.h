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

#include <QMutex>
#include <QQueue>

#include <QSslSocket>

#include <memory>

class KSslErrorUiData;

namespace KIMAP
{

class ImapStreamParser;
struct Response;

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

    void setUseNetworkProxy(bool useProxy);

    void sendData(const QByteArray &payload);

public Q_SLOTS:
    void closeSocket();
    void startSsl(QSsl::SslProtocol protocol);
    void sslErrorHandlerResponse(bool result);

Q_SIGNALS:
    void socketConnected();
    void socketDisconnected();
    void socketActivity();
    void socketError(QAbstractSocket::SocketError);
    void responseReceived(const KIMAP::Response &response);
    void encryptionNegotiationResult(bool, QSsl::SslProtocol);
    void sslError(const KSslErrorUiData &);

private Q_SLOTS:
    void reconnect();
    void threadInit();
    void threadQuit();
    void readMessage();
    void writeDataQueue();
    void sslConnected();
    void doCloseSocket();
    void slotSocketError(QAbstractSocket::SocketError);
    void slotSocketDisconnected();
    void doStartSsl(QSsl::SslProtocol);
    void doSslErrorHandlerResponse(bool result);
    void setUseProxyInternal(bool useProxy);

private:
    QString m_hostName;
    quint16 m_port;

    std::unique_ptr<QSslSocket> m_socket;
    std::unique_ptr<ImapStreamParser> m_stream;

    QQueue<QByteArray> m_dataQueue;

    // Protects m_dataQueue
    QMutex m_mutex;

    bool m_encryptedMode = false;
    bool m_useProxy = false;
};

}

#endif
