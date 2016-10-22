/*
   Copyright (C) 2016 Daniel Vrátil <dvratil@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include <qtest.h>

#include "kimaptest/fakeserver.h"
#include "kimap/session.h"
#include "kimap/imapset.h"
#include "../src/movejob.h"

#include <QtTest>

class MoveJobTest: public QObject
{
    Q_OBJECT

private Q_SLOTS:

    void testMove_data()
    {
        QTest::addColumn<bool>("uidBased");
        QTest::addColumn<qint64>("id");
        QTest::addColumn<qint64>("resultUid");
        QTest::addColumn<QString>("mailbox");
        QTest::addColumn< QList<QByteArray> >("scenario");

        QList<QByteArray> scenario;
        scenario << FakeServer::preauth()
                 << "C: A000001 MOVE 3 \"foo\""
                 << "S: * OK [COPYUID 12345 3 7]"
                 << "S: A000001 OK MOVE completed";

        QTest::newRow("not uid based") << false << qint64(3) << qint64(7)
                                       << QStringLiteral("foo") << scenario;

        scenario.clear();
        scenario << FakeServer::preauth()
                 << "C: A000001 UID MOVE 1024 \"bar\""
                 << "S: * OK [COPYUID 12346 4 2048]"
                 << "S: A000001 OK MOVE completed";

        QTest::newRow("uid based") << true << qint64(1024) << qint64(2048)
                                   << QStringLiteral("bar") << scenario;
    }

    void testMove()
    {
        QFETCH(bool, uidBased);
        QFETCH(qint64, id);
        QFETCH(qint64, resultUid);
        QFETCH(QString, mailbox);
        QFETCH(QList<QByteArray>, scenario);

        FakeServer fakeServer;
        fakeServer.setScenario(scenario);
        fakeServer.startAndWait();

        KIMAP::Session session(QStringLiteral("127.0.0.1"), 5989);

        auto job = new KIMAP::MoveJob(&session);
        job->setMailBox(mailbox);
        job->setUidBased(uidBased);
        job->setSequenceSet(KIMAP::ImapSet(id));
        bool result = job->exec();
        QVERIFY(result);
        QCOMPARE(job->resultingUids(), KIMAP::ImapSet(resultUid));

        fakeServer.quit();
    }

};

QTEST_GUILESS_MAIN(MoveJobTest)

#include "movejobtest.moc"

