/*
   SPDX-FileCopyrightText: 2009 Andras Mantia <amantia@kde.org>

   SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
   SPDX-FileContributor: Kevin Ottens <kevin@kdab.com>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kimap/capabilitiesjob.h"
#include "kimap/session.h"
#include "kimaptest/fakeserver.h"

#include <QTest>

class CapabilitiesJobTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    void testCapabilities_data()
    {
        QTest::addColumn<QList<QByteArray>>("scenario");
        QTest::addColumn<QStringList>("capabilities");
        QList<QByteArray> scenario;
        scenario << "S: * PREAUTH"
                 << "C: A000001 CAPABILITY"
                 << "S: * CAPABILITY IMAP4rev1 STARTTLS AUTH=GSSAPI"
                 << "S: A000001 OK CAPABILITY completed";

        QStringList capabilities;
        capabilities << QStringLiteral("IMAP4REV1") << QStringLiteral("STARTTLS") << QStringLiteral("AUTH=GSSAPI");
        QTest::newRow("good") << scenario << capabilities;

        scenario.clear();
        capabilities.clear();
        scenario << "S: * PREAUTH"
                 << "C: A000001 CAPABILITY"
                 << "S: A000001 BAD command unknown or arguments invalid";
        QTest::newRow("bad") << scenario << capabilities;

        scenario.clear();
        capabilities.clear();
        scenario << "S: * PREAUTH"
                 << "C: A000001 CAPABILITY"
                 << "S: * CAPABILITY IMAP4rev1 STARTTLS AUTH=PLAIN"
                 << "S: * some response"
                 << "S: A000001 OK CAPABILITY completed";

        capabilities << QStringLiteral("IMAP4REV1") << QStringLiteral("STARTTLS") << QStringLiteral("AUTH=PLAIN");
        QTest::newRow("extra-untagged") << scenario << capabilities;

        scenario.clear();
        capabilities.clear();
        scenario << "S: * PREAUTH"
                 << "C: A000001 CAPABILITY"
                 << "S: * CAPABILITY IMAP4rev2 ESEARCH SASL-IR LITERAL+"
                 << "S: A000001 OK CAPABILITY completed";

        capabilities << QStringLiteral("IMAP4REV2") << QStringLiteral("ESEARCH") << QStringLiteral("SASL-IR") << QStringLiteral("LITERAL+")
                     << QStringLiteral("NAMESPACE") << QStringLiteral("UNSELECT") << QStringLiteral("UIDPLUS") << QStringLiteral("SEARCHRES")
                     << QStringLiteral("ENABLE") << QStringLiteral("IDLE") << QStringLiteral("LIST-EXTENDED") << QStringLiteral("LIST-STATUS")
                     << QStringLiteral("MOVE");
        QTest::newRow("rev2-folded-in") << scenario << capabilities;

        scenario.clear();
        capabilities.clear();
        scenario << "S: * PREAUTH"
                 << "C: A000001 CAPABILITY"
                 << "S: * CAPABILITY IMAP4rev2 LIST-EXTENDED UIDPLUS"
                 << "S: A000001 OK CAPABILITY completed";

        capabilities << QStringLiteral("IMAP4REV2") << QStringLiteral("LIST-EXTENDED") << QStringLiteral("UIDPLUS") << QStringLiteral("NAMESPACE")
                     << QStringLiteral("UNSELECT") << QStringLiteral("ESEARCH") << QStringLiteral("SEARCHRES") << QStringLiteral("ENABLE")
                     << QStringLiteral("IDLE") << QStringLiteral("SASL-IR") << QStringLiteral("LIST-STATUS") << QStringLiteral("MOVE")
                     << QStringLiteral("LITERAL-");
        QTest::newRow("rev2-folded-in-2") << scenario << capabilities;

        scenario.clear();
        capabilities.clear();
        scenario << "S: * PREAUTH"
                 << "C: A000001 CAPABILITY"
                 << "S: * CAPABILITY IMAP4rev1 STARTTLS QRESYNC"
                 << "S: A000001 OK CAPABILITY completed";

        capabilities << QStringLiteral("IMAP4REV1") << QStringLiteral("STARTTLS") << QStringLiteral("QRESYNC") << QStringLiteral("CONDSTORE");
        QTest::newRow("condstore-qresync") << scenario << capabilities;
    }

    void testCapabilities()
    {
        QFETCH(QList<QByteArray>, scenario);
        QFETCH(QStringList, capabilities);

        FakeServer fakeServer;
        fakeServer.setScenario(scenario);
        fakeServer.startAndWait();
        KIMAP::Session session(QStringLiteral("127.0.0.1"), 5989);

        auto job = new KIMAP::CapabilitiesJob(&session);
        bool result = job->exec();
        QEXPECT_FAIL("bad", "Expected failure on BAD response", Continue);
        QVERIFY(result);
        if (result) {
            QCOMPARE(job->capabilities(), capabilities);
        }
        fakeServer.quit();
    }
};

QTEST_GUILESS_MAIN(CapabilitiesJobTest)

#include "capabilitiesjobtest.moc"
