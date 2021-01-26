/*
   SPDX-FileCopyrightText: 2009 Andras Mantia <amantia@kde.org>

   SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
   SPDX-FileContributor: Kevin Ottens <kevin@kdab.com>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <QTest>

#include "kimap/renamejob.h"
#include "kimap/session.h"
#include "kimaptest/fakeserver.h"

#include <QTest>

class RenameJobTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    void testRename_data()
    {
        QTest::addColumn<QString>("mailbox");
        QTest::addColumn<QString>("newname");
        QTest::addColumn<QList<QByteArray>>("scenario");

        QList<QByteArray> scenario;
        scenario << FakeServer::preauth() << R"(C: A000001 RENAME "INBOX" "oldmail")"
                 << "S: A000001 OK RENAME completed";
        QTest::newRow("good") << "INBOX"
                              << "oldmail" << scenario;

        scenario.clear();
        scenario << FakeServer::preauth() << R"(C: A000001 RENAME "INBOX-FAIL-BAD" "oldmail-bad")"
                 << "S: A000001 BAD command unknown or arguments invalid";
        QTest::newRow("bad") << "INBOX-FAIL-BAD"
                             << "oldmail-bad" << scenario;

        scenario.clear();
        scenario << FakeServer::preauth() << R"(C: A000001 RENAME "INBOX-FAIL-NO" "oldmail-no")"
                 << "S: A000001 NO rename failure";
        QTest::newRow("no") << "INBOX-FAIL-NO"
                            << "oldmail-no" << scenario;
    }

    void testRename()
    {
        QFETCH(QString, mailbox);
        QFETCH(QString, newname);
        QFETCH(QList<QByteArray>, scenario);

        FakeServer fakeServer;
        fakeServer.setScenario(scenario);
        fakeServer.startAndWait();

        KIMAP::Session session(QStringLiteral("127.0.0.1"), 5989);

        auto job = new KIMAP::RenameJob(&session);
        job->setSourceMailBox(mailbox);
        job->setDestinationMailBox(newname);
        bool result = job->exec();
        QEXPECT_FAIL("bad", "Expected failure on BAD response", Continue);
        QEXPECT_FAIL("no", "Expected failure on NO response", Continue);
        QVERIFY(result);
        QCOMPARE(job->sourceMailBox(), mailbox);
        QCOMPARE(job->destinationMailBox(), newname);

        fakeServer.quit();
    }
};

QTEST_GUILESS_MAIN(RenameJobTest)

#include "renamejobtest.moc"
