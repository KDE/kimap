/*
   SPDX-FileCopyrightText: 2009 Kevin Ottens <ervin@kde.org>

   SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
   SPDX-FileContributor: Kevin Ottens <kevin@kdab.com>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kimap/storejob.h"
#include "kimap/session.h"
#include "kimaptest/fakeserver.h"

#include <QTest>

class StoreJobTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    void testStore_data()
    {
        QTest::addColumn<bool>("uidBased");
        QTest::addColumn<QList<qint64>>("ids");
        QTest::addColumn<QList<qint64>>("uids");
        QTest::addColumn<qint64>("lastModSeq");
        QTest::addColumn<QList<QByteArray>>("flags");
        QTest::addColumn<QList<qint64>>("expectedModified");
        QTest::addColumn<QList<QByteArray>>("scenario");

        QList<QByteArray> scenario;
        scenario << FakeServer::preauth() << "C: A000001 STORE 3 FLAGS (\\Seen \\Foo)"
                 << "S: * 3 FETCH (FLAGS (\\Seen \\Foo) UID 1096)"
                 << "S: A000001 OK Conditional Store completed";

        QTest::newRow("not uid based") << false << (QList<qint64>() << qint64(3)) << (QList<qint64>() << qint64(1096)) << qint64(0)
                                       << (QList<QByteArray>() << "\\Seen"
                                                               << "\\Foo")
                                       << QList<qint64>() << scenario;

        scenario.clear();
        scenario << FakeServer::preauth() << "C: A000001 UID STORE 1096 FLAGS (\\Seen \\Foo)"
                 << "S: * 3 FETCH (FLAGS (\\Seen \\Foo) UID 1096)"
                 << "S: A000001 OK Conditional Store completed";

        QTest::newRow("uid based") << true << (QList<qint64>() << qint64(3)) << (QList<qint64>() << qint64(1096)) << qint64(0)
                                   << (QList<QByteArray>() << "\\Seen"
                                                           << "\\Foo")
                                   << QList<qint64>() << scenario;

        scenario.clear();
        scenario << FakeServer::preauth() << "C: A000001 STORE 3 FLAGS (\\Seen \\Foo)"
                 << "S: * 3 FETCH (FLAGS (\\Seen \\Foo) UID 1096)"
                 << "S: A000001 OK Conditional Store completed";

        QTest::newRow("Ensure store don't sent \\Recent flag") << false << (QList<qint64>() << qint64(3)) << (QList<qint64>() << qint64(1096)) << qint64(0)
                                                               << (QList<QByteArray>() << "\\Seen"
                                                                                       << "\\Foo"
                                                                                       << "\\Recent")
                                                               << QList<qint64>() << scenario;

        scenario.clear();
        scenario << FakeServer::preauth() << "C: A000001 STORE 3 (UNCHANGEDSINCE 123456789) FLAGS (\\Seen \\Foo)"
                 << "S: * 3 FETCH (MODSEQ (123456790) FLAGS (\\Seen \\Foo) UID 1096)"
                 << "S: A000001 OK Conditional Store completed";

        QTest::newRow("CONDTSORE not uid based") << false << (QList<qint64>() << qint64(3)) << (QList<qint64>() << qint64(1096)) << qint64(123456789)
                                                 << (QList<QByteArray>() << "\\Seen"
                                                                         << "\\Foo")
                                                 << QList<qint64>() << scenario;

        scenario.clear();
        scenario << FakeServer::preauth() << "C: A000001 UID STORE 1096 (UNCHANGEDSINCE 123456789) FLAGS (\\Seen \\Foo)"
                 << "S: * 3 FETCH (MODSEQ (123456796) FLAGS (\\Seen \\Foo) UID 1096)"
                 << "S: A000001 OK Conditional Store completed";

        QTest::newRow("CONDSTORE uid based") << true << (QList<qint64>() << qint64(3)) << (QList<qint64>() << qint64(1096)) << qint64(123456789)
                                             << (QList<QByteArray>() << "\\Seen"
                                                                     << "\\Foo")
                                             << QList<qint64>() << scenario;

        scenario.clear();
        scenario << FakeServer::preauth() << "C: A000001 STORE 3 (UNCHANGEDSINCE 123456789) FLAGS (\\Seen \\Foo)"
                 << "S: * 3 FETCH (MODSEQ (123456818) FLAGS (\\Seen \\Foo) UID 1096)"
                 << "S: A000001 OK Conditional Store completed";

        QTest::newRow("CONDSTORE Ensure store don't sent \\Recent flag")
            << false << (QList<qint64>() << qint64(3)) << (QList<qint64>() << qint64(1096)) << qint64(123456789)
            << (QList<QByteArray>() << "\\Seen"
                                    << "\\Foo"
                                    << "\\Recent")
            << QList<qint64>() << scenario;

        scenario.clear();
        scenario << FakeServer::preauth() << "C: A000001 STORE 5,7,9 (UNCHANGEDSINCE 123456789) FLAGS (\\Deleted \\Foo)"
                 << "S: * 5 FETCH (MODSEQ (123456818))"
                 << "S: A000001 OK [MODIFIED 7,9] Conditional STORE failed";

        QTest::newRow("CONDSTORE partial fail no FETCH") << false << (QList<qint64>() << qint64(5) << qint64(7) << qint64(9))
                                                         << (QList<qint64>() << qint64(1096) << qint64(1098) << qint64(2000)) << qint64(123456789)
                                                         << (QList<QByteArray>() << "\\Deleted"
                                                                                 << "\\Foo"
                                                                                 << "\\Recent")
                                                         << (QList<qint64>() << qint64(7) << qint64(9)) << scenario;

        scenario.clear();
        scenario << FakeServer::preauth() << "C: A000001 STORE 5,7,9 (UNCHANGEDSINCE 123456789) FLAGS (\\Deleted \\Foo)"
                 << "S: * 7 FETCH (MODSEQ (320162342) FLAGS (\\Seen \\Deleted))"
                 << "S: * 5 FETCH (MODSEQ (123456818))"
                 << "S: * 9 FETCH (MODSEQ (320162349) FLAGS (\\Answered))"
                 << "S: A000001 OK [MODIFIED 7,9] Conditional STORE failed";

        QTest::newRow("CONDSTORE partial fail with FETCH") << false << (QList<qint64>() << qint64(5) << qint64(7) << qint64(9))
                                                           << (QList<qint64>() << qint64(1096) << qint64(1098) << qint64(2000)) << qint64(123456789)
                                                           << (QList<QByteArray>() << "\\Deleted"
                                                                                   << "\\Foo"
                                                                                   << "\\Recent")
                                                           << (QList<qint64>() << qint64(7) << qint64(9)) << scenario;
    }

    void testStore()
    {
        QFETCH(bool, uidBased);
        QFETCH(QList<qint64>, ids);
        QFETCH(QList<qint64>, uids);
        QFETCH(qint64, lastModSeq);
        QFETCH(QList<QByteArray>, flags);
        QFETCH(QList<qint64>, expectedModified);
        QFETCH(QList<QByteArray>, scenario);

        FakeServer fakeServer;
        fakeServer.setScenario(scenario);
        fakeServer.startAndWait();

        KIMAP::Session session(QStringLiteral("127.0.0.1"), 5989);

        auto job = new KIMAP::StoreJob(&session);
        job->setUidBased(uidBased);
        auto sequenceSet = KIMAP::ImapSet();
        sequenceSet.add(uidBased ? uids : ids);
        job->setSequenceSet(sequenceSet);
        job->setLastModSeq(lastModSeq);
        job->setFlags(flags);
        job->setMode(KIMAP::StoreJob::SetFlags);
        bool result = job->exec();
        QVERIFY(result);
        auto modifiedSet = KIMAP::ImapSet();
        modifiedSet.add(expectedModified);
        QVERIFY(job->unchangedMessages() == modifiedSet);
        auto idsToVerify = uidBased ? uids : ids;
        for (const qint64 &id : idsToVerify) {
            if (!expectedModified.contains(id)) {
                QVERIFY(job->resultingFlags().contains(id));
                if (lastModSeq > 0) {
                    QVERIFY(job->resultingModSeqs().contains(id));
                }
            }
        }

        fakeServer.quit();
    }
};

QTEST_GUILESS_MAIN(StoreJobTest)

#include "storejobtest.moc"
