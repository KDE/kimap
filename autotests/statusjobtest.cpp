/*
    SPDX-FileCopyrightText: 2016 Daniel Vrátil <dvratil@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <QTest>

#include "kimap/loginjob.h"
#include "kimap/session.h"
#include "kimap/statusjob.h"
#include "kimaptest/fakeserver.h"

using StatusMap = QList<QPair<QByteArray, qint64>>;
Q_DECLARE_METATYPE(StatusMap)

class StatusJobTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    void testStatus_data()
    {
        QTest::addColumn<QList<QByteArray>>("scenario");
        QTest::addColumn<QList<QByteArray>>("dataItems");
        QTest::addColumn<StatusMap>("results");

        QList<QByteArray> scenario;
        QList<QByteArray> dataItems;
        StatusMap results;
        scenario << FakeServer::preauth() << "C: A000001 STATUS \"INBOX\" (MESSAGES RECENT UIDNEXT UIDVALIDITY UNSEEN)"
                 << "S: * STATUS \"INBOX\" (MESSAGES 294 RECENT 1 UIDNEXT 295 UIDVALIDITY 458587604 UNSEEN 181)"
                 << "S: A000001 OK STATUS Completed";
        dataItems = {"MESSAGES", "RECENT", "UIDNEXT", "UIDVALIDITY", "UNSEEN"};
        results = {{"MESSAGES", 294}, {"RECENT", 1}, {"UIDNEXT", 295}, {"UIDVALIDITY", 458587604}, {"UNSEEN", 181}};
        QTest::newRow("good") << scenario << dataItems << results;

        scenario.clear();
        results.clear();
        scenario << FakeServer::preauth() << "C: A000001 STATUS \"INBOX\" (MESSAGES UIDNEXT HIGHESTMODSEQ)"
                 << "S: * STATUS \"INBOX\" (MESSAGES 294 UIDNEXT 295)"
                 << "S: A000001 OK STATUS Completed";
        dataItems = {"MESSAGES", "UIDNEXT", "HIGHESTMODSEQ"};
        results = {{"MESSAGES", 294}, {"UIDNEXT", 295}};
        QTest::newRow("incomplete") << scenario << dataItems << results;

        scenario.clear();
        results.clear();
        scenario << FakeServer::preauth() << "C: A000001 STATUS \"INBOX\" (HIGHESTMODSEQ)"
                 << "S: * STATUS \"INBOX\" ()"
                 << "S: A000001 OK STATUS Completed";
        dataItems = {"HIGHESTMODSEQ"};
        QTest::newRow("empty response") << scenario << dataItems << results;

        scenario.clear();
        results.clear();
        scenario << FakeServer::preauth() << "C: A000001 STATUS \"INBOX\" (MESSAGES HIGHESTMODSEQ)"
                 << "S: A000001 NO status failure";
        dataItems = {"MESSAGES", "HIGHESTMODSEQ"};
        results.clear();
        QTest::newRow("no") << scenario << dataItems << results;

        scenario.clear();
        results.clear();
        scenario << FakeServer::preauth() << "C: A000001 STATUS \"INBOX\" (UIDNEXT)"
                 << "S: A000001 NO bad command";
        dataItems = {"UIDNEXT"};
        QTest::newRow("bad") << scenario << dataItems << results;
    }

    void testStatus()
    {
        QFETCH(QList<QByteArray>, scenario);
        QFETCH(QList<QByteArray>, dataItems);
        QFETCH(StatusMap, results);

        FakeServer fakeServer;
        fakeServer.setScenario(scenario);
        fakeServer.startAndWait();

        KIMAP::Session session(QStringLiteral("127.0.0.1"), 5989);
        auto job = new KIMAP::StatusJob(&session);
        job->setMailBox(QStringLiteral("INBOX"));
        job->setDataItems(dataItems);
        bool result = job->exec();

        QEXPECT_FAIL("bad", "Expected failure on BAD scenario", Continue);
        QEXPECT_FAIL("no", "Expected failure on NO scenario", Continue);
        QVERIFY(result);

        if (result) {
            const StatusMap status = job->status();
            QCOMPARE(status.count(), results.count());
            for (int i = 0; i < results.count(); ++i) {
                QCOMPARE(results[i].first, status[i].first);
                QCOMPARE(results[i].second, status[i].second);
            }
        }

        fakeServer.quit();
    }
};

QTEST_GUILESS_MAIN(StatusJobTest)

#include "statusjobtest.moc"
