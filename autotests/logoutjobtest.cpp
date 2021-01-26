/*
   SPDX-FileCopyrightText: 2009 Andras Mantia <amantia@kde.org>

   SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
   SPDX-FileContributor: Kevin Ottens <kevin@kdab.com>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <QTest>

#include "kimap/loginjob.h"
#include "kimap/logoutjob.h"
#include "kimap/session.h"
#include "kimaptest/fakeserver.h"

#include <QTest>

class LogoutJobTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    void testLogout()
    {
        FakeServer fakeServer;
        fakeServer.setScenario(QList<QByteArray>() << FakeServer::preauth() << "C: A000001 LOGOUT"
                                                   << "S: A000001 OK LOGOUT completed");
        fakeServer.startAndWait();

        auto session = new KIMAP::Session(QStringLiteral("127.0.0.1"), 5989);

        auto logout = new KIMAP::LogoutJob(session);
        QVERIFY(logout->exec());

        fakeServer.quit();
        delete session;
    }

    void testLogoutUntagged()
    {
        FakeServer fakeServer;
        fakeServer.setScenario(QList<QByteArray>() << FakeServer::preauth() << "C: A000001 LOGOUT"
                                                   << "S: * some untagged response"
                                                   << "S: A000001 OK LOGOUT completed");
        fakeServer.startAndWait();

        auto session = new KIMAP::Session(QStringLiteral("127.0.0.1"), 5989);

        auto logout = new KIMAP::LogoutJob(session);
        QVERIFY(logout->exec());

        fakeServer.quit();
        delete session;
    }
};

QTEST_GUILESS_MAIN(LogoutJobTest)

#include "logoutjobtest.moc"
