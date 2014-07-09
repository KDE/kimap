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

#include <qtest.h>

#include "kimaptest/fakeserver.h"
#include "kimap/session.h"
#include "kimap/loginjob.h"

#include <QTcpSocket>
#include <QtTest>
#include <QDebug>


class TestUiProxy: public KIMAP::SessionUiProxy {
    virtual bool ignoreSslError(const KSslErrorUiData &) {
      return true;
    }
};

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

void shouldUseSsl_data()
{
  QTest::addColumn< QList<QByteArray> >( "scenario" );
  QTest::addColumn< int >( "serverEncryption" );
  QTest::addColumn< int >( "clientEncryption" );

  {
    QList<QByteArray> scenario;
    scenario << FakeServer::greeting()
            << "C: A000001 STARTTLS"
            << "S: A000001 OK"
            << "C: A000002 CAPABILITY"
            << "S: A000002 OK"
            << "C: A000003 LOGIN \"user\" \"password\""
            << "S: A000003 OK";

    //KIMAP ties tlsv1 to starttls
    QTest::newRow( "tlsv1" ) << scenario << static_cast<int>(QSsl::TlsV1) << static_cast<int>(KIMAP::LoginJob::TlsV1);
  }
  {
    QList<QByteArray> scenario;
    scenario << FakeServer::greeting()
            << "C: A000001 CAPABILITY"
            << "S: A000001 OK"
            << "C: A000002 LOGIN \"user\" \"password\""
            << "S: A000002 OK";

    QTest::newRow( "sslv3" ) << scenario << static_cast<int>(QSsl::SslV3) << static_cast<int>(KIMAP::LoginJob::SslV3);
    QTest::newRow( "sslv2" ) << scenario << static_cast<int>(QSsl::SslV2) << static_cast<int>(KIMAP::LoginJob::SslV2);
    //AnySslVersion doesn't mean the server can force a specific version (e.g. openssl always starts with a sslv2 hello)
    QTest::newRow( "any protocol with anyssl version" ) << scenario << static_cast<int>(QSsl::AnyProtocol) << static_cast<int>(KIMAP::LoginJob::AnySslVersion);
    //KIMAP and KTcpSocket use SslV3_1 but really mean tls without starttls
    QTest::newRow( "sslv3_1" ) << scenario << static_cast<int>(QSsl::TlsV1SslV3) << static_cast<int>(KIMAP::LoginJob::SslV3_1);
  }
}

void shouldUseSsl()
{
  QFETCH( QList<QByteArray>, scenario );
  QFETCH( int, serverEncryption );
  QFETCH( int, clientEncryption );

  FakeServer fakeServer;
  fakeServer.setEncrypted( static_cast<QSsl::SslProtocol>(serverEncryption) );
  fakeServer.setScenario( scenario );
  fakeServer.startAndWait();

  KIMAP::Session *session = new KIMAP::Session( "127.0.0.1", 5989 );

  KIMAP::SessionUiProxy::Ptr uiProxy(new TestUiProxy);
  session->setUiProxy(uiProxy);

  KIMAP::LoginJob *login = new KIMAP::LoginJob( session );
  login->setUserName( "user" );
  login->setPassword( "password" );
  login->setEncryptionMode( static_cast<KIMAP::LoginJob::EncryptionMode>(clientEncryption) );
  QVERIFY( login->exec() );

  fakeServer.quit();
  delete session;
}

void shouldFailOnWrongSslSettings_data()
{
  QTest::addColumn< QList<QByteArray> >( "scenario" );
  QTest::addColumn< int >( "serverEncryption" );
  QTest::addColumn< int >( "clientEncryption" );
  QTest::addColumn< int >( "expectedErrorCode" );

  {
    QList<QByteArray> scenario;
    scenario << FakeServer::greeting();

    //For some reason only connecting to tlsv1 results in an ssl handshake error, with the wrong version only the server detects the error and disconnects
//     QTest::newRow( "ssl v3 v2" ) << scenario << static_cast<int>(QSsl::SslV3) << static_cast<int>(KIMAP::LoginJob::SslV2) << static_cast<int>(KJob::UserDefinedError);
    QTest::newRow( "ssl tlsv1 v3" ) << scenario << static_cast<int>(QSsl::TlsV1) << static_cast<int>(KIMAP::LoginJob::SslV3) << static_cast<int>(KJob::UserDefinedError);
  }
}

void shouldFailOnWrongSslSettings()
{
  QFETCH( QList<QByteArray>, scenario );
  QFETCH( int, serverEncryption );
  QFETCH( int, clientEncryption );
  QFETCH( int, expectedErrorCode );

  FakeServer fakeServer;
  fakeServer.setEncrypted( static_cast<QSsl::SslProtocol>(serverEncryption) );
  fakeServer.setScenario( scenario );
  fakeServer.startAndWait();

  KIMAP::Session *session = new KIMAP::Session( "127.0.0.1", 5989 );

  KIMAP::SessionUiProxy::Ptr uiProxy(new TestUiProxy);
  session->setUiProxy(uiProxy);

  KIMAP::LoginJob *login = new KIMAP::LoginJob( session );
  login->setUserName( "user" );
  login->setPassword( "password" );
  login->setEncryptionMode( static_cast<KIMAP::LoginJob::EncryptionMode>(clientEncryption) );
  QVERIFY( !login->exec() );
  QCOMPARE( static_cast<int>(login->error()), expectedErrorCode );

  fakeServer.quit();
  delete session;
}

};

QTEST_GUILESS_MAIN( LoginJobTest )

#include "loginjobtest.moc"
