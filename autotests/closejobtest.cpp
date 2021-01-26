/*
   SPDX-FileCopyrightText: 2020  Daniel Vrátil <dvratil@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <QTest>

#include "kimap/closejob.h"
#include "kimap/session.h"
#include "kimaptest/fakeserver.h"

#include <QTest>

class CloseJobTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    void testClose_data()
    {
        QTest::addColumn<QList<QByteArray>>("scenario");
        QTest::addColumn<quint64>("highestModSeq");

        QList<QByteArray> scenario;
        scenario << FakeServer::preauth() << "C: A000001 CLOSE"
                 << "S: A000001 OK Closed";
        QTest::newRow("good") << scenario << 0ULL;

        scenario.clear();
        scenario << FakeServer::preauth() << "C: A000001 CLOSE"
                 << "S: A000001 BAD No mailbox selected";
        QTest::newRow("bad") << scenario << 0ULL;

        scenario.clear();
        scenario << FakeServer::preauth() << "C: A000001 CLOSE"
                 << "S: A000001 OK [HIGHESTMODSEQ 123456789] Closed.";
        QTest::newRow("qresync") << scenario << 123456789ULL;
    }

    void testClose()
    {
        QFETCH(QList<QByteArray>, scenario);
        QFETCH(quint64, highestModSeq);

        FakeServer fakeServer;
        fakeServer.setScenario(scenario);
        fakeServer.startAndWait();

        KIMAP::Session session(QStringLiteral("127.0.0.1"), 5989);

        auto job = new KIMAP::CloseJob(&session);
        bool result = job->exec();
        QEXPECT_FAIL("bad", "Expected failure on BAD response", Continue);
        QVERIFY(result);
        if (result) {
            QCOMPARE(job->newHighestModSeq(), highestModSeq);
        }

        fakeServer.quit();
    }
};

QTEST_GUILESS_MAIN(CloseJobTest)

#include "closejobtest.moc"
