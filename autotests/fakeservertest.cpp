/*
   SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
   SPDX-FileContributor: Kevin Ottens <kevin@kdab.com>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <QTest>

#include "kimap/listjob.h"
#include "kimap/session.h"
#include "kimaptest/fakeserver.h"

#include <QTest>

class FakeServerTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    void testLoadScenario()
    {
        KIMAP::MailBoxDescriptor descriptor;
        QList<KIMAP::MailBoxDescriptor> listresult;

        descriptor.separator = QLatin1Char('/');
        descriptor.name = QStringLiteral("INBOX");
        listresult << descriptor;
        descriptor.separator = QLatin1Char('/');
        descriptor.name = QString::fromUtf8("INBOX/ä ö ü @ €");
        listresult << descriptor;
        descriptor.separator = QLatin1Char('/');
        descriptor.name = QStringLiteral("INBOX/lost+found");
        listresult << descriptor;
        descriptor.separator = QLatin1Char('/');
        descriptor.name = QStringLiteral("INBOX/lost+found/Calendar Public-20080128");
        listresult << descriptor;

        FakeServer fakeServer;
        fakeServer.addScenarioFromFile(QStringLiteral(TEST_DATA) + QStringLiteral("/fakeserverscenario.log"));
        fakeServer.startAndWait();

        KIMAP::Session session(QStringLiteral("127.0.0.1"), 5989);

        auto job = new KIMAP::ListJob(&session);
        job->setIncludeUnsubscribed(true);
        QVERIFY(job->exec());

        fakeServer.quit();
    }
};

QTEST_GUILESS_MAIN(FakeServerTest)

#include "fakeservertest.moc"
