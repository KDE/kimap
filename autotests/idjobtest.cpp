/*
   SPDX-FileCopyrightText: 2015 Christian Mollekopf <mollekopf@kolabsys.com>

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

#include "kimaptest/fakeserver.h"
#include "kimap/session.h"
#include "kimap/idjob.h"

#include <QTest>

typedef QMap<QByteArray, QByteArray> ArrayMap;
Q_DECLARE_METATYPE(ArrayMap)

class IdJobTest: public QObject {
  Q_OBJECT

private Q_SLOTS:

void testId_data() {
    QTest::addColumn<QList<QByteArray> >("scenario");
    QTest::addColumn<ArrayMap>("values");
    QList<QByteArray> scenario;
    scenario << "S: * PREAUTH"
             << "C: A000001 ID (\"name\" \"clientid\")"
             << "S: * ID NIL"
             << "S: A000001 OK ID completed";
  
    ArrayMap values;
    values.insert("name", "clientid");
    QTest::newRow("good") << scenario << values;
}

void testId()
{
    QFETCH(QList<QByteArray>, scenario);
    QFETCH(ArrayMap, values);

    FakeServer fakeServer;
    fakeServer.setScenario(scenario);
    fakeServer.startAndWait();
    KIMAP::Session session(QStringLiteral("127.0.0.1"), 5989);

    KIMAP::IdJob *job = new KIMAP::IdJob(&session);
    foreach (const QByteArray &key, values.keys()) {
        job->setField(key, values.value(key));
    }
    bool result = job->exec();
    QVERIFY(result);
    fakeServer.quit();
}

};

QTEST_GUILESS_MAIN(IdJobTest)

#include "idjobtest.moc"
