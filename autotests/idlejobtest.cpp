/*
   SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
   SPDX-FileContributor: Kevin Ottens <kevin@kdab.com>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <QTest>

#include "kimap/idlejob.h"
#include "kimap/selectjob.h"
#include "kimap/session.h"
#include "kimaptest/fakeserver.h"

#include <QSignalSpy>
#include <QTest>

Q_DECLARE_METATYPE(QList<int>)
Q_DECLARE_METATYPE(QList<qint64>)
Q_DECLARE_METATYPE(KIMAP::IdleJob *)

class IdleJobTest : public QObject
{
    Q_OBJECT

public:
    explicit IdleJobTest(QObject *parent = nullptr)
        : QObject(parent)
    {
        qRegisterMetaType<KIMAP::IdleJob *>();
    }

private Q_SLOTS:
    void shouldReactToIdle_data()
    {
        QTest::addColumn<QList<QByteArray>>("scenario");
        QTest::addColumn<QString>("expectedMailBox");
        QTest::addColumn<QList<int>>("expectedMessageCounts");
        QTest::addColumn<QList<int>>("expectedRecentCounts");
        QTest::addColumn<QList<qint64>>("expectedFlagsNotifications");

        QList<QByteArray> scenario;
        QString expectedMailBox;
        QList<int> expectedMessageCounts;
        QList<int> expectedRecentCounts;
        QList<qint64> expectedFlagsNotifications;

        scenario.clear();
        scenario << FakeServer::preauth() << "C: A000001 SELECT \"INBOX/Foo\""
                 << "S: A000001 OK SELECT done"
                 << "C: A000002 IDLE"
                 << "S: + OK"
                 << "S: * 0 RECENT"
                 << "S: * 1 EXISTS"
                 << "S: * 1 RECENT"
                 << "S: * 2 EXISTS"
                 << "S: A000002 OK done idling";

        expectedMailBox = QStringLiteral("INBOX/Foo");

        expectedMessageCounts.clear();
        expectedRecentCounts.clear();

        expectedMessageCounts << 1 << 2;
        expectedRecentCounts << 0 << 1;

        QTest::newRow("normal") << scenario << expectedMailBox << expectedMessageCounts << expectedRecentCounts << expectedFlagsNotifications;

        scenario.clear();
        scenario << FakeServer::preauth() << "C: A000001 SELECT \"INBOX/Foo\""
                 << "S: A000001 OK SELECT done"
                 << "C: A000002 IDLE"
                 << "S: + OK"
                 << "S: * 0 RECENT"
                 << "S: * 1 RECENT"
                 << "S: A000002 OK done idling";

        expectedMailBox = QStringLiteral("INBOX/Foo");

        expectedMessageCounts.clear();
        expectedRecentCounts.clear();

        expectedMessageCounts << -1 << -1;
        expectedRecentCounts << 0 << 1;

        QTest::newRow("only RECENT") << scenario << expectedMailBox << expectedMessageCounts << expectedRecentCounts << expectedFlagsNotifications;

        scenario.clear();
        scenario << FakeServer::preauth() << "C: A000001 SELECT \"INBOX/Foo\""
                 << "S: A000001 OK SELECT done"
                 << "C: A000002 IDLE"
                 << "S: + OK"
                 << "S: * 1 EXISTS"
                 << "S: * 2 EXISTS"
                 << "S: A000002 OK done idling";

        expectedMailBox = QStringLiteral("INBOX/Foo");

        expectedMessageCounts.clear();
        expectedRecentCounts.clear();

        expectedMessageCounts << 1 << 2;
        expectedRecentCounts << -1 << -1;

        QTest::newRow("only EXISTS") << scenario << expectedMailBox << expectedMessageCounts << expectedRecentCounts << expectedFlagsNotifications;

        scenario.clear();
        scenario << FakeServer::preauth() << "C: A000001 SELECT \"INBOX/Foo\""
                 << "S: A000001 OK SELECT done"
                 << "C: A000002 IDLE"
                 << "S: + OK"
                 << "S: * 2 EXISTS"
                 << "W: 150"
                 << "S: * 1 RECENT"
                 << "S: A000002 OK done idling";

        expectedMailBox = QStringLiteral("INBOX/Foo");

        expectedMessageCounts.clear();
        expectedRecentCounts.clear();

        expectedMessageCounts << 2;
        expectedRecentCounts << 1;

        QTest::newRow("under 200ms, same notification")
            << scenario << expectedMailBox << expectedMessageCounts << expectedRecentCounts << expectedFlagsNotifications;

        scenario.clear();
        scenario << FakeServer::preauth() << "C: A000001 SELECT \"INBOX/Foo\""
                 << "S: A000001 OK SELECT done"
                 << "C: A000002 IDLE"
                 << "S: + OK"
                 << "S: * 2 EXISTS"
                 << "W: 250"
                 << "S: * 1 RECENT"
                 << "S: A000002 OK done idling";

        expectedMailBox = QStringLiteral("INBOX/Foo");

        expectedMessageCounts.clear();
        expectedRecentCounts.clear();

        expectedMessageCounts << 2 << -1;
        expectedRecentCounts << -1 << 1;

        QTest::newRow("above 200ms, two notifications")
            << scenario << expectedMailBox << expectedMessageCounts << expectedRecentCounts << expectedFlagsNotifications;

        scenario.clear();
        scenario << FakeServer::preauth() << "C: A000001 SELECT \"INBOX/Foo\""
                 << "S: A000001 OK SELECT done"
                 << "C: A000002 IDLE"
                 << "S: + OK"
                 << "S: * 1 FETCH (FLAGS ())"
                 << "W: 200"
                 << "S: * 2 FETCH (FLAGS (\\Seen))"
                 << "S: A000002 OK done idling";

        expectedMailBox = QStringLiteral("INBOX/Foo");

        expectedMessageCounts.clear();
        expectedRecentCounts.clear();
        expectedFlagsNotifications << 1 << 2;

        QTest::newRow("2 flags change notifications") << scenario << expectedMailBox << expectedMessageCounts << expectedRecentCounts
                                                      << expectedFlagsNotifications;
    }

    void shouldReactToIdle()
    {
        QFETCH(QList<QByteArray>, scenario);
        QFETCH(QString, expectedMailBox);
        QFETCH(QList<int>, expectedMessageCounts);
        QFETCH(QList<int>, expectedRecentCounts);
        QFETCH(QList<qint64>, expectedFlagsNotifications);

        QVERIFY(expectedMessageCounts.size() == expectedRecentCounts.size());

        FakeServer fakeServer;
        fakeServer.setScenario(scenario);
        fakeServer.startAndWait();

        KIMAP::Session session(QStringLiteral("127.0.0.1"), 5989);

        auto select = new KIMAP::SelectJob(&session);
        select->setMailBox(expectedMailBox);
        QVERIFY(select->exec());

        auto idle = new KIMAP::IdleJob(&session);

        QSignalSpy statsSpy(idle, &KIMAP::IdleJob::mailBoxStats);
        QSignalSpy flagsSpy(idle, &KIMAP::IdleJob::mailBoxMessageFlagsChanged);

        bool result = idle->exec();

        if (result) {
            QCOMPARE(statsSpy.count(), expectedMessageCounts.size());
            QCOMPARE(flagsSpy.count(), expectedFlagsNotifications.size());

            for (int i = 0; i < statsSpy.count(); i++) {
                const KIMAP::IdleJob *job = statsSpy.at(i).at(0).value<KIMAP::IdleJob *>();
                const QString mailBox = statsSpy.at(i).at(1).toString();
                const int messageCount = statsSpy.at(i).at(2).toInt();
                const int recentCount = statsSpy.at(i).at(3).toInt();

                QCOMPARE(job, idle);
                QCOMPARE(mailBox, expectedMailBox);
                QCOMPARE(messageCount, expectedMessageCounts.at(i));
                QCOMPARE(recentCount, expectedRecentCounts.at(i));
            }

            for (int i = 0; i < flagsSpy.count(); i++) {
                const KIMAP::IdleJob *job = flagsSpy.at(i).at(0).value<KIMAP::IdleJob *>();
                const qint64 uid = flagsSpy.at(i).at(1).toLongLong();

                QCOMPARE(job, idle);
                QCOMPARE(uid, expectedFlagsNotifications.at(i));
            }
        }

        fakeServer.quit();
    }

    void shouldResetTimeout_data()
    {
        QTest::addColumn<QList<QByteArray>>("scenario");
        {
            QList<QByteArray> scenario;
            scenario << FakeServer::preauth() << "C: A000001 SELECT \"INBOX\""
                     << "S: A000001 OK SELECT done"
                     << "C: A000002 IDLE"
                     << "S: + OK"
                     << "S: A000002 OK done idling";

            QTest::newRow("good") << scenario;
        }

        {
            QList<QByteArray> scenario;
            scenario << FakeServer::preauth() << "C: A000001 SELECT \"INBOX\""
                     << "S: A000001 OK SELECT done"
                     << "C: A000002 IDLE"
                     << "S: + OK"
                     << "X";

            QTest::newRow("bad") << scenario;
        }
    }

    void shouldResetTimeout()
    {
        QFETCH(QList<QByteArray>, scenario);

        FakeServer fakeServer;
        fakeServer.setScenario(scenario);
        fakeServer.startAndWait();

        KIMAP::Session session(QStringLiteral("127.0.0.1"), 5989);
        const int originalTimeout = session.timeout();

        auto select = new KIMAP::SelectJob(&session);
        select->setMailBox(QStringLiteral("INBOX"));
        QVERIFY(select->exec());

        auto idle = new KIMAP::IdleJob(&session);
        idle->exec();

        QCOMPARE(session.timeout(), originalTimeout);

        fakeServer.quit();
    }
};

QTEST_GUILESS_MAIN(IdleJobTest)

#include "idlejobtest.moc"
