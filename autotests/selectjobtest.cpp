/*
   SPDX-FileCopyrightText: 2009 Andras Mantia <amantia@kde.org>

   SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
   SPDX-FileContributor: Kevin Ottens <kevin@kdab.com>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <QTest>

#include "kimap/loginjob.h"
#include "kimap/selectjob.h"
#include "kimap/session.h"
#include "kimaptest/fakeserver.h"

#include <QSignalSpy>
#include <QTest>

using Messages = QMap<qint64, KIMAP::Message>;

Q_DECLARE_METATYPE(KIMAP::Message)
Q_DECLARE_METATYPE(Messages)

class SelectJobTest : public QObject
{
    Q_OBJECT

public:
    explicit SelectJobTest()
    {
        qRegisterMetaType<KIMAP::ImapSet>();
        qRegisterMetaType<KIMAP::Message>();
        qRegisterMetaType<QMap<qint64, KIMAP::Message>>("QMap<qint64,KIMAP::Message>");
    }

private Q_SLOTS:

    void testSingleSelect_data()
    {
        QTest::addColumn<QList<QByteArray>>("scenario");
        QTest::addColumn<QList<QByteArray>>("flags");
        QTest::addColumn<QList<QByteArray>>("permanentflags");
        QTest::addColumn<int>("messagecount");
        QTest::addColumn<int>("recentcount");
        QTest::addColumn<int>("firstUnseenIndex");
        QTest::addColumn<qint64>("uidValidity");
        QTest::addColumn<qint64>("nextUid");
        QTest::addColumn<quint64>("highestmodseq");
        QTest::addColumn<bool>("condstoreEnabled");
        QTest::addColumn<bool>("readonly");
        QTest::addColumn<qint64>("lastUidvalidity");
        QTest::addColumn<quint64>("lastModseq");
        QTest::addColumn<KIMAP::ImapSet>("vanished");
        QTest::addColumn<Messages>("modified");

        QList<QByteArray> scenario;
        QList<QByteArray> flags;
        QList<QByteArray> permanentflags;
        scenario << FakeServer::preauth() << "C: A000001 SELECT \"INBOX\" (CONDSTORE)"
                 << "S: * 172 EXISTS"
                 << "S: * 1 RECENT"
                 << "S: * OK [UNSEEN 12] Message 12 is first unseen"
                 << "S: * OK [UIDVALIDITY 3857529045] UIDs valid"
                 << "S: * OK [UIDNEXT 4392] Predicted next UID"
                 << "S: * OK [HIGHESTMODSEQ 123456789]"
                 << R"(S: * FLAGS (\Answered \Flagged \Deleted \Seen \Draft))"
                 << R"(S: * OK [PERMANENTFLAGS (\Deleted \Seen \*)] Limited)"
                 << "S: A000001 OK [READ-WRITE] SELECT completed";

        flags << "\\Answered"
              << "\\Flagged"
              << "\\Deleted"
              << "\\Seen"
              << "\\Draft";
        permanentflags << "\\Deleted"
                       << "\\Seen"
                       << "\\*";
        QTest::newRow("good") << scenario << flags << permanentflags << 172 << 1 << 12 << 3857529045LL << 4392LL << 123456789ULL << true << false << -1LL
                              << 0ULL << KIMAP::ImapSet{} << Messages{};

        scenario.clear();
        flags.clear();
        permanentflags.clear();
        scenario << FakeServer::preauth() << "C: A000001 SELECT \"INBOX\""
                 << "S: A000001 BAD command unknown or arguments invalid";
        QTest::newRow("bad") << scenario << flags << permanentflags << 0 << 0 << 0 << 0LL << 0LL << 0ULL << false << false << -1LL << 0ULL << KIMAP::ImapSet{}
                             << Messages{};

        scenario.clear();
        flags.clear();
        permanentflags.clear();
        scenario << FakeServer::preauth() << "C: A000001 SELECT \"INBOX\""
                 << "S: A000001 NO select failure";
        QTest::newRow("no") << scenario << flags << permanentflags << 0 << 0 << 0 << 0LL << 0LL << 0ULL << false << false << -1LL << 0ULL << KIMAP::ImapSet{}
                            << Messages{};

        scenario.clear();
        scenario << FakeServer::preauth() << "C: A000001 SELECT \"INBOX\" (QRESYNC (67890007 90060115194045000))"
                 << "S: * 314 EXISTS"
                 << "S: * 15 RECENT"
                 << "S: * OK [UIDVALIDITY 67890007] UIDVALIDITY"
                 << "S: * OK [UIDNEXT 567] Predicted next UID"
                 << "S: * OK [HIGHESTMODSEQ 90060115205545359]"
                 << "S: * OK [UNSEEN 7] There are some unseen messages in the mailbox"
                 << R"(S: * FLAGS (\Answered \Flagged \Draft \Deleted \Seen))"
                 << R"(S: * OK [PERMANENTFLAGS (\Answered \Flagged \Draft \Deleted \Seen \*)] Permanent flags)"
                 << "S: * VANISHED (EARLIER) 41,43:116,118,120:211,214:540"
                 << "S: * 49 FETCH (UID 117 FLAGS (\\Seen \\Answered) MODSEQ (90060115194045001))"
                 << "S: * 50 FETCH (UID 119 FLAGS (\\Draft $MDNSent) MODSEQ (90060115194045308))"
                 << "S: * 100 FETCH (UID 541 FLAGS (\\Seen $Forwarded) MODSEQ (90060115194045001))"
                 << "S: A000001 OK [READ-WRITE] mailbox selected";
        permanentflags = {"\\Answered", "\\Flagged", "\\Draft", "\\Deleted", "\\Seen", "\\*"};
        flags = {"\\Answered", "\\Flagged", "\\Draft", "\\Deleted", "\\Seen"};
        KIMAP::ImapSet vanished;
        vanished.add(41);
        vanished.add(KIMAP::ImapInterval{43, 116});
        vanished.add(118);
        vanished.add(KIMAP::ImapInterval{120, 211});
        vanished.add(KIMAP::ImapInterval{214, 540});
        Messages modified = {{49, KIMAP::Message{117, 0, {"\\Seen", "\\Answered"}, {{"MODSEQ", QVariant{90060115194045001ULL}}}, {}, {}}},
                             {50, KIMAP::Message{119, 0, {"\\Draft", "$MDNSent"}, {{"MODSEQ", QVariant{90060115194045308ULL}}}, {}, {}}},
                             {100, KIMAP::Message{541, 0, {"\\Seen", "$Forwarded"}, {{"MODSEQ", QVariant{90060115194045001ULL}}}, {}, {}}}};
        QTest::newRow("QResync") << scenario << flags << permanentflags << 314 << 15 << 7 << 67890007LL << 567LL << 90060115205545359ULL << false << false
                                 << 67890007LL << 90060115194045000ULL << vanished << modified;
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
        QFETCH(qint64, lastUidvalidity);
        QFETCH(quint64, lastModseq);
        QFETCH(KIMAP::ImapSet, vanished);
        QFETCH(Messages, modified);

        FakeServer fakeServer;
        fakeServer.setScenario(scenario);
        fakeServer.startAndWait();

        KIMAP::Session session(QStringLiteral("127.0.0.1"), 5989);

        auto job = new KIMAP::SelectJob(&session);
        job->setCondstoreEnabled(condstoreEnabled);
        job->setMailBox(QStringLiteral("INBOX"));
        if (lastUidvalidity > -1 && lastModseq > 0) {
            job->setQResync(lastUidvalidity, lastModseq);
        }

        QSignalSpy vanishedSpy(job, &KIMAP::SelectJob::vanished);
        QVERIFY(vanishedSpy.isValid());

        QSignalSpy modifiedSpy(job, &KIMAP::SelectJob::modified);
        QVERIFY(modifiedSpy.isValid());

        bool result = job->exec();
        QEXPECT_FAIL("bad", "Expected failure on BAD scenario", Continue);
        QEXPECT_FAIL("no", "Expected failure on NO scenario", Continue);
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

            if (!vanished.isEmpty()) {
                QCOMPARE(vanishedSpy.size(), 1);
                QCOMPARE(vanishedSpy.at(0).at(0).value<KIMAP::ImapSet>(), vanished);
            }

            if (!modified.isEmpty()) {
                Messages collectedModified;
                for (const auto &modifiedSpyCatch : modifiedSpy) {
                    const auto msgs = modifiedSpyCatch.at(0).value<Messages>();
                    for (auto it = msgs.begin(); it != msgs.end(); ++it) {
                        collectedModified.insert(it.key(), it.value());
                    }
                }

                QCOMPARE(collectedModified, modified);
            }
        }

        fakeServer.quit();
    }

    void testSeveralSelect()
    {
        FakeServer fakeServer;
        fakeServer.setScenario(QList<QByteArray>() << FakeServer::preauth() << "C: A000001 SELECT \"INBOX\""
                                                   << "S: A000001 OK [READ-WRITE] SELECT completed"
                                                   << "C: A000002 SELECT \"INBOX/Foo\""
                                                   << "S: A000002 OK [READ-WRITE] SELECT completed");
        fakeServer.startAndWait();

        KIMAP::Session session(QStringLiteral("127.0.0.1"), 5989);

        auto job = new KIMAP::SelectJob(&session);
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
            scenario << FakeServer::preauth() << "C: A000001 SELECT \"INBOX\""
                     << "S: A000001 OK [READ-WRITE] SELECT ok";
            QTest::newRow("SELECT rw") << scenario << false << false;
        }

        {
            QList<QByteArray> scenario;
            scenario << FakeServer::preauth() << "C: A000001 SELECT \"INBOX\""
                     << "S: A000001 OK SELECT ok";
            QTest::newRow("SELECT rw (without code)") << scenario << false << false;
        }

        {
            QList<QByteArray> scenario;
            scenario << FakeServer::preauth() << "C: A000001 SELECT \"INBOX\""
                     << "S: A000001 OK [READ-ONLY] SELECT ok";
            QTest::newRow("SELECT ro") << scenario << false << true;
        }

        {
            QList<QByteArray> scenario;
            scenario << FakeServer::preauth() << "C: A000001 EXAMINE \"INBOX\""
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
