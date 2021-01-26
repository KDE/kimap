/*
   SPDX-FileCopyrightText: 2009 Andras Mantia <amantia@kde.org>

   SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
   SPDX-FileContributor: Kevin Ottens <kevin@kdab.com>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <QTest>

#include "kimap/createjob.h"
#include "kimap/session.h"
#include "kimaptest/fakeserver.h"

#include <QTest>

class CreateJobTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    void testCreate_data()
    {
        QTest::addColumn<QString>("mailbox");
        QTest::addColumn<QList<QByteArray>>("scenario");

        QList<QByteArray> scenario;
        scenario << FakeServer::preauth() << "C: A000001 CREATE \"INBOX\""
                 << "S: A000001 OK CREATE completed";
        QTest::newRow("good") << "INBOX" << scenario;

        scenario.clear();
        scenario << FakeServer::preauth() << "C: A000001 CREATE \"INBOX-FAIL-BAD\""
                 << "S: A000001 BAD command unknown or arguments invalid";
        QTest::newRow("bad") << "INBOX-FAIL-BAD" << scenario;

        scenario.clear();
        scenario << FakeServer::preauth() << "C: A000001 CREATE \"INBOX-FAIL-NO\""
                 << "S: A000001 NO create failure";
        QTest::newRow("no") << "INBOX-FAIL-NO" << scenario;

        scenario.clear();
        scenario << FakeServer::preauth() << "C: A000001 CREATE \"INBOX-FAIL-IGNOREDCODE\""
                 << "S: A000001 NO create failure [IGNOREDCODE]";
        QTest::newRow("ignoredcode") << "INBOX-FAIL-IGNOREDCODE" << scenario;

        scenario.clear();
        scenario << FakeServer::preauth() << "C: A000001 CREATE \"INBOX-ALREADYEXISTS\""
                 << "S: A000001 NO create failure [ALREADYEXISTS]";
        QTest::newRow("alreadyexists") << "INBOX-ALREADYEXISTS" << scenario;
    }

    void testCreate()
    {
        QFETCH(QString, mailbox);
        QFETCH(QList<QByteArray>, scenario);

        FakeServer fakeServer;
        fakeServer.setScenario(scenario);
        fakeServer.startAndWait();
        KIMAP::Session session(QStringLiteral("127.0.0.1"), 5989);

        auto job = new KIMAP::CreateJob(&session);
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
};

QTEST_GUILESS_MAIN(CreateJobTest)

#include "createjobtest.moc"
