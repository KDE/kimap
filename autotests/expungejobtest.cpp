/*
   SPDX-FileCopyrightText: 2016 Daniel Vrátil <dvratil@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <QTest>

#include "imapset.h"
#include "kimap/expungejob.h"
#include "kimap/session.h"
#include "kimaptest/fakeserver.h"

#include <QTest>

class ExpungeJobTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    void testDelete_data()
    {
        QTest::addColumn<QList<QByteArray>>("scenario");
        QTest::addColumn<KIMAP::ImapSet>("vanishedSet");
        QTest::addColumn<quint64>("highestModSeq");

        QList<QByteArray> scenario;
        scenario << FakeServer::preauth() << "C: A000001 EXPUNGE"
                 << "S: * 1 EXPUNGE"
                 << "S: * 2 EXPUNGE"
                 << "S: * 3 EXPUNGE"
                 << "S: A000001 OK EXPUNGE completed";
        QTest::newRow("good") << scenario << KIMAP::ImapSet{} << 0ULL;

        scenario.clear();
        scenario << FakeServer::preauth() << "C: A000001 EXPUNGE"
                 << "S: * 1" // missing EXPUNGE word
                 << "S: A000001 OK EXPUNGE completed";
        QTest::newRow("non-standard response") << scenario << KIMAP::ImapSet{} << 0ULL;

        scenario.clear();
        scenario << FakeServer::preauth() << "C: A000001 EXPUNGE"
                 << "S: A000001 BAD command unknown or arguments invalid";
        QTest::newRow("bad") << scenario << KIMAP::ImapSet{} << 0ULL;

        scenario.clear();
        scenario << FakeServer::preauth() << "C: A000001 EXPUNGE"
                 << "S: A000001 NO access denied";
        QTest::newRow("no") << scenario << KIMAP::ImapSet{} << 0ULL;

        scenario.clear();
        scenario << FakeServer::preauth() << "C: A000001 EXPUNGE"
                 << "S: * VANISHED 405,407,410:420"
                 << "S: A000001 OK [HIGHESTMODSEQ 123456789] Expunged.";
        KIMAP::ImapSet vanishedSet;
        vanishedSet.add(QVector<qint64>{405, 407});
        vanishedSet.add(KIMAP::ImapInterval{410, 420});
        QTest::newRow("qresync") << scenario << vanishedSet << 123456789ULL;
    }

    void testDelete()
    {
        QFETCH(QList<QByteArray>, scenario);
        QFETCH(KIMAP::ImapSet, vanishedSet);
        QFETCH(quint64, highestModSeq);

        FakeServer fakeServer;
        fakeServer.setScenario(scenario);
        fakeServer.startAndWait();

        KIMAP::Session session(QStringLiteral("127.0.0.1"), 5989);

        auto job = new KIMAP::ExpungeJob(&session);
        bool result = job->exec();
        QEXPECT_FAIL("bad", "Expected failure on BAD response", Continue);
        QEXPECT_FAIL("no", "Expected failure on NO response", Continue);
        QVERIFY(result);
        if (result) {
            QCOMPARE(job->vanishedMessages(), vanishedSet);
            QCOMPARE(job->newHighestModSeq(), highestModSeq);
        }

        fakeServer.quit();
    }
};

QTEST_GUILESS_MAIN(ExpungeJobTest)

#include "expungejobtest.moc"
