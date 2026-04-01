/*
   SPDX-FileCopyrightText: 2009 Andras Mantia <amantia@kde.org>

   SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
   SPDX-FileContributor: Kevin Ottens <kevin@kdab.com>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kimap/deletejob.h"
#include "kimap/loginjob.h"
#include "kimap/session.h"
#include "kimaptest/fakeserver.h"

#include <QTest>

class DeleteJobTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    void testDelete_data()
    {
        QTest::addColumn<QString>("mailbox");
        QTest::addColumn<QList<QByteArray>>("scenario");

        QList<QByteArray> scenario;
        scenario << FakeServer::preauth() << "C: A000001 DELETE \"foo\""
                 << "S: A000001 OK DELETE completed";
        QTest::newRow("good") << "foo" << scenario;

        scenario.clear();
        scenario << FakeServer::preauth() << "C: A000001 DELETE \"foo-BAD\""
                 << "S: A000001 BAD command unknown or arguments invalid";
        QTest::newRow("bad") << "foo-BAD" << scenario;

        scenario.clear();
        scenario << FakeServer::preauth() << "C: A000001 DELETE \"foo\""
                 << "S: A000001 Name \"foo\" has inferior hierarchical names";
        QTest::newRow("no") << "foo" << scenario;

        scenario.clear();
        scenario << FakeServer::preauth() << "C: A000001 DELETE \"foo-IGNOREDCODE\""
                 << "S: A000001 NO Name \"foo-IGNOREDCODE\" does not exist [IGNOREDCODE]";
        QTest::newRow("ignoredcode") << "foo-IGNOREDCODE" << scenario;

        scenario.clear();
        scenario << FakeServer::preauth() << "C: A000001 DELETE \"foo-NONEXISTENT\""
                 << "S: A000001 NO Name \"foo-NONEXISTENT\" does not exist [NONEXISTENT]";
        QTest::newRow("nonexistent") << "foo-NONEXISTENT" << scenario;

        scenario.clear();
        scenario << FakeServer::preauth() << "C: A000001 DELETE \"foo/bar\""
                 << "S: A000001 OK DELETE completed";
        QTest::newRow("hierarchical") << "foo/bar" << scenario;
    }

    void testDelete()
    {
        QFETCH(QString, mailbox);
        QFETCH(QList<QByteArray>, scenario);

        FakeServer fakeServer;
        fakeServer.setScenario(scenario);
        fakeServer.startAndWait();

        KIMAP::Session session(QStringLiteral("127.0.0.1"), 5989);

        auto job = new KIMAP::DeleteJob(&session);
        job->setMailBox(mailbox);
        bool result = job->exec();
        QEXPECT_FAIL("bad", "Expected failure on BAD response", Continue);
        QEXPECT_FAIL("no", "Expected failure on NO response", Continue);
        QEXPECT_FAIL("ignoredcode", "Expected failure on NO response with ignored response code", Continue);
        QVERIFY(result);
        if (result) {
            QCOMPARE(job->mailBox(), mailbox);
        }

        fakeServer.quit();
    }
    void testDeleteWithUtf8()
    {
        // gr\xc3\xa5 is the UTF-8 encoding of "å"
        QList<QByteArray> scenario;
        scenario << FakeServer::greeting() << "C: A000001 LOGIN \"user\" \"password\""
                 << "S: * CAPABILITY IMAP4rev1 UTF8=ACCEPT"
                 << "S: A000001 OK logged in"
                 << "C: A000002 ENABLE UTF8=ACCEPT"
                 << "S: * ENABLED UTF8=ACCEPT"
                 << "S: A000002 OK"
                 << "C: A000003 DELETE \"INBOX/gr\xc3\xa5\""
                 << "S: A000003 OK DELETE completed";

        FakeServer fakeServer;
        fakeServer.setScenario(scenario);
        fakeServer.startAndWait();

        KIMAP::Session session(QStringLiteral("127.0.0.1"), 5989);

        auto login = new KIMAP::LoginJob(&session);
        login->setUserName(QStringLiteral("user"));
        login->setPassword(QStringLiteral("password"));
        QVERIFY(login->exec());

        auto job = new KIMAP::DeleteJob(&session);
        job->setMailBox(QString::fromUtf8("INBOX/gr\xc3\xa5"));
        QVERIFY(job->exec());

        fakeServer.quit();
    }
};

QTEST_GUILESS_MAIN(DeleteJobTest)

#include "deletejobtest.moc"
