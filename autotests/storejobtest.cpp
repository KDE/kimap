/*
   SPDX-FileCopyrightText: 2009 Kevin Ottens <ervin@kde.org>

   SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
   SPDX-FileContributor: Kevin Ottens <kevin@kdab.com>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <QTest>

#include "kimap/session.h"
#include "kimap/storejob.h"
#include "kimaptest/fakeserver.h"

#include <QTest>

class StoreJobTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    void testStore_data()
    {
        QTest::addColumn<bool>("uidBased");
        QTest::addColumn<qint64>("id");
        QTest::addColumn<qint64>("uid");
        QTest::addColumn<QList<QByteArray>>("flags");
        QTest::addColumn<QList<QByteArray>>("scenario");

        QList<QByteArray> scenario;
        scenario << FakeServer::preauth() << "C: A000001 STORE 3 FLAGS (\\Seen \\Foo)"
                 << "S: * 3 FETCH (FLAGS (\\Seen \\Foo) UID 1096)"
                 << "S: A000001 OK STORE completed";

        QTest::newRow("not uid based") << false << qint64(3) << qint64(1096)
                                       << (QList<QByteArray>() << "\\Seen"
                                                               << "\\Foo")
                                       << scenario;

        scenario.clear();
        scenario << FakeServer::preauth() << "C: A000001 UID STORE 1096 FLAGS (\\Seen \\Foo)"
                 << "S: * 3 FETCH (FLAGS (\\Seen \\Foo) UID 1096)"
                 << "S: A000001 OK STORE completed";

        QTest::newRow("uid based") << true << qint64(3) << qint64(1096)
                                   << (QList<QByteArray>() << "\\Seen"
                                                           << "\\Foo")
                                   << scenario;
    }

    void testStore()
    {
        QFETCH(bool, uidBased);
        QFETCH(qint64, id);
        QFETCH(qint64, uid);
        QFETCH(QList<QByteArray>, flags);
        QFETCH(QList<QByteArray>, scenario);

        FakeServer fakeServer;
        fakeServer.setScenario(scenario);
        fakeServer.startAndWait();

        KIMAP::Session session(QStringLiteral("127.0.0.1"), 5989);

        auto job = new KIMAP::StoreJob(&session);
        job->setUidBased(uidBased);
        job->setSequenceSet(KIMAP::ImapSet(uidBased ? uid : id));
        job->setFlags(flags);
        job->setMode(KIMAP::StoreJob::SetFlags);
        bool result = job->exec();
        QVERIFY(result);
        if (uidBased) {
            QVERIFY(job->resultingFlags().contains(uid));
        } else {
            QVERIFY(job->resultingFlags().contains(id));
        }

        fakeServer.quit();
    }
};

QTEST_GUILESS_MAIN(StoreJobTest)

#include "storejobtest.moc"
