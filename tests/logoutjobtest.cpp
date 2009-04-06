/*
   Copyright (C) 2009 Andras Mantia <amantia@kde.org>

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

#include "fakeserver.h"
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
    fakeServer.start();
    KIMAP::Session *session = new KIMAP::Session("127.0.0.1", 5989);
    QStringList list;
    list << "A000001 OK User logged in" << "A000002 OK LOGOUT completed" ;
    fakeServer.setResponse( list );

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
    fakeServer.start();
    KIMAP::Session *session = new KIMAP::Session("127.0.0.1", 5989);
    QStringList list;
    list << "A000001 OK User logged in" << "* some untagged response " << "A000002 OK LOGOUT completed" ;
    fakeServer.setResponse( list );

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

QTEST_KDEMAIN( LogoutJobTest, NoGUI )

#include "logoutjobtest.moc"
