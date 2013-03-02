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
#include "kimap/loginjob.h"

#include <QTcpSocket>
#include <QtTest>
#include <KDebug>

class LoginJobTest: public QObject {
  Q_OBJECT

private Q_SLOTS:

void shouldHandleLogin_data()
{
  QTest::addColumn<QString>( "user" );
  QTest::addColumn<QString>( "password" );
  QTest::addColumn< QList<QByteArray> >( "scenario" );

  QList<QByteArray> scenario;
  scenario << FakeServer::greeting()
           << "C: A000001 LOGIN \"user\" \"password\""
           << "S: A000001 OK User logged in";

  QTest::newRow( "success" ) << "user" << "password" << scenario;

  scenario.clear();
  scenario << FakeServer::greeting()
           << "C: A000001 LOGIN \"user_bad\" \"password\""
           << "S: A000001 NO Login failed: authentication failure";

  QTest::newRow( "wrong login" ) << "user_bad" << "password" << scenario;

  scenario.clear();
  scenario << FakeServer::greeting()
           << "C: A000001 LOGIN \"user\" \"aa\\\"bb\\\\cc[dd ee\""
           << "S: A000001 OK User logged in";

  QTest::newRow( "special chars" ) << "user" << "aa\"bb\\cc[dd ee" << scenario;

  scenario.clear();
  scenario << FakeServer::preauth();

  QTest::newRow( "already authenticated" ) << "user" << "password" << scenario;
}

void shouldHandleLogin()
{
  QFETCH( QString, user );
  QFETCH( QString, password );
  QFETCH( QList<QByteArray>, scenario );

  FakeServer fakeServer;
  fakeServer.setScenario( scenario );
  fakeServer.startAndWait();

  KIMAP::Session *session = new KIMAP::Session( "127.0.0.1", 5989 );

  KIMAP::LoginJob *login = new KIMAP::LoginJob( session );
  login->setUserName( user );
  login->setPassword( password );
  bool result = login->exec();

  QEXPECT_FAIL( "wrong login","Login with bad user name", Continue );
  QEXPECT_FAIL( "already authenticated","Trying to log on an already authenticated session", Continue );
  QVERIFY( result );

  fakeServer.quit();
  delete session;
}

void shouldHandleProxyLogin_data()
{
  QTest::addColumn<QString>( "user" );
  QTest::addColumn<QString>( "proxy" );
  QTest::addColumn<QString>( "password" );
  QTest::addColumn< QList<QByteArray> >( "scenario" );

  QList<QByteArray> scenario;
  scenario << FakeServer::greeting()
           << "C: A000001 AUTHENTICATE PLAIN"
           << "S: A000001 OK (success)"
           << "C: A000001 LOGIN \"proxy\" \"user\" \"password\""
           << "S: A000001 OK User logged in";

  QTest::newRow( "success" ) << "user" << "proxy" << "password" << scenario;
}

void shouldHandleProxyLogin()
{
  QFETCH( QString, user );
  QFETCH( QString, proxy );
  QFETCH( QString, password );
  QFETCH( QList<QByteArray>, scenario );

  FakeServer fakeServer;
  fakeServer.setScenario( scenario );
  fakeServer.startAndWait();

  KIMAP::Session *session = new KIMAP::Session( "127.0.0.1", 5989 );

  KIMAP::LoginJob *login = new KIMAP::LoginJob( session );
  login->setAuthenticationMode( KIMAP::LoginJob::Plain );
  login->setUserName( user );
  login->setAuthorizationName( proxy );
  login->setPassword( password );
  bool result = login->exec();

  QVERIFY( result );

  fakeServer.quit();
  delete session;
}

void shouldSaveServerGreeting_data()
{
  QTest::addColumn<QString>( "greeting" );
  QTest::addColumn< QList<QByteArray> >( "scenario" );

  QList<QByteArray> scenario;
  scenario << FakeServer::greeting()
           << "C: A000001 LOGIN \"user\" \"password\""
           << "S: A000001 OK Welcome John Smith";

  QTest::newRow( "greeting" ) << "Welcome John Smith" << scenario;

  scenario.clear();
  scenario << FakeServer::greeting()
           << "C: A000001 LOGIN \"user\" \"password\""
           << "S: A000001 OK Welcome John Smith (last login: Feb 21, 2010)";

  QTest::newRow( "greeting with parenthesis" ) << "Welcome John Smith (last login: Feb 21, 2010)" << scenario;

  scenario.clear();
  scenario << FakeServer::greeting()
           << "C: A000001 LOGIN \"user\" \"password\""
           << "S: A000001 OK";

  QTest::newRow( "no greeting" ) << "" << scenario;

  scenario.clear();
  scenario << FakeServer::greeting()
           << "C: A000001 LOGIN \"user\" \"password\""
           << "S: A000001 NO Login failed: authentication failure";

  QTest::newRow( "login failed" ) << "" << scenario;
}

void shouldSaveServerGreeting()
{
  QFETCH( QString, greeting );
  QFETCH( QList<QByteArray>, scenario );

  FakeServer fakeServer;
  fakeServer.setScenario( scenario );
  fakeServer.startAndWait();

  KIMAP::Session *session = new KIMAP::Session( "127.0.0.1", 5989 );

  KIMAP::LoginJob *login = new KIMAP::LoginJob( session );
  login->setUserName( "user" );
  login->setPassword( "password" );
  login->exec();

  QCOMPARE( login->serverGreeting(), greeting );

  fakeServer.quit();
  delete session;
}
};

QTEST_KDEMAIN_CORE( LoginJobTest )

#include "loginjobtest.moc"
