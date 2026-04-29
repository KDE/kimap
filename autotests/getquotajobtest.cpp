/*
   SPDX-FileCopyrightText: 2026 Arnt Gulbrandsen <arnt@gulbrandsen.priv.no>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kimap/getquotajob.h"
#include "kimap/loginjob.h"
#include "kimap/session.h"
#include "kimaptest/fakeserver.h"

#include <QTest>

class GetQuotaJobTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    void testRoundTrip()
    {
        QList<QByteArray> scenario;
        scenario << FakeServer::preauth() << "C: A000001 GETQUOTA \"user/foo\""
                 << "S: * QUOTA \"user/foo\" (STORAGE 10 512)"
                 << "S: A000001 OK GETQUOTA completed";

        FakeServer fakeServer;
        fakeServer.setScenario(scenario);
        fakeServer.startAndWait();

        KIMAP::Session session(QStringLiteral("127.0.0.1"), 5989);

        auto job = new KIMAP::GetQuotaJob(&session);
        job->setRoot(QStringLiteral("user/foo"));
        QCOMPARE(job->root(), QStringLiteral("user/foo"));
        QVERIFY(job->exec());

        fakeServer.quit();
    }

    // Quota roots can be mailbox names. Without UTF8=ACCEPT they go on the
    // wire in mUTF-7; the API hides that and exposes the decoded name.
    void testEncodesMUtf7()
    {
        // "user/grå" mUTF-7-encoded
        QList<QByteArray> scenario;
        scenario << FakeServer::preauth() << "C: A000001 GETQUOTA \"user/gr&AOU-\""
                 << "S: * QUOTA \"user/gr&AOU-\" (STORAGE 10 512)"
                 << "S: A000001 OK GETQUOTA completed";

        FakeServer fakeServer;
        fakeServer.setScenario(scenario);
        fakeServer.startAndWait();

        KIMAP::Session session(QStringLiteral("127.0.0.1"), 5989);

        auto job = new KIMAP::GetQuotaJob(&session);
        job->setRoot(QString::fromUtf8("user/gr\xc3\xa5"));
        QVERIFY(job->exec());
        QCOMPARE(job->root(), QString::fromUtf8("user/gr\xc3\xa5"));

        fakeServer.quit();
    }

    void testPassesThroughUtf8()
    {
        QList<QByteArray> scenario;
        scenario << FakeServer::greeting() << "C: A000001 LOGIN \"user\" \"password\""
                 << "S: * CAPABILITY IMAP4rev1 UTF8=ACCEPT"
                 << "S: A000001 OK logged in"
                 << "C: A000002 ENABLE UTF8=ACCEPT"
                 << "S: * ENABLED UTF8=ACCEPT"
                 << "S: A000002 OK"
                 << "C: A000003 GETQUOTA \"user/gr\xc3\xa5\""
                 << "S: * QUOTA \"user/gr\xc3\xa5\" (STORAGE 10 512)"
                 << "S: A000003 OK GETQUOTA completed";

        FakeServer fakeServer;
        fakeServer.setScenario(scenario);
        fakeServer.startAndWait();

        KIMAP::Session session(QStringLiteral("127.0.0.1"), 5989);

        auto login = new KIMAP::LoginJob(&session);
        login->setUserName(QStringLiteral("user"));
        login->setPassword(QStringLiteral("password"));
        QVERIFY(login->exec());

        auto job = new KIMAP::GetQuotaJob(&session);
        job->setRoot(QString::fromUtf8("user/gr\xc3\xa5"));
        QVERIFY(job->exec());
        QCOMPARE(job->root(), QString::fromUtf8("user/gr\xc3\xa5"));

        fakeServer.quit();
    }
};

QTEST_GUILESS_MAIN(GetQuotaJobTest)

#include "getquotajobtest.moc"
