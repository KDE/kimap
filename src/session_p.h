/*
    SPDX-FileCopyrightText: 2009 Kevin Ottens <ervin@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "session.h"
#include "sessionuiproxy.h"

#include <QObject>
#include <QQueue>
#include <QSslSocket>
#include <QString>
#include <QTimer>

class KJob;

namespace KIMAP
{
class Job;
struct Response;
class SessionLogger;
class SessionThread;

class KIMAP_EXPORT SessionPrivate : public QObject
{
    Q_OBJECT

    friend class Session;

public:
    explicit SessionPrivate(Session *session);
    ~SessionPrivate() override;

    void addJob(Job *job);
    QByteArray sendCommand(const QByteArray &command, const QByteArray &args = QByteArray());
    void startSsl(QSsl::SslProtocol protocol);
    void sendData(const QByteArray &data);

    QSsl::SslProtocol negotiatedEncryption() const;

    void setSocketTimeout(int ms);
    int socketTimeout() const;

Q_SIGNALS:
    void encryptionNegotiationResult(bool);

private Q_SLOTS:
    void onEncryptionNegotiationResult(bool isEncrypted, QSsl::SslProtocol sslVersion);
    void onSocketTimeout();

    void doStartNext();
    void jobDone(KJob *);
    void jobDestroyed(QObject *);
    void responseReceived(const KIMAP::Response &);

    void socketConnected();
    void socketDisconnected();
    void socketError(QAbstractSocket::SocketError error);
    void socketActivity();

    void handleSslError(const KSslErrorUiData &errorData);

private:
    void startNext();
    void clearJobQueue();
    void setState(Session::State state);

    void startSocketTimer();
    void stopSocketTimer();
    void restartSocketTimer();
    bool isConnected() const;

    Session *const q;

    bool isSocketConnected = false;
    Session::State state;

    SessionLogger *logger = nullptr;
    SessionThread *thread = nullptr;
    SessionUiProxy::Ptr uiProxy;

    bool jobRunning = false;
    Job *currentJob = nullptr;
    QQueue<Job *> queue;

    QByteArray authTag;
    QByteArray selectTag;
    QByteArray closeTag;

    QString userName;
    QByteArray greeting;
    QByteArray currentMailBox;
    QByteArray upcomingMailBox;
    quint16 tagCount;

    QSsl::SslProtocol sslVersion;

    int socketTimerInterval = 0;
    QTimer socketTimer;
};

}

