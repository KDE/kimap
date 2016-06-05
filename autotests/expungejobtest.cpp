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
#include "kimap/expungejob.h"

#include <QtTest>

class ExpungeJobTest: public QObject
{
    Q_OBJECT

private Q_SLOTS:

    void testDelete_data()
    {
        QTest::addColumn<QList<QByteArray> >("scenario");

        QList<QByteArray> scenario;
        scenario << FakeServer::preauth()
                 << "C: A000001 EXPUNGE"
                 << "S: * 1 EXPUNGE"
                 << "S: * 2 EXPUNGE"
                 << "S: * 3 EXPUNGE"
                 << "S: A000001 OK EXPUNGE completed";
        QTest::newRow("good") << scenario;

        scenario.clear();
        scenario << FakeServer::preauth()
                 << "C: A000001 EXPUNGE"
                 << "S: * 1" // missing EXPUNGE word
                 << "S: A000001 OK EXPUNGE completed";
        QTest::newRow("non-standard response") << scenario;

        scenario.clear();
        scenario << FakeServer::preauth()
                 << "C: A000001 EXPUNGE"
                 << "S: A000001 BAD command unknown or arguments invalid";
        QTest::newRow("bad") << scenario;

        scenario.clear();
        scenario << FakeServer::preauth()
                 << "C: A000001 EXPUNGE"
                 << "S: A000001 NO access deined";
        QTest::newRow("no") << scenario;
    }

    void testDelete()
    {
        QFETCH(QList<QByteArray>, scenario);

        FakeServer fakeServer;
        fakeServer.setScenario(scenario);
        fakeServer.startAndWait();

        KIMAP::Session session(QStringLiteral("127.0.0.1"), 5989);

        KIMAP::ExpungeJob *job = new KIMAP::ExpungeJob(&session);
        bool result = job->exec();
        QEXPECT_FAIL("bad" , "Expected failure on BAD response", Continue);
        QEXPECT_FAIL("no" , "Expected failure on NO response", Continue);
        QVERIFY(result);

        fakeServer.quit();
    }

};

QTEST_GUILESS_MAIN(ExpungeJobTest)

#include "expungejobtest.moc"
