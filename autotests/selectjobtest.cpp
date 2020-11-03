/*
   SPDX-FileCopyrightText: 2009 Andras Mantia <amantia@kde.org>

   SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
   SPDX-FileContributor: Kevin Ottens <kevin@kdab.com>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <QTest>

#include "kimaptest/fakeserver.h"
#include "kimap/loginjob.h"
#include "kimap/session.h"
#include "kimap/selectjob.h"

#include <QTest>

class SelectJobTest: public QObject
{
    Q_OBJECT

private Q_SLOTS:

    void testSingleSelect_data()
    {
        QTest::addColumn<QList<QByteArray> >("scenario");
        QTest::addColumn<QList<QByteArray> >("flags");
        QTest::addColumn<QList<QByteArray> >("permanentflags");
        QTest::addColumn<int>("messagecount");
        QTest::addColumn<int>("recentcount");
        QTest::addColumn<int>("firstUnseenIndex");
        QTest::addColumn<qint64>("uidValidity");
        QTest::addColumn<qint64>("nextUid");
        QTest::addColumn<quint64>("highestmodseq");
        QTest::addColumn<bool>("condstoreEnabled");
        QTest::addColumn<bool>("readonly");

        QList<QByteArray> scenario;
        QList<QByteArray> flags;
        QList<QByteArray> permanentflags;
        scenario << FakeServer::preauth()
                 << "C: A000001 SELECT \"INBOX\" (CONDSTORE)"
                 << "S: * 172 EXISTS"
                 << "S: * 1 RECENT"
                 << "S: * OK [UNSEEN 12] Message 12 is first unseen"
                 << "S: * OK [UIDVALIDITY 3857529045] UIDs valid"
                 << "S: * OK [UIDNEXT 4392] Predicted next UID"
                 << "S: * OK [HIGHESTMODSEQ 123456789]"
                 << R"(S: * FLAGS (\Answered \Flagged \Deleted \Seen \Draft))"
                 << R"(S: * OK [PERMANENTFLAGS (\Deleted \Seen \*)] Limited)"
                 << "S: A000001 OK [READ-WRITE] SELECT completed";

        flags << "\\Answered" << "\\Flagged" << "\\Deleted" << "\\Seen" << "\\Draft";
        permanentflags << "\\Deleted" << "\\Seen" << "\\*";
        QTest::newRow("good") << scenario << flags << permanentflags << 172 << 1 << 12 << (qint64)3857529045 << (qint64)4392 << (quint64)123456789 << true << false;

        scenario.clear();
        flags.clear();
        permanentflags.clear();
        scenario << FakeServer::preauth()
                 << "C: A000001 SELECT \"INBOX\""
                 << "S: A000001 BAD command unknown or arguments invalid";
        QTest::newRow("bad") << scenario << flags << permanentflags << 0 << 0 << 0 << (qint64)0 << (qint64)0 << (quint64)0 << false << false;

        scenario.clear();
        flags.clear();
        permanentflags.clear();
        scenario << FakeServer::preauth()
                 << "C: A000001 SELECT \"INBOX\""
                 << "S: A000001 NO select failure";
        QTest::newRow("no") << scenario << flags << permanentflags << 0 << 0 << 0 << (qint64)0 << (qint64)0 << (quint64)0 << false << false;
    }

    void testSingleSelect()
    {
        QFETCH(QList<QByteArray>, scenario);
        QFETCH(QList<QByteArray>, flags);
        QFETCH(QList<QByteArray>, permanentflags);
        QFETCH(int, messagecount);
        QFETCH(int, recentcount);
        QFETCH(int, firstUnseenIndex);
        QFETCH(qint64, uidValidity);
        QFETCH(qint64, nextUid);
        QFETCH(quint64, highestmodseq);
        QFETCH(bool, condstoreEnabled);

        FakeServer fakeServer;
        fakeServer.setScenario(scenario);
        fakeServer.startAndWait();

        KIMAP::Session session(QStringLiteral("127.0.0.1"), 5989);

        auto *job = new KIMAP::SelectJob(&session);
        job->setCondstoreEnabled(condstoreEnabled);
        job->setMailBox(QStringLiteral("INBOX"));
        bool result = job->exec();
        QEXPECT_FAIL("bad" , "Expected failure on BAD scenario", Continue);
        QEXPECT_FAIL("no" , "Expected failure on NO scenario", Continue);
        QVERIFY(result);
        if (result) {
            QCOMPARE(job->flags(), flags);
            QCOMPARE(job->permanentFlags(), permanentflags);
            QCOMPARE(job->messageCount(), messagecount);
            QCOMPARE(job->recentCount(), recentcount);
            QCOMPARE(job->firstUnseenIndex(), firstUnseenIndex);
            QCOMPARE(job->uidValidity(), uidValidity);
            QCOMPARE(job->nextUid(), nextUid);
            QCOMPARE(job->highestModSequence(), highestmodseq);
        }

        fakeServer.quit();
    }

    void testSeveralSelect()
    {
        FakeServer fakeServer;
        fakeServer.setScenario(QList<QByteArray>()
                               << FakeServer::preauth()
                               << "C: A000001 SELECT \"INBOX\""
                               << "S: A000001 OK [READ-WRITE] SELECT completed"
                               << "C: A000002 SELECT \"INBOX/Foo\""
                               << "S: A000002 OK [READ-WRITE] SELECT completed"
                              );
        fakeServer.startAndWait();

        KIMAP::Session session(QStringLiteral("127.0.0.1"), 5989);

        auto *job = new KIMAP::SelectJob(&session);
        job->setMailBox(QStringLiteral("INBOX"));
        QVERIFY(job->exec());

        job = new KIMAP::SelectJob(&session);
        job->setMailBox(QStringLiteral("INBOX/Foo"));
        QVERIFY(job->exec());
    }

    void testReadOnlySelect_data()
    {
        QTest::addColumn<QList<QByteArray>>("scenario");
        QTest::addColumn<bool>("examine");
        QTest::addColumn<bool>("isReadOnly");

        {
            QList<QByteArray> scenario;
            scenario << FakeServer::preauth()
                     << "C: A000001 SELECT \"INBOX\""
                     << "S: A000001 OK [READ-WRITE] SELECT ok";
            QTest::newRow("SELECT rw") << scenario << false << false;
        }

        {
            QList<QByteArray> scenario;
            scenario << FakeServer::preauth()
                     << "C: A000001 SELECT \"INBOX\""
                     << "S: A000001 OK SELECT ok";
            QTest::newRow("SELECT rw (without code)") << scenario << false << false;
        }

        {
            QList<QByteArray> scenario;
            scenario << FakeServer::preauth()
                     << "C: A000001 SELECT \"INBOX\""
                     << "S: A000001 OK [READ-ONLY] SELECT ok";
            QTest::newRow("SELECT ro") << scenario << false << true;
        }

        {
            QList<QByteArray> scenario;
            scenario << FakeServer::preauth()
                     << "C: A000001 EXAMINE \"INBOX\""
                     << "S: A000001 OK [READ-ONLY] EXAMINE ok";
            QTest::newRow("EXAMINE ro") << scenario << true << true;
        }
    }

    void testReadOnlySelect()
    {
        QFETCH(QList<QByteArray>, scenario);
        QFETCH(bool, examine);
        QFETCH(bool, isReadOnly);

        FakeServer fakeServer;
        fakeServer.setScenario(scenario);
        fakeServer.startAndWait();

        KIMAP::Session session(QStringLiteral("127.0.0.1"), 5989);

        KIMAP::SelectJob select(&session);
        select.setOpenReadOnly(examine);
        select.setMailBox(QStringLiteral("INBOX"));
        QVERIFY(select.exec());

        QCOMPARE(select.isOpenReadOnly(), isReadOnly);
    }
};

QTEST_GUILESS_MAIN(SelectJobTest)

#include "selectjobtest.moc"
