/*
    SPDX-FileCopyrightText: 2009 Kevin Ottens <ervin@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "sessionthread_p.h"

#include <KSslErrorUiData>

#include "kimap_debug.h"
#include <QDebug>
#include <QNetworkProxy>
#include <QSslCipher>
#include <QThread>

#include "imapstreamparser.h"
#include "response_p.h"

using namespace KIMAP;

Q_DECLARE_METATYPE(KSslErrorUiData)

namespace
{
static const int _kimap_abstractSocketError = qRegisterMetaType<QAbstractSocket::SocketError>();
static const int _kimap_sslErrorUiData = qRegisterMetaType<KSslErrorUiData>();
}

SessionThread::SessionThread(const QString &hostName, quint16 port)
    : QObject()
    , m_hostName(hostName)
    , m_port(port)
{
    // Just like the Qt docs now recommend, for event-driven threads:
    // don't derive from QThread, create one directly and move the object to it.
    auto thread = new QThread();
    moveToThread(thread);
    thread->start();
    QMetaObject::invokeMethod(this, &SessionThread::threadInit);
}

SessionThread::~SessionThread()
{
    QMetaObject::invokeMethod(this, &SessionThread::threadQuit);
    if (!thread()->wait(10 * 1000)) {
        qCWarning(KIMAP_LOG) << "Session thread refuses to die, killing harder...";
        thread()->terminate();
        // Make sure to wait until it's done, otherwise it can crash when the pthread callback is called
        thread()->wait();
    }
    delete thread();
}

// Called in primary thread, passes setting to secondary thread
void SessionThread::setUseNetworkProxy(bool useProxy)
{
    QMetaObject::invokeMethod(
        this,
        [this, useProxy]() {
            setUseProxyInternal(useProxy);
        },
        Qt::QueuedConnection);
}

// Called in primary thread
void SessionThread::sendData(const QByteArray &payload)
{
    QMutexLocker locker(&m_mutex);

    m_dataQueue.enqueue(payload);
    QMetaObject::invokeMethod(this, &SessionThread::writeDataQueue);
}

// Called in secondary thread
void SessionThread::writeDataQueue()
{
    Q_ASSERT(QThread::currentThread() == thread());
    if (!m_socket) {
        return;
    }
    QMutexLocker locker(&m_mutex);

    while (!m_dataQueue.isEmpty()) {
        m_socket->write(m_dataQueue.dequeue());
    }
}

// Called in secondary thread
void SessionThread::readMessage()
{
    Q_ASSERT(QThread::currentThread() == thread());
    if (!m_stream || m_stream->availableDataSize() == 0) {
        return;
    }

    Response message;
    QList<Response::Part> *payload = &message.content;

    try {
        while (!m_stream->atCommandEnd()) {
            if (m_stream->hasString()) {
                QByteArray string = m_stream->readString();
                if (string == "NIL") {
                    *payload << Response::Part(QList<QByteArray>());
                } else {
                    *payload << Response::Part(string);
                }
            } else if (m_stream->hasList()) {
                *payload << Response::Part(m_stream->readParenthesizedList());
            } else if (m_stream->hasResponseCode()) {
                payload = &message.responseCode;
            } else if (m_stream->atResponseCodeEnd()) {
                payload = &message.content;
            } else if (m_stream->hasLiteral()) {
                QByteArray literal;
                while (!m_stream->atLiteralEnd()) {
                    literal += m_stream->readLiteralPart();
                }
                *payload << Response::Part(literal);
            } else {
                // Oops! Something really bad happened, we won't be able to recover
                // so close the socket immediately
                qWarning("Inconsistent state, probably due to some packet loss");
                doCloseSocket();
                return;
            }
        }

        Q_EMIT responseReceived(message);

    } catch (const KIMAP::ImapParserException &e) {
        qCWarning(KIMAP_LOG) << "The stream parser raised an exception:" << e.what();
    }

    if (m_stream->availableDataSize() > 1) {
        QMetaObject::invokeMethod(this, &SessionThread::readMessage, Qt::QueuedConnection);
    }
}

// Called in main thread
void SessionThread::closeSocket()
{
    QMetaObject::invokeMethod(this, &SessionThread::doCloseSocket, Qt::QueuedConnection);
}

// Called in secondary thread
void SessionThread::doCloseSocket()
{
    Q_ASSERT(QThread::currentThread() == thread());
    if (!m_socket) {
        return;
    }
    m_encryptedMode = false;
    qCDebug(KIMAP_LOG) << "close";
    m_socket->close();
}

// Called in secondary thread
void SessionThread::reconnect()
{
    Q_ASSERT(QThread::currentThread() == thread());
    if (m_socket == nullptr) { // threadQuit already called
        return;
    }
    if (m_socket->state() != QSslSocket::ConnectedState && m_socket->state() != QSslSocket::ConnectingState) {
        QNetworkProxy proxy;
        if (!m_useProxy) {
            qCDebug(KIMAP_LOG) << "Connecting to IMAP server with no proxy";
            proxy.setType(QNetworkProxy::NoProxy);
        } else {
            qCDebug(KIMAP_LOG) << "Connecting to IMAP server using default system proxy";
            proxy.setType(QNetworkProxy::DefaultProxy);
        }
        m_socket->setProxy(proxy);

        if (m_encryptedMode) {
            qCDebug(KIMAP_LOG) << "connectToHostEncrypted" << m_hostName << m_port;
            m_socket->connectToHostEncrypted(m_hostName, m_port);
        } else {
            qCDebug(KIMAP_LOG) << "connectToHost" << m_hostName << m_port;
            m_socket->connectToHost(m_hostName, m_port);
        }
    }
}

// Called in secondary thread
void SessionThread::threadInit()
{
    Q_ASSERT(QThread::currentThread() == thread());
    m_socket = std::make_unique<QSslSocket>();
    m_stream = std::make_unique<ImapStreamParser>(m_socket.get());
    connect(m_socket.get(), &QIODevice::readyRead, this, &SessionThread::readMessage, Qt::QueuedConnection);

    // Delay the call to slotSocketDisconnected so that it finishes disconnecting before we call reconnect()
    connect(m_socket.get(), &QSslSocket::disconnected, this, &SessionThread::slotSocketDisconnected, Qt::QueuedConnection);
    connect(m_socket.get(), &QSslSocket::connected, this, &SessionThread::socketConnected);
    connect(m_socket.get(),
            QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::errorOccurred),
            this,
            &SessionThread::slotSocketError);

    connect(m_socket.get(), &QIODevice::bytesWritten, this, &SessionThread::socketActivity);
    connect(m_socket.get(), &QSslSocket::encryptedBytesWritten, this, &SessionThread::socketActivity);
    connect(m_socket.get(), &QIODevice::readyRead, this, &SessionThread::socketActivity);
    QMetaObject::invokeMethod(this, &SessionThread::reconnect, Qt::QueuedConnection);
}

// Called in secondary thread
void SessionThread::threadQuit()
{
    Q_ASSERT(QThread::currentThread() == thread());
    m_stream.reset();
    m_socket.reset();
    thread()->quit();
}

// Called in secondary thread
void SessionThread::setUseProxyInternal(bool useProxy)
{
    m_useProxy = useProxy;
    if (m_socket != nullptr) {
        if (m_socket->state() != QSslSocket::UnconnectedState) {
            m_socket->disconnectFromHost();
            QMetaObject::invokeMethod(this, &SessionThread::reconnect, Qt::QueuedConnection);
        }
    }
}

// Called in primary thread
void SessionThread::startSsl(QSsl::SslProtocol protocol)
{
    QMetaObject::invokeMethod(this, [this, protocol]() {
        doStartSsl(protocol);
    });
}

// Called in secondary thread (via invokeMethod)
void SessionThread::doStartSsl(QSsl::SslProtocol protocol)
{
    Q_ASSERT(QThread::currentThread() == thread());
    if (!m_socket) {
        return;
    }

    m_socket->setProtocol(protocol);
    m_socket->ignoreSslErrors(); // Don't worry, errors are handled manually below
    connect(m_socket.get(), &QSslSocket::encrypted, this, &SessionThread::sslConnected);
    m_socket->startClientEncryption();
}

// Called in secondary thread
void SessionThread::slotSocketDisconnected()
{
    Q_ASSERT(QThread::currentThread() == thread());
    Q_EMIT socketDisconnected();
}

// Called in secondary thread
void SessionThread::slotSocketError(QAbstractSocket::SocketError error)
{
    Q_ASSERT(QThread::currentThread() == thread());
    if (!m_socket) {
        return;
    }
    Q_EMIT socketError(error);
}

// Called in secondary thread
void SessionThread::sslConnected()
{
    Q_ASSERT(QThread::currentThread() == thread());
    if (!m_socket) {
        return;
    }
    QSslCipher cipher = m_socket->sessionCipher();
    if (!m_socket->sslHandshakeErrors().isEmpty()
        || !m_socket->isEncrypted() || cipher.isNull() || cipher.usedBits() == 0) {
        qCDebug(KIMAP_LOG) << "Initial SSL handshake failed. cipher.isNull() is" << cipher.isNull() << ", cipher.usedBits() is" << cipher.usedBits()
                           << ", the socket says:" << m_socket->errorString() << "and the list of SSL errors contains"
                           << m_socket->sslHandshakeErrors().count()
                           << "items.";
        KSslErrorUiData errorData(m_socket.get());
        Q_EMIT sslError(errorData);
    } else {
        qCDebug(KIMAP_LOG) << "TLS negotiation done, the negotiated protocol is" << cipher.protocolString();
        m_encryptedMode = true;
        Q_EMIT encryptionNegotiationResult(true, m_socket->sessionProtocol());
    }
}

void SessionThread::sslErrorHandlerResponse(bool response)
{
    QMetaObject::invokeMethod(this, [this, response]() {
        doSslErrorHandlerResponse(response);
    });
}

// Called in secondary thread (via invokeMethod)
void SessionThread::doSslErrorHandlerResponse(bool response)
{
    Q_ASSERT(QThread::currentThread() == thread());
    if (!m_socket) {
        return;
    }
    if (response) {
        m_encryptedMode = true;
        Q_EMIT encryptionNegotiationResult(true, m_socket->sessionProtocol());
    } else {
        m_encryptedMode = false;
        // reconnect in unencrypted mode, so new commands can be issued
        m_socket->disconnectFromHost();
        m_socket->waitForDisconnected();
        m_socket->connectToHost(m_hostName, m_port);
        Q_EMIT encryptionNegotiationResult(false, QSsl::UnknownProtocol);
    }
}

#include "moc_sessionthread_p.cpp"
