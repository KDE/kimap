/*
   SPDX-FileCopyrightText: 2015 Christian Mollekopf <mollekopf@kolabsys.com>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kimap/idjob.h"
#include "kimap/session.h"
#include "kimaptest/fakeserver.h"

#include <QTest>

using ArrayMap = QMap<QByteArray, QByteArray>;
Q_DECLARE_METATYPE(ArrayMap)

class IdJobTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    void testId_data()
    {
        QTest::addColumn<QList<QByteArray>>("scenario");
        QTest::addColumn<ArrayMap>("values");
        QList<QByteArray> scenario;
        scenario << "S: * PREAUTH"
                 << R"(C: A000001 ID ("name" "clientid"))"
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

        auto job = new KIMAP::IdJob(&session);
        const auto keys = values.keys();
        for (const QByteArray &key : keys) {
            job->setField(key, values.value(key));
        }
        bool result = job->exec();
        QVERIFY(result);
        fakeServer.quit();
    }
};

QTEST_GUILESS_MAIN(IdJobTest)

#include "idjobtest.moc"
