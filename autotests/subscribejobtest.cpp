/*
   SPDX-FileCopyrightText: 2009 Andras Mantia <amantia@kde.org>

   SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
   SPDX-FileContributor: Kevin Ottens <kevin@kdab.com>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <QTest>

#include "kimap/session.h"
#include "kimap/subscribejob.h"
#include "kimaptest/fakeserver.h"

#include <QTest>

class SubscribeJobTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    void testSubscribe_data()
    {
        QTest::addColumn<QString>("mailbox");
        QTest::addColumn<QList<QByteArray>>("scenario");

        QList<QByteArray> scenario;
        scenario << FakeServer::preauth() << "C: A000001 SUBSCRIBE \"INBOX/foo\""
                 << "S: A000001 OK CREATE completed";
        QTest::newRow("good") << "INBOX/foo" << scenario;

        scenario.clear();
        scenario << FakeServer::preauth() << "C: A000001 SUBSCRIBE \"INBOX-FAIL-BAD\""
                 << "S: A000001 BAD command unknown or arguments invalid";
        QTest::newRow("bad") << "INBOX-FAIL-BAD" << scenario;

        scenario.clear();
        scenario << FakeServer::preauth() << "C: A000001 SUBSCRIBE \"INBOX-FAIL-NO\""
                 << "S: A000001 NO subscribe failure";
        QTest::newRow("no") << "INBOX-FAIL-NO" << scenario;
    }

    void testSubscribe()
    {
        QFETCH(QString, mailbox);
        QFETCH(QList<QByteArray>, scenario);

        FakeServer fakeServer;
        fakeServer.setScenario(scenario);
        fakeServer.startAndWait();

        KIMAP::Session session(QStringLiteral("127.0.0.1"), 5989);

        auto job = new KIMAP::SubscribeJob(&session);
        job->setMailBox(mailbox);
        bool result = job->exec();
        QEXPECT_FAIL("bad", "Expected failure on BAD scenario", Continue);
        QEXPECT_FAIL("no", "Expected failure on NO scenario", Continue);
        QVERIFY(result);
        QCOMPARE(job->mailBox(), mailbox);

        fakeServer.quit();
    }
};

QTEST_GUILESS_MAIN(SubscribeJobTest)

#include "subscribejobtest.moc"
