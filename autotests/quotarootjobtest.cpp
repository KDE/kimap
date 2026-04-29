/*
   SPDX-FileCopyrightText: 2011 Andras Mantia <amantia@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kimap/getquotarootjob.h"
#include "kimap/loginjob.h"
#include "kimap/session.h"
#include "kimaptest/fakeserver.h"

#include <QTest>

Q_DECLARE_METATYPE(QList<qint64>)

class QuotaRootJobTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    void testGetQuotaRoot_data()
    {
        QTest::addColumn<QString>("mailbox");
        QTest::addColumn<QStringList>("roots");
        QTest::addColumn<QList<QByteArray>>("resources");
        QTest::addColumn<QList<qint64>>("usages");
        QTest::addColumn<QList<qint64>>("limits");
        QTest::addColumn<QList<QByteArray>>("scenario");

        QStringList roots;
        QList<QByteArray> resources;
        QList<qint64> usages;
        QList<qint64> limits;
        QList<QByteArray> scenario;
        roots << QString();
        resources << "STORAGE";
        limits << 512;
        usages << 10;
        scenario << FakeServer::preauth() << "C: A000001 GETQUOTAROOT \"INBOX\""
                 << "S: * QUOTAROOT INBOX \"\" "
                 << "S: * QUOTA \"\" (STORAGE 10 512)"
                 << "S: A000001 OK GETQUOTA completed";
        QTest::newRow("one root, one resource") << "INBOX" << roots << resources << usages << limits << scenario;

        roots.clear();
        resources.clear();
        usages.clear();
        limits.clear();
        scenario.clear();
        roots << QString();
        resources << "STORAGE"
                  << "MESSAGE";
        usages << 10 << 8221;
        limits << 512 << 20000;
        scenario << FakeServer::preauth() << "C: A000001 GETQUOTAROOT \"INBOX\""
                 << "S: * QUOTAROOT INBOX \"\" "
                 << "S: * QUOTA \"\" (STORAGE 10 512)"
                 << "S: * QUOTA \"\" ( MESSAGE 8221 20000 ) "
                 << "S: A000001 OK GETQUOTA completed";
        QTest::newRow("one root, multiple resource") << "INBOX" << roots << resources << usages << limits << scenario;

        roots.clear();
        resources.clear();
        usages.clear();
        limits.clear();
        scenario.clear();
        roots << QStringLiteral("root1") << QStringLiteral("root2");
        resources << "STORAGE"
                  << "MESSAGE";
        usages << 10 << 8221 << 30 << 100;
        limits << 512 << 20000 << 5124 << 120000;
        scenario << FakeServer::preauth() << "C: A000001 GETQUOTAROOT \"INBOX\""
                 << "S: * QUOTAROOT INBOX \"root1\" root2 "
                 << "S: * QUOTA \"root1\" (STORAGE 10 512)"
                 << "S: * QUOTA \"root1\" ( MESSAGE 8221 20000 ) "
                 << "S: * QUOTA \"root2\" ( MESSAGE 100 120000 ) "
                 << "S: * QUOTA \"root2\" (STORAGE 30 5124)"
                 << "S: A000001 OK GETQUOTA completed";
        QTest::newRow("multiple roots, multiple resource") << "INBOX" << roots << resources << usages << limits << scenario;

        roots.clear();
        resources.clear();
        usages.clear();
        limits.clear();
        scenario.clear();
        roots << QString();
        resources << "STORAGE"
                  << "MESSAGE";
        usages << 10 << 8221;
        limits << 512 << 20000;
        scenario << FakeServer::preauth() << "C: A000001 GETQUOTAROOT \"INBOX\""
                 << "S: * QUOTAROOT INBOX"
                 << "S: * QUOTA (STORAGE 10 512)"
                 << "S: * QUOTA ( MESSAGE 8221 20000 ) "
                 << "S: A000001 OK GETQUOTA completed";
        QTest::newRow("no rootname, multiple resource") << "INBOX" << roots << resources << usages << limits << scenario;

        roots.clear();
        resources.clear();
        usages.clear();
        limits.clear();
        scenario.clear();
        roots << QString();
        resources << "STORAGE";
        limits << 512;
        usages << 10;
        scenario << FakeServer::preauth() << "C: A000001 GETQUOTAROOT \"INBOX\""
                 << "S: * QUOTAROOT INBOX \"\" "
                 << "S: * QUOTA (STORAGE 10 512)"
                 << "S: A000001 OK GETQUOTA completed";
        QTest::newRow("no rootname in QUOTA, one resource") << "INBOX" << roots << resources << usages << limits << scenario;

        roots.clear();
        resources.clear();
        usages.clear();
        limits.clear();
        scenario.clear();
        roots << QString();
        resources << "STORAGE";
        limits << 512;
        usages << 10;
        scenario << FakeServer::preauth() << "C: A000001 GETQUOTAROOT \"INBOX\""
                 << "S: * QUOTAROOT INBOX \"root1\" "
                 << "S: * QUOTA \"root2\" (STORAGE 10 512)"
                 << "S: A000001 OK GETQUOTA completed";
        QTest::newRow("rootname mismatch") << "INBOX" << roots << resources << usages << limits << scenario;
    }

    void testGetQuotaRoot()
    {
        QFETCH(QString, mailbox);
        QFETCH(QStringList, roots);
        QFETCH(QList<QByteArray>, resources);
        QFETCH(QList<qint64>, usages);
        QFETCH(QList<qint64>, limits);
        QFETCH(QList<QByteArray>, scenario);

        FakeServer fakeServer;
        fakeServer.setScenario(scenario);
        fakeServer.startAndWait();

        KIMAP::Session session(QStringLiteral("127.0.0.1"), 5989);

        auto job = new KIMAP::GetQuotaRootJob(&session);
        job->setMailBox(mailbox);
        bool result = job->exec();
        QEXPECT_FAIL("bad", "Expected failure on BAD response", Continue);
        QEXPECT_FAIL("no", "Expected failure on NO response", Continue);
        QVERIFY(result);
        QEXPECT_FAIL("rootname mismatch", "Expected failure on rootname mismatch in QUOTAROOT and QUOTA response", Abort);
        QCOMPARE(job->roots(), roots);
        for (int rootIdx = 0; rootIdx < roots.size(); rootIdx++) {
            const QString &root = roots[rootIdx];
            for (int i = 0; i < resources.size(); i++) {
                int idx = i + rootIdx * roots.size();
                QByteArray resource = resources[i];
                QCOMPARE(job->limit(root, resource), limits[idx]);
                QCOMPARE(job->usage(root, resource), usages[idx]);
            }
        }

        fakeServer.quit();
    }

    // The wire format for non-ASCII mailbox names is mUTF-7 (RFC 3501) when
    // UTF8=ACCEPT is not enabled and raw UTF-8 (RFC 9755) when it is. roots()
    // hides that distinction and returns decoded Unicode in both cases.
    void testGetQuotaRootDecodesMUtf7()
    {
        // "INBOX/grå" mUTF-7-encoded
        QList<QByteArray> scenario;
        scenario << FakeServer::preauth() << "C: A000001 GETQUOTAROOT \"INBOX/gr&AOU-\""
                 << "S: * QUOTAROOT INBOX/gr&AOU- INBOX/gr&AOU- "
                 << "S: * QUOTA INBOX/gr&AOU- (STORAGE 10 512)"
                 << "S: A000001 OK GETQUOTA completed";

        FakeServer fakeServer;
        fakeServer.setScenario(scenario);
        fakeServer.startAndWait();

        KIMAP::Session session(QStringLiteral("127.0.0.1"), 5989);

        auto job = new KIMAP::GetQuotaRootJob(&session);
        job->setMailBox(QString::fromUtf8("INBOX/gr\xc3\xa5"));
        QVERIFY(job->exec());
        const QString decoded = QString::fromUtf8("INBOX/gr\xc3\xa5");
        QCOMPARE(job->roots(), QStringList{decoded});
        QCOMPARE(job->limit(decoded, "STORAGE"), 512);
        QCOMPARE(job->usage(decoded, "STORAGE"), 10);

        fakeServer.quit();
    }

    void testGetQuotaRootPassesThroughUtf8()
    {
        // Same mailbox, but UTF8=ACCEPT enabled — bytes on the wire are raw UTF-8.
        QList<QByteArray> scenario;
        scenario << FakeServer::greeting() << "C: A000001 LOGIN \"user\" \"password\""
                 << "S: * CAPABILITY IMAP4rev1 UTF8=ACCEPT"
                 << "S: A000001 OK logged in"
                 << "C: A000002 ENABLE UTF8=ACCEPT"
                 << "S: * ENABLED UTF8=ACCEPT"
                 << "S: A000002 OK"
                 << "C: A000003 GETQUOTAROOT \"INBOX/gr\xc3\xa5\""
                 << "S: * QUOTAROOT INBOX/gr\xc3\xa5 INBOX/gr\xc3\xa5 "
                 << "S: * QUOTA INBOX/gr\xc3\xa5 (STORAGE 10 512)"
                 << "S: A000003 OK GETQUOTA completed";

        FakeServer fakeServer;
        fakeServer.setScenario(scenario);
        fakeServer.startAndWait();

        KIMAP::Session session(QStringLiteral("127.0.0.1"), 5989);

        auto login = new KIMAP::LoginJob(&session);
        login->setUserName(QStringLiteral("user"));
        login->setPassword(QStringLiteral("password"));
        QVERIFY(login->exec());

        auto job = new KIMAP::GetQuotaRootJob(&session);
        job->setMailBox(QString::fromUtf8("INBOX/gr\xc3\xa5"));
        QVERIFY(job->exec());
        const QString decoded = QString::fromUtf8("INBOX/gr\xc3\xa5");
        QCOMPARE(job->roots(), QStringList{decoded});
        QCOMPARE(job->limit(decoded, "STORAGE"), 512);
        QCOMPARE(job->usage(decoded, "STORAGE"), 10);

        fakeServer.quit();
    }
};

QTEST_GUILESS_MAIN(QuotaRootJobTest)

#include "quotarootjobtest.moc"
