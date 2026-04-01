/*
   SPDX-FileCopyrightText: 2026 Arnt Gulbrandsen <arnt@gulbrandsen.priv.no>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kimap/namespacejob.h"
#include "kimap/listjob.h"
#include "kimap/loginjob.h"
#include "kimap/session.h"
#include "kimaptest/fakeserver.h"

#include <QTest>

class NamespaceJobTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    void testNamespaceWithUtf8()
    {
        // gr\xc3\xa5 is the UTF-8 encoding of "grå".
        // With UTF8=ACCEPT active the server returns raw UTF-8 namespace
        // prefixes; KIMAP must not run them through the modified-UTF-7
        // decoder.
        QList<QByteArray> scenario;
        scenario << FakeServer::greeting() << "C: A000001 LOGIN \"user\" \"password\""
                 << "S: * CAPABILITY IMAP4rev1 UTF8=ACCEPT"
                 << "S: A000001 OK logged in"
                 << "C: A000002 ENABLE UTF8=ACCEPT"
                 << "S: * ENABLED UTF8=ACCEPT"
                 << "S: A000002 OK"
                 << "C: A000003 NAMESPACE"
                 << "S: * NAMESPACE ((\"INBOX/gr\xc3\xa5/\" \"/\")) NIL NIL"
                 << "S: A000003 OK NAMESPACE completed";

        FakeServer fakeServer;
        fakeServer.setScenario(scenario);
        fakeServer.startAndWait();

        KIMAP::Session session(QStringLiteral("127.0.0.1"), 5989);

        auto login = new KIMAP::LoginJob(&session);
        login->setUserName(QStringLiteral("user"));
        login->setPassword(QStringLiteral("password"));
        QVERIFY(login->exec());

        auto job = new KIMAP::NamespaceJob(&session);
        QVERIFY(job->exec());

        const QList<KIMAP::MailBoxDescriptor> personal = job->personalNamespaces();
        QCOMPARE(personal.size(), 1);
        QCOMPARE(personal.first().name, QString::fromUtf8("INBOX/gr\xc3\xa5/"));
        QCOMPARE(personal.first().separator, QLatin1Char('/'));

        QCOMPARE(job->userNamespaces().size(), 0);
        QCOMPARE(job->sharedNamespaces().size(), 0);

        fakeServer.quit();
    }
};

QTEST_GUILESS_MAIN(NamespaceJobTest)

#include "namespacejobtest.moc"
