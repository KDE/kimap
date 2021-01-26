/*
    SPDX-FileCopyrightText: 2009 Kevin Ottens <ervin@kde.org>

    SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
    SPDX-FileContributor: Kevin Ottens <kevin@kdab.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "session.h"
#include "session_p.h"

#include <QPointer>
#include <QTimer>

#include "kimap_debug.h"

#include "job.h"
#include "job_p.h"
#include "loginjob.h"
#include "response_p.h"
#include "rfccodecs.h"
#include "sessionlogger_p.h"
#include "sessionthread_p.h"

Q_DECLARE_METATYPE(QSsl::SslProtocol)
Q_DECLARE_METATYPE(QSslSocket::SslMode)
static const int _kimap_sslVersionId = qRegisterMetaType<QSsl::SslProtocol>();

using namespace KIMAP;

Session::Session(const QString &hostName, quint16 port, QObject *parent)
    : QObject(parent)
    , d(new SessionPrivate(this))
{
    if (!qEnvironmentVariableIsEmpty("KIMAP_LOGFILE")) {
        d->logger = new SessionLogger;
    }

    d->isSocketConnected = false;
    d->state = Disconnected;
    d->jobRunning = false;

    d->thread = new SessionThread(hostName, port);
    connect(d->thread, &SessionThread::encryptionNegotiationResult, d, &SessionPrivate::onEncryptionNegotiationResult);
    connect(d->thread, &SessionThread::sslError, d, &SessionPrivate::handleSslError);
    connect(d->thread, &SessionThread::socketDisconnected, d, &SessionPrivate::socketDisconnected);
    connect(d->thread, &SessionThread::responseReceived, d, &SessionPrivate::responseReceived);
    connect(d->thread, &SessionThread::socketConnected, d, &SessionPrivate::socketConnected);
    connect(d->thread, &SessionThread::socketActivity, d, &SessionPrivate::socketActivity);
    connect(d->thread, &SessionThread::socketError, d, &SessionPrivate::socketError);

    d->socketTimer.setSingleShot(true);
    connect(&d->socketTimer, &QTimer::timeout, d, &SessionPrivate::onSocketTimeout);

    d->startSocketTimer();
}

Session::~Session()
{
    // Make sure all jobs know we're done
    d->socketDisconnected();
    delete d->thread;
    d->thread = nullptr;
}

void Session::setUiProxy(const SessionUiProxy::Ptr &proxy)
{
    d->uiProxy = proxy;
}

void Session::setUiProxy(SessionUiProxy *proxy)
{
    setUiProxy(SessionUiProxy::Ptr(proxy));
}

QString Session::hostName() const
{
    return d->thread->hostName();
}

quint16 Session::port() const
{
    return d->thread->port();
}

void Session::setUseNetworkProxy(bool useProxy)
{
    d->thread->setUseNetworkProxy(useProxy);
}

Session::State Session::state() const
{
    return d->state;
}

QString Session::userName() const
{
    return d->userName;
}

QByteArray Session::serverGreeting() const
{
    return d->greeting;
}

int Session::jobQueueSize() const
{
    return d->queue.size() + (d->jobRunning ? 1 : 0);
}

void KIMAP::Session::close()
{
    d->thread->closeSocket();
}

void SessionPrivate::handleSslError(const KSslErrorUiData &errorData)
{
    // ignoreSslError is async, so the thread might already be gone when it returns
    QPointer<SessionThread> _t = thread;
    const bool ignoreSslError = uiProxy && uiProxy->ignoreSslError(errorData);
    if (_t) {
        _t->sslErrorHandlerResponse(ignoreSslError);
    }
}

SessionPrivate::SessionPrivate(Session *session)
    : QObject(session)
    , q(session)
    , isSocketConnected(false)
    , state(Session::Disconnected)
    , logger(nullptr)
    , thread(nullptr)
    , jobRunning(false)
    , currentJob(nullptr)
    , tagCount(0)
    , sslVersion(QSsl::UnknownProtocol)
    , socketTimerInterval(30000) // By default timeouts on 30s
{
}

SessionPrivate::~SessionPrivate()
{
    delete logger;
}

void SessionPrivate::addJob(Job *job)
{
    queue.append(job);
    Q_EMIT q->jobQueueSizeChanged(q->jobQueueSize());

    QObject::connect(job, &KJob::result, this, &SessionPrivate::jobDone);
    QObject::connect(job, &QObject::destroyed, this, &SessionPrivate::jobDestroyed);

    if (state != Session::Disconnected) {
        startNext();
    }
}

void SessionPrivate::startNext()
{
    QMetaObject::invokeMethod(this, &SessionPrivate::doStartNext);
}

void SessionPrivate::doStartNext()
{
    if (queue.isEmpty() || jobRunning || !isSocketConnected) {
        return;
    }

    restartSocketTimer();
    jobRunning = true;

    currentJob = queue.dequeue();
    currentJob->doStart();
}

void SessionPrivate::jobDone(KJob *job)
{
    Q_UNUSED(job)
    Q_ASSERT(job == currentJob);

    stopSocketTimer();

    jobRunning = false;
    currentJob = nullptr;
    Q_EMIT q->jobQueueSizeChanged(q->jobQueueSize());
    startNext();
}

void SessionPrivate::jobDestroyed(QObject *job)
{
    queue.removeAll(static_cast<KIMAP::Job *>(job));
    if (currentJob == job) {
        currentJob = nullptr;
    }
}

void SessionPrivate::responseReceived(const Response &response)
{
    if (logger && isConnected()) {
        logger->dataReceived(response.toString());
    }

    QByteArray tag;
    QByteArray code;

    if (response.content.size() >= 1) {
        tag = response.content[0].toString();
    }

    if (response.content.size() >= 2) {
        code = response.content[1].toString();
    }

    // BYE may arrive as part of a LOGOUT sequence or before the server closes the connection after an error.
    // In any case we should wait until the server closes the connection, so we don't have to do anything.
    if (code == "BYE") {
        Response simplified = response;
        if (simplified.content.size() >= 2) {
            simplified.content.removeFirst(); // Strip the tag
            simplified.content.removeFirst(); // Strip the code
        }
        qCDebug(KIMAP_LOG) << "Received BYE: " << simplified.toString();
        return;
    }

    switch (state) {
    case Session::Disconnected:
        if (socketTimer.isActive()) {
            stopSocketTimer();
        }
        if (code == "OK") {
            setState(Session::NotAuthenticated);

            Response simplified = response;
            simplified.content.removeFirst(); // Strip the tag
            simplified.content.removeFirst(); // Strip the code
            greeting = simplified.toString().trimmed(); // Save the server greeting
            startNext();
        } else if (code == "PREAUTH") {
            setState(Session::Authenticated);

            Response simplified = response;
            simplified.content.removeFirst(); // Strip the tag
            simplified.content.removeFirst(); // Strip the code
            greeting = simplified.toString().trimmed(); // Save the server greeting

            startNext();
        } else {
            thread->closeSocket();
        }
        return;
    case Session::NotAuthenticated:
        if (code == "OK" && tag == authTag) {
            setState(Session::Authenticated);
        }
        break;
    case Session::Authenticated:
        if (code == "OK" && tag == selectTag) {
            setState(Session::Selected);
            currentMailBox = upcomingMailBox;
        }
        break;
    case Session::Selected:
        if ((code == "OK" && tag == closeTag) || (code != "OK" && tag == selectTag)) {
            setState(Session::Authenticated);
            currentMailBox = QByteArray();
        } else if (code == "OK" && tag == selectTag) {
            currentMailBox = upcomingMailBox;
        }
        break;
    }

    if (tag == authTag) {
        authTag.clear();
    }
    if (tag == selectTag) {
        selectTag.clear();
    }
    if (tag == closeTag) {
        closeTag.clear();
    }

    // If a job is running forward it the response
    if (currentJob != nullptr) {
        restartSocketTimer();
        currentJob->handleResponse(response);
    } else {
        qCWarning(KIMAP_LOG) << "A message was received from the server with no job to handle it:" << response.toString()
                             << '(' + response.toString().toHex() + ')';
    }
}

void SessionPrivate::setState(Session::State s)
{
    if (s != state) {
        Session::State oldState = state;
        state = s;
        Q_EMIT q->stateChanged(state, oldState);
    }
}

QByteArray SessionPrivate::sendCommand(const QByteArray &command, const QByteArray &args)
{
    QByteArray tag = 'A' + QByteArray::number(++tagCount).rightJustified(6, '0');

    QByteArray payload = tag + ' ' + command;
    if (!args.isEmpty()) {
        payload += ' ' + args;
    }

    sendData(payload);

    if (command == "LOGIN" || command == "AUTHENTICATE") {
        authTag = tag;
    } else if (command == "SELECT" || command == "EXAMINE") {
        selectTag = tag;
        upcomingMailBox = args;
        upcomingMailBox.remove(0, 1);
        upcomingMailBox = upcomingMailBox.left(upcomingMailBox.indexOf('\"'));
        upcomingMailBox = KIMAP::decodeImapFolderName(upcomingMailBox);
    } else if (command == "CLOSE") {
        closeTag = tag;
    }
    return tag;
}

void SessionPrivate::sendData(const QByteArray &data)
{
    restartSocketTimer();

    if (logger && isConnected()) {
        logger->dataSent(data);
    }

    thread->sendData(data + "\r\n");
}

void SessionPrivate::socketConnected()
{
    stopSocketTimer();
    isSocketConnected = true;

    bool willUseSsl = false;
    if (!queue.isEmpty()) {
        auto login = qobject_cast<KIMAP::LoginJob *>(queue.first());
        if (login) {
            willUseSsl = (login->encryptionMode() == KIMAP::LoginJob::SSLorTLS);

            userName = login->userName();
        }
    }

    if (state == Session::Disconnected && willUseSsl) {
        startNext();
    } else {
        startSocketTimer();
    }
}

bool SessionPrivate::isConnected() const
{
    return state == Session::Authenticated || state == Session::Selected;
}

void SessionPrivate::socketDisconnected()
{
    if (socketTimer.isActive()) {
        stopSocketTimer();
    }

    if (logger && isConnected()) {
        logger->disconnectionOccured();
    }

    if (isSocketConnected) {
        setState(Session::Disconnected);
        Q_EMIT q->connectionLost();
    } else {
        Q_EMIT q->connectionFailed();
    }

    isSocketConnected = false;

    clearJobQueue();
}

void SessionPrivate::socketActivity()
{
    restartSocketTimer();
}

void SessionPrivate::socketError(QAbstractSocket::SocketError error)
{
    if (socketTimer.isActive()) {
        stopSocketTimer();
    }

    if (currentJob) {
        currentJob->d_ptr->setSocketError(error);
    } else if (!queue.isEmpty()) {
        currentJob = queue.takeFirst();
        currentJob->d_ptr->setSocketError(error);
    }

    if (isSocketConnected) {
        thread->closeSocket();
    } else {
        Q_EMIT q->connectionFailed();
        clearJobQueue();
    }
}

void SessionPrivate::clearJobQueue()
{
    if (currentJob) {
        currentJob->connectionLost();
    } else if (!queue.isEmpty()) {
        currentJob = queue.takeFirst();
        currentJob->connectionLost();
    }

    QQueue<Job *> queueCopy = queue; // copy because jobDestroyed calls removeAll
    qDeleteAll(queueCopy);
    queue.clear();
    Q_EMIT q->jobQueueSizeChanged(0);
}

void SessionPrivate::startSsl(QSsl::SslProtocol protocol)
{
    thread->startSsl(protocol);
}

QString Session::selectedMailBox() const
{
    return QString::fromUtf8(d->currentMailBox);
}

void SessionPrivate::onEncryptionNegotiationResult(bool isEncrypted, QSsl::SslProtocol protocol)
{
    if (isEncrypted) {
        sslVersion = protocol;
    } else {
        sslVersion = QSsl::UnknownProtocol;
    }
    Q_EMIT encryptionNegotiationResult(isEncrypted);
}

QSsl::SslProtocol SessionPrivate::negotiatedEncryption() const
{
    return sslVersion;
}

void SessionPrivate::setSocketTimeout(int ms)
{
    bool timerActive = socketTimer.isActive();

    if (timerActive) {
        stopSocketTimer();
    }

    socketTimerInterval = ms;

    if (timerActive) {
        startSocketTimer();
    }
}

int SessionPrivate::socketTimeout() const
{
    return socketTimerInterval;
}

void SessionPrivate::startSocketTimer()
{
    if (socketTimerInterval < 0) {
        return;
    }
    Q_ASSERT(!socketTimer.isActive());

    socketTimer.start(socketTimerInterval);
}

void SessionPrivate::stopSocketTimer()
{
    if (socketTimerInterval < 0) {
        return;
    }

    socketTimer.stop();
}

void SessionPrivate::restartSocketTimer()
{
    if (socketTimer.isActive()) {
        stopSocketTimer();
    }
    startSocketTimer();
}

void SessionPrivate::onSocketTimeout()
{
    qCDebug(KIMAP_LOG) << "Socket timeout!";
    thread->closeSocket();
}

void Session::setTimeout(int timeout)
{
    d->setSocketTimeout(timeout * 1000);
}

int Session::timeout() const
{
    return d->socketTimeout() / 1000;
}

#include "moc_session.cpp"
#include "moc_session_p.cpp"
