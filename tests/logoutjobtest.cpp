/*
   Copyright (C) 2009 Andras Mantia <amantia@kde.org>

   Copyright (c) 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
   Author: Kevin Ottens <kevin@kdab.com>

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

#include <qtest_kde.h>

#include "kimaptest/fakeserver.h"
#include "kimap/session.h"
#include "kimap/logoutjob.h"
#include "kimap/loginjob.h"

#include <QTcpSocket>
#include <QtTest>
#include <KDebug>

class LogoutJobTest: public QObject {
  Q_OBJECT

private Q_SLOTS:

void testLogout()
{
    FakeServer fakeServer;
    fakeServer.setScenario( QList<QByteArray>()
        << FakeServer::greeting()
        << "C: A000001 LOGIN user password"
        << "S: A000001 OK User logged in"
        << "C: A000002 LOGOUT"
        << "S: A000002 OK LOGOUT completed"
    );
    fakeServer.start();

    KIMAP::Session *session = new KIMAP::Session("127.0.0.1", 5989);

    KIMAP::LoginJob *login = new KIMAP::LoginJob(session);
    login->setUserName("user");
    login->setPassword("password");
    login->exec();

    KIMAP::LogoutJob *logout = new KIMAP::LogoutJob(session);
    QVERIFY(logout->exec());

    fakeServer.quit();
    delete session;
}

void testLogoutUntagged()
{
    FakeServer fakeServer;
    fakeServer.setScenario( QList<QByteArray>()
        << FakeServer::greeting()
        << "C: A000001 LOGIN user password"
        << "S: A000001 OK User logged in"
        << "C: A000002 LOGOUT"
        << "S: * some untagged response"
        << "S: A000002 OK LOGOUT completed"
    );
    fakeServer.start();

    KIMAP::Session *session = new KIMAP::Session("127.0.0.1", 5989);

    KIMAP::LoginJob *login = new KIMAP::LoginJob(session);
    login->setUserName("user");
    login->setPassword("password");
    login->exec();

    KIMAP::LogoutJob *logout = new KIMAP::LogoutJob(session);
    QVERIFY(logout->exec());

    fakeServer.quit();
    delete session;
}

};

QTEST_KDEMAIN_CORE( LogoutJobTest )

#include "logoutjobtest.moc"
