/*
   SPDX-FileCopyrightText: 2008 Omat Holding B.V. <info@omat.nl>

   SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
   SPDX-FileContributor: Kevin Ottens <kevin@kdab.com>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

// Own
#include "fakeserver.h"
#include "sslserver.h"

// Qt
#include <QDebug>

#include <QFile>
#include <QTcpServer>
#include <QTcpSocket>
#include <QTest>

#include "imapstreamparser.h"

QByteArray FakeServer::preauth()
{
    return "S: * PREAUTH localhost Test Library server ready";
}

QByteArray FakeServer::greeting()
{
    return "S: * OK localhost Test Library server ready";
}

FakeServer::FakeServer(QObject *parent)
    : QThread(parent)
    , m_encrypted(false)
    , m_starttls(false)
{
    moveToThread(this);
}

FakeServer::~FakeServer()
{
    quit();
    wait();
}

void FakeServer::startAndWait()
{
    start();
    // this will block until the event queue starts
    QMetaObject::invokeMethod(this, &FakeServer::started, Qt::BlockingQueuedConnection);
}

void FakeServer::dataAvailable()
{
    QMutexLocker locker(&m_mutex);

    auto socket = qobject_cast<QTcpSocket *>(sender());
    QVERIFY(socket);

    int scenarioNumber = m_clientSockets.indexOf(socket);

    if (m_scenarios[scenarioNumber].isEmpty()) {
        KIMAP::ImapStreamParser *clientParser = m_clientParsers[scenarioNumber];
        QByteArray received = "C: " + clientParser->readUntilCommandEnd().trimmed();
        qWarning() << "Scenario" << scenarioNumber << "finished, but we got command" << received;
        QVERIFY(false);
    }

    readClientPart(scenarioNumber);
    writeServerPart(scenarioNumber);
    if (m_starttls) {
        m_starttls = false;
        qDebug() << "start tls";
        static_cast<QSslSocket *>(socket)->startServerEncryption();
    }
}

void FakeServer::newConnection()
{
    QMutexLocker locker(&m_mutex);

    m_clientSockets << m_tcpServer->nextPendingConnection();
    connect(m_clientSockets.last(), SIGNAL(readyRead()), this, SLOT(dataAvailable()));
    m_clientParsers << new KIMAP::ImapStreamParser(m_clientSockets.last(), true);

    QVERIFY(m_clientSockets.size() <= m_scenarios.size());

    writeServerPart(m_clientSockets.size() - 1);
}

void FakeServer::setEncrypted(QSsl::SslProtocol protocol)
{
    m_encrypted = true;
    m_sslProtocol = protocol;
}

void FakeServer::setWaitForStartTls(bool wait)
{
    m_waitForStartTls = wait;
}

void FakeServer::run()
{
    if (m_encrypted) {
        m_tcpServer = new SslServer(m_sslProtocol, m_waitForStartTls);
    } else {
        m_tcpServer = new QTcpServer();
    }
    if (!m_tcpServer->listen(QHostAddress(QHostAddress::LocalHost), 5989)) {
        qFatal("Unable to start the server");
    }

    connect(m_tcpServer, SIGNAL(newConnection()), this, SLOT(newConnection()));

    exec();

    qDeleteAll(m_clientParsers);
    qDeleteAll(m_clientSockets);

    delete m_tcpServer;
}

void FakeServer::started()
{
    // do nothing: this is a dummy slot used by startAndWait()
}

void FakeServer::setScenario(const QList<QByteArray> &scenario)
{
    QMutexLocker locker(&m_mutex);

    m_scenarios.clear();
    m_scenarios << scenario;
}

void FakeServer::addScenario(const QList<QByteArray> &scenario)
{
    QMutexLocker locker(&m_mutex);

    m_scenarios << scenario;
}

void FakeServer::addScenarioFromFile(const QString &fileName)
{
    QFile file(fileName);
    file.open(QFile::ReadOnly);

    QList<QByteArray> scenario;

    // When loading from files we never have the authentication phase
    // force jumping directly to authenticated state.
    scenario << preauth();

    while (!file.atEnd()) {
        scenario << file.readLine().trimmed();
    }

    file.close();

    addScenario(scenario);
}

bool FakeServer::isScenarioDone(int scenarioNumber) const
{
    QMutexLocker locker(&m_mutex);

    if (scenarioNumber < m_scenarios.size()) {
        return m_scenarios[scenarioNumber].isEmpty();
    } else {
        return true; // Non existent hence empty, right?
    }
}

bool FakeServer::isAllScenarioDone() const
{
    QMutexLocker locker(&m_mutex);

    for (const QList<QByteArray> &scenario : std::as_const(m_scenarios)) {
        if (!scenario.isEmpty()) {
            return false;
        }
    }

    return true;
}

void FakeServer::writeServerPart(int scenarioNumber)
{
    QList<QByteArray> scenario = m_scenarios[scenarioNumber];
    QTcpSocket *clientSocket = m_clientSockets[scenarioNumber];

    while (!scenario.isEmpty() && (scenario.first().startsWith("S: ") || scenario.first().startsWith("W: "))) {
        QByteArray rule = scenario.takeFirst();

        if (rule.startsWith("S: ")) {
            QByteArray payload = rule.mid(3);
            clientSocket->write(payload + "\r\n");
        } else {
            int timeout = rule.mid(3).toInt();
            QTest::qWait(timeout);
        }
    }

    if (!scenario.isEmpty() && scenario.first().startsWith("X")) {
        scenario.takeFirst();
        clientSocket->close();
    }

    if (!scenario.isEmpty()) {
        QVERIFY(scenario.first().startsWith("C: "));
    }

    m_scenarios[scenarioNumber] = scenario;
}

void FakeServer::compareReceived(const QByteArray &received, const QByteArray &expected) const
{
    QCOMPARE(QString::fromUtf8(received), QString::fromUtf8(expected));
    QCOMPARE(received, expected);
}

void FakeServer::readClientPart(int scenarioNumber)
{
    QList<QByteArray> scenario = m_scenarios[scenarioNumber];
    KIMAP::ImapStreamParser *clientParser = m_clientParsers[scenarioNumber];

    while (!scenario.isEmpty() && scenario.first().startsWith("C: ")) {
        QByteArray received = "C: " + clientParser->readUntilCommandEnd().trimmed();
        QByteArray expected = scenario.takeFirst();
        if (expected.contains("C: SKIP")) {
            continue;
        }
        compareReceived(received, expected);
        if (received.contains("STARTTLS")) {
            m_starttls = true;
        }
    }

    if (!scenario.isEmpty()) {
        QVERIFY(scenario.first().startsWith("S: ") || scenario.first().startsWith("X"));
    }

    m_scenarios[scenarioNumber] = scenario;
}
