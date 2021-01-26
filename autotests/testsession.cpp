/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2008 Kevin Ottens <ervin@kde.org>

    SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
    SPDX-FileContributor: Kevin Ottens <kevin@kdab.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
  */

#include <QEventLoop>
#include <QObject>
#include <QSignalSpy>
#include <QTest>

#include "job.h"
#include "kimaptest/fakeserver.h"
#include "kimaptest/mockjob.h"
#include "session.h"

Q_DECLARE_METATYPE(KIMAP::Session::State)
Q_DECLARE_METATYPE(KJob *)

class SessionTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    void initTestCase()
    {
        qRegisterMetaType<KIMAP::Session::State>();
    }

    void shouldStartDisconnected()
    {
        FakeServer fakeServer;
        fakeServer.setScenario(QList<QByteArray>() << FakeServer::greeting());
        fakeServer.startAndWait();
        KIMAP::Session s(QStringLiteral("127.0.0.1"), 5989);
        QSignalSpy spy(&s, SIGNAL(stateChanged(KIMAP::Session::State, KIMAP::Session::State)));
        QCOMPARE((int)s.state(), (int)KIMAP::Session::Disconnected);
        QTest::qWait(600);
        QCOMPARE((int)s.state(), (int)KIMAP::Session::NotAuthenticated);
        QCOMPARE(spy.count(), 1); // NotAuthenticated
        QList<QVariant> arguments = spy.takeFirst();
        QCOMPARE((int)qvariant_cast<KIMAP::Session::State>(arguments.at(0)), (int)KIMAP::Session::NotAuthenticated);
        QCOMPARE((int)qvariant_cast<KIMAP::Session::State>(arguments.at(1)), (int)KIMAP::Session::Disconnected);
    }

    void shouldFailForInvalidHosts()
    {
        KIMAP::Session s(QStringLiteral("0.0.0.0"), 1234);
        s.setTimeout(1); // 1 second timout

        QSignalSpy spyFail(&s, SIGNAL(connectionFailed()));
        QSignalSpy spyLost(&s, SIGNAL(connectionLost()));
        QSignalSpy spyState(&s, SIGNAL(stateChanged(KIMAP::Session::State, KIMAP::Session::State)));

        QCOMPARE((int)s.state(), (int)KIMAP::Session::Disconnected);

        QTest::qWait(500);
        QCOMPARE((int)s.state(), (int)KIMAP::Session::Disconnected);
        QCOMPARE(spyFail.count(), 1);
        QCOMPARE(spyLost.count(), 0);
        QCOMPARE(spyState.count(), 0);

        // Wait 800ms more. So now it's 1.3 seconds, check that the socket timeout has correctly been
        // disabled, and that it hadn't fired unexpectedly.
        QTest::qWait(800);
        QCOMPARE(spyFail.count(), 1);
    }

    /**
      Checks that the timeout works when the connection succeeds, but the server doesn't sends anything
      back to the client. This could happen for example if we connected to a non-IMAP server.
    */
    void shouldTimeoutOnNoGreeting()
    {
        FakeServer fakeServer;
        fakeServer.setScenario(QList<QByteArray>());
        fakeServer.startAndWait();

        KIMAP::Session s(QStringLiteral("127.0.0.1"), 5989);
        s.setTimeout(2);
        QSignalSpy spyFail(&s, SIGNAL(connectionFailed()));
        QSignalSpy spyLost(&s, SIGNAL(connectionLost()));
        QSignalSpy spyState(&s, SIGNAL(stateChanged(KIMAP::Session::State, KIMAP::Session::State)));
        QCOMPARE((int)s.state(), (int)KIMAP::Session::Disconnected);

        // Wait 1.8 second. Since the timeout is set to 2 seconds, the socket should be still
        // disconnected at this point, yet the connectionFailed() signal shouldn't have been emitted.
        QTest::qWait(1800);
        QCOMPARE((int)s.state(), (int)KIMAP::Session::Disconnected);
        QCOMPARE(spyFail.count(), 0);
        QCOMPARE(spyLost.count(), 0);
        QCOMPARE(spyState.count(), 0);

        // Wait 0.5 second more. Now we are at 2.3 seconds, the socket should have timed out, and the
        // connectionFailed() signal should have been emitted.
        QTest::qWait(500);
        QCOMPARE((int)s.state(), (int)KIMAP::Session::Disconnected);
        QCOMPARE(spyFail.count(), 0);
        QCOMPARE(spyLost.count(), 1);
        QCOMPARE(spyState.count(), 0);
    }

    void shouldSupportPreauth()
    {
        FakeServer fakeServer;
        fakeServer.setScenario(QList<QByteArray>() << FakeServer::preauth());
        fakeServer.startAndWait();

        KIMAP::Session s(QStringLiteral("127.0.0.1"), 5989);
        QSignalSpy spy(&s, SIGNAL(stateChanged(KIMAP::Session::State, KIMAP::Session::State)));
        QCOMPARE((int)s.state(), (int)KIMAP::Session::Disconnected);
        QTest::qWait(500);
        QCOMPARE((int)s.state(), (int)KIMAP::Session::Authenticated);
        QCOMPARE(spy.count(), 1); // Authenticated
        QList<QVariant> arguments = spy.takeFirst();
        QCOMPARE((int)qvariant_cast<KIMAP::Session::State>(arguments.at(0)), (int)KIMAP::Session::Authenticated);
        QCOMPARE((int)qvariant_cast<KIMAP::Session::State>(arguments.at(1)), (int)KIMAP::Session::Disconnected);
    }

    void shouldRespectStartOrder()
    {
        FakeServer fakeServer;
        fakeServer.setScenario(QList<QByteArray>() << FakeServer::greeting());
        fakeServer.startAndWait();

        KIMAP::Session s(QStringLiteral("127.0.0.1"), 5989);
        auto j1 = new MockJob(&s);
        connect(j1, SIGNAL(result(KJob *)), this, SLOT(jobDone(KJob *)));
        auto j2 = new MockJob(&s);
        connect(j2, SIGNAL(result(KJob *)), this, SLOT(jobDone(KJob *)));
        auto j3 = new MockJob(&s);
        connect(j3, SIGNAL(result(KJob *)), this, SLOT(jobDone(KJob *)));
        auto j4 = new MockJob(&s);
        connect(j4, SIGNAL(result(KJob *)), this, SLOT(jobDone(KJob *)));

        j4->start();
        j2->start();
        j3->start();
        j1->start();

        m_expectedCalls = 4;
        m_eventLoop.exec();

        QCOMPARE(m_jobs.size(), 4);
        QCOMPARE(m_jobs[0], j4);
        QCOMPARE(m_jobs[1], j2);
        QCOMPARE(m_jobs[2], j3);
        QCOMPARE(m_jobs[3], j1);
    }

    void shouldManageQueueSize()
    {
        FakeServer fakeServer;
        fakeServer.setScenario(QList<QByteArray>() << FakeServer::greeting());
        fakeServer.startAndWait();

        KIMAP::Session s(QStringLiteral("127.0.0.1"), 5989);

        QSignalSpy queueSpy(&s, SIGNAL(jobQueueSizeChanged(int)));

        QCOMPARE(s.jobQueueSize(), 0);

        auto j1 = new MockJob(&s);
        auto j2 = new MockJob(&s);
        auto j3 = new MockJob(&s);
        auto j4 = new MockJob(&s);
        connect(j4, SIGNAL(result(KJob *)), &m_eventLoop, SLOT(quit()));

        QCOMPARE(s.jobQueueSize(), 0);

        j1->start();
        QCOMPARE(s.jobQueueSize(), 1);
        QCOMPARE(queueSpy.size(), 1);
        QCOMPARE(queueSpy.at(0).at(0).toInt(), 1);

        j2->start();
        QCOMPARE(s.jobQueueSize(), 2);
        QCOMPARE(queueSpy.size(), 2);
        QCOMPARE(queueSpy.at(1).at(0).toInt(), 2);

        j3->start();
        QCOMPARE(s.jobQueueSize(), 3);
        QCOMPARE(queueSpy.size(), 3);
        QCOMPARE(queueSpy.at(2).at(0).toInt(), 3);

        j4->start();
        QCOMPARE(s.jobQueueSize(), 4);
        QCOMPARE(queueSpy.size(), 4);
        QCOMPARE(queueSpy.at(3).at(0).toInt(), 4);

        queueSpy.clear();
        m_eventLoop.exec();

        QCOMPARE(s.jobQueueSize(), 0);

        QCOMPARE(queueSpy.at(0).at(0).toInt(), 3);
        QCOMPARE(queueSpy.at(1).at(0).toInt(), 2);
        QCOMPARE(queueSpy.at(2).at(0).toInt(), 1);
        QCOMPARE(queueSpy.at(3).at(0).toInt(), 0);
    }

    void shouldTimeoutOnNoReply()
    {
        FakeServer fakeServer;
        fakeServer.setScenario(QList<QByteArray>() << FakeServer::preauth() << "C: A000001 DUMMY"
                                                   << "S: * DUMMY"
                                                   << "S: * DUMMY"
                                                   << "S: * DUMMY"
                                                   << "S: * DUMMY"
                                                   << "S: * DUMMY"
                                                   << "S: * DUMMY"
                                                   << "S: * DUMMY"
                                                   << "S: * DUMMY"
                                                   << "S: * DUMMY"
                                                   << "S: * DUMMY"
                                                   << "S: * DUMMY"
                                                   << "S: * DUMMY"
                                                   << "S: * DUMMY"
                               // We never get a OK or anything, so the job can't normally complete
        );
        fakeServer.startAndWait();

        KIMAP::Session s(QStringLiteral("127.0.0.1"), 5989);

        QSignalSpy spyFail(&s, SIGNAL(connectionFailed()));
        QSignalSpy spyLost(&s, SIGNAL(connectionLost()));
        QSignalSpy spyState(&s, SIGNAL(stateChanged(KIMAP::Session::State, KIMAP::Session::State)));

        auto mock = new MockJob(&s);
        mock->setCommand("DUMMY");

        mock->exec();
        // We expect to get an error here due to some timeout
        QVERIFY(mock->error() != 0);
        QCOMPARE(spyFail.count(), 0);
        QCOMPARE(spyLost.count(), 1);
        QCOMPARE(spyState.count(), 2); // Authenticated, Disconnected
    }

    void shouldFailFirstJobOnConnectionFailed()
    {
        qRegisterMetaType<KJob *>();

        FakeServer fakeServer;
        fakeServer.setScenario(QList<QByteArray>());
        fakeServer.startAndWait();

        KIMAP::Session s(QStringLiteral("127.0.0.1"), 5989);
        s.setTimeout(1);

        auto j1 = new MockJob(&s);
        QSignalSpy spyResult1(j1, SIGNAL(result(KJob *)));
        QSignalSpy spyDestroyed1(j1, SIGNAL(destroyed()));

        auto j2 = new MockJob(&s);
        QSignalSpy spyResult2(j2, SIGNAL(result(KJob *)));
        QSignalSpy spyDestroyed2(j2, SIGNAL(destroyed()));

        auto j3 = new MockJob(&s);
        QSignalSpy spyResult3(j3, SIGNAL(result(KJob *)));
        QSignalSpy spyDestroyed3(j3, SIGNAL(destroyed()));

        j1->start();
        j2->start();
        j3->start();

        QCOMPARE(s.jobQueueSize(), 3);

        QTest::qWait(1100);

        // Check that only the first job has emitted it's result
        QCOMPARE(spyResult1.count(), 1);
        QCOMPARE(spyResult2.count(), 0);
        QCOMPARE(spyResult3.count(), 0);

        // Check that all jobs have been deleted
        QCOMPARE(spyDestroyed1.count(), 1);
        QCOMPARE(spyDestroyed2.count(), 1);
        QCOMPARE(spyDestroyed3.count(), 1);

        QCOMPARE(s.jobQueueSize(), 0);
    }

    void shouldCloseOnInconsistency()
    {
        for (int count = 0; count < 10; count++) {
            FakeServer fakeServer;
            fakeServer.setScenario(QList<QByteArray>() << FakeServer::preauth() << "C: A000001 DUMMY"
                                                       << "S: * DUMMY %"
                                                       << "S: DUMMY)");
            fakeServer.startAndWait();

            KIMAP::Session s(QStringLiteral("127.0.0.1"), 5989);

            QSignalSpy spyFail(&s, SIGNAL(connectionFailed()));
            QSignalSpy spyLost(&s, SIGNAL(connectionLost()));
            QSignalSpy spyState(&s, SIGNAL(stateChanged(KIMAP::Session::State, KIMAP::Session::State)));

            auto mock = new MockJob(&s);
            mock->setTimeout(5000);
            mock->setCommand("DUMMY");

            mock->setAutoDelete(false);
            mock->start();
            QTest::qWait(250); // Should be plenty

            // We expect to get an error here due to the inconsistency
            QVERIFY(mock->error() != 0);
            QCOMPARE(spyFail.count(), 0);
            QCOMPARE(spyLost.count(), 1);
            QCOMPARE(spyState.count(), 2); // Authenticated, Disconnected

            delete mock;
        }
    }

public Q_SLOTS:
    void jobDone(KJob *job)
    {
        m_jobs << job;

        if (m_expectedCalls == m_jobs.size()) {
            m_eventLoop.quit();
        }
    }

private:
    QEventLoop m_eventLoop;
    int m_expectedCalls;
    QList<KJob *> m_jobs;
};

QTEST_GUILESS_MAIN(SessionTest)

#include "testsession.moc"
