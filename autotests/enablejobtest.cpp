/*
   SPDX-FileCopyrightText: 2020 Daniel Vrátil <dvratil@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <QTest>

#include "kimap/enablejob.h"
#include "kimap/session.h"
#include "kimaptest/fakeserver.h"

#include <QTest>

Q_DECLARE_METATYPE(QList<QList<QByteArray>>)

class EnableJobTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    void testEnable_data()
    {
        QTest::addColumn<QList<QByteArray>>("scenario");
        QTest::addColumn<QStringList>("reqCapabilities");
        QTest::addColumn<QStringList>("supportedCaps");

        QList<QByteArray> scenario;
        scenario << FakeServer::preauth() << "C: A000001 ENABLE CONDSTORE X-GOOD-IDEA"
                 << "S: * ENABLED X-GOOD-IDEA"
                 << "S: A000001 OK Enabled";
        QStringList reqCapabilities = {QStringLiteral("CONDSTORE"), QStringLiteral("X-GOOD-IDEA")};
        QStringList supportedCaps = {QStringLiteral("X-GOOD-IDEA")};
        QTest::newRow("one feature") << scenario << reqCapabilities << supportedCaps;

        scenario.clear();
        scenario << FakeServer::preauth() << "C: A000001 ENABLE FEATURE1 FEATURE2"
                 << "S: * ENABLED FEATURE1 FEATURE2"
                 << "S: A000001 OK Enabled";
        reqCapabilities = QStringList{QStringLiteral("FEATURE1"), QStringLiteral("FEATURE2")};
        supportedCaps = QStringList{QStringLiteral("FEATURE1"), QStringLiteral("FEATURE2")};
        QTest::newRow("both features") << scenario << reqCapabilities << supportedCaps;
    }

    void testEnable()
    {
        QFETCH(QList<QByteArray>, scenario);
        QFETCH(QStringList, reqCapabilities);
        QFETCH(QStringList, supportedCaps);

        FakeServer fakeServer;
        fakeServer.setScenario(scenario);
        fakeServer.startAndWait();

        KIMAP::Session session(QStringLiteral("127.0.0.1"), 5989);

        auto job = new KIMAP::EnableJob(&session);
        job->setCapabilities(reqCapabilities);
        QVERIFY(job->exec());

        QCOMPARE(job->enabledCapabilities(), supportedCaps);

        fakeServer.quit();
    }
};

QTEST_GUILESS_MAIN(EnableJobTest)

#include "enablejobtest.moc"
