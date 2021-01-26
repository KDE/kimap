/*
   SPDX-FileCopyrightText: 2009 Andras Mantia <amantia@kde.org>

   SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
   SPDX-FileContributor: Kevin Ottens <kevin@kdab.com>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <QTest>

#include "kimap/session.h"
#include "kimap/unsubscribejob.h"
#include "kimaptest/fakeserver.h"

#include <QTest>

class UnsubscribeJobTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    void testUnsubscribe_data()
    {
        QTest::addColumn<QString>("mailbox");
        QTest::addColumn<QList<QByteArray>>("scenario");

        QList<QByteArray> scenario;
        scenario << FakeServer::preauth() << "C: A000001 UNSUBSCRIBE \"#news.comp.mail.mime\""
                 << "S: A000001 OK UNSUBSCRIBE completed";
        QTest::newRow("good") << "#news.comp.mail.mime" << scenario;

        scenario.clear();
        scenario << FakeServer::preauth() << "C: A000001 UNSUBSCRIBE \"INBOX-FAIL-BAD\""
                 << "S: A000001 BAD command unknown or arguments invalid";
        QTest::newRow("bad") << "INBOX-FAIL-BAD" << scenario;

        scenario.clear();
        scenario << FakeServer::preauth() << "C: A000001 UNSUBSCRIBE \"INBOX-FAIL-NO\""
                 << "S: A000001 NO unsubscribe failure";
        QTest::newRow("no") << "INBOX-FAIL-NO" << scenario;
    }

    void testUnsubscribe()
    {
        QFETCH(QString, mailbox);
        QFETCH(QList<QByteArray>, scenario);

        FakeServer fakeServer;
        fakeServer.setScenario(scenario);
        fakeServer.startAndWait();

        KIMAP::Session session(QStringLiteral("127.0.0.1"), 5989);

        auto job = new KIMAP::UnsubscribeJob(&session);
        job->setMailBox(mailbox);
        bool result = job->exec();
        QEXPECT_FAIL("bad", "Expected failure on BAD scenario", Continue);
        QEXPECT_FAIL("no", "Expected failure on NO scenario", Continue);
        QVERIFY(result);
        QCOMPARE(job->mailBox(), mailbox);

        fakeServer.quit();
    }
};

QTEST_GUILESS_MAIN(UnsubscribeJobTest)

#include "unsubscribejobtest.moc"
