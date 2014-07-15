/*
   Copyright (C) 2013 Christian Mollekopf <mollekopf@kolabsys.com>

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
#include "kimap/appendjob.h"

#include <QTcpSocket>
#include <QtTest>
#include <QDebug>
#include <KDateTime>

class AppendJobTest: public QObject {
  Q_OBJECT

private Q_SLOTS:

void testAppend_data() {
  QTest::addColumn<QString>( "mailbox" );
  QTest::addColumn<QList<QByteArray> >( "scenario" );
  QTest::addColumn<QList<QByteArray> >( "flags" );
  QTest::addColumn<KDateTime>( "internaldate" );
  QTest::addColumn<QByteArray>( "content" );
  QTest::addColumn<qint64>( "uid" );

  QList<QByteArray> flags;
  flags << QByteArray("\\Seen");
  {
    QList<QByteArray> scenario;
    scenario << FakeServer::preauth()
            << "C: A000001 APPEND \"INBOX\" (\\Seen)  {7}\r\ncontent"
            << "S: A000001 OK APPEND completed. [ APPENDUID 492 2671 ]";
    QTest::newRow( "good" ) << "INBOX" << scenario << flags << KDateTime() << QByteArray("content") << qint64(2671);
  }
  {
    QList<QByteArray> scenario;
    scenario << FakeServer::preauth()
            << "C: A000001 APPEND \"INBOX\" (\\Seen) \"26-Feb-2014 12:38:00 +0000\"  {7}\r\ncontent"
            << "S: A000001 OK APPEND completed. [ APPENDUID 493 2672 ]";
    QTest::newRow( "good, with internalDate set" ) << "INBOX" << scenario << flags << KDateTime::fromString(QLatin1String("2014-02-26T12:38:00Z")) << QByteArray("content") << qint64(2672);
  }

  {
    QList<QByteArray> scenario;
    scenario << FakeServer::preauth()
            << "C: A000001 APPEND \"INBOX\" (\\Seen)  {7}\r\ncontent"
            << "S: BYE"
            << "X" ;
    QTest::newRow( "bad" ) << "INBOX" << scenario << flags << KDateTime() << QByteArray("content") << qint64(0);
  }
  {
    QList<QByteArray> scenario;
    scenario << FakeServer::preauth()
            << "C: A000001 APPEND \"INBOX\" (\\Seen)  {7}\r\ncontent"
            << "S: "
            << "X" ;
    QTest::newRow( "Don't crash on empty response" ) << "INBOX" << scenario << flags << KDateTime() << QByteArray("content") << qint64(0);
  }
}

void testAppend()
{
    QFETCH( QString, mailbox );
    QFETCH( QList<QByteArray>, scenario );
    QFETCH( QList<QByteArray>, flags );
    QFETCH( KDateTime, internaldate );
    QFETCH( QByteArray, content );
    QFETCH( qint64, uid );

    FakeServer fakeServer;
    fakeServer.setScenario( scenario );
    fakeServer.startAndWait();
    KIMAP::Session session( QLatin1String("127.0.0.1"), 5989 );

    KIMAP::AppendJob *job = new KIMAP::AppendJob( &session );
    job->setContent( content );
    job->setFlags( flags );
    job->setInternalDate( internaldate );
    job->setMailBox( mailbox );
    const bool result = job->exec();
    QEXPECT_FAIL( "bad" , "Expected failure on connection abort", Continue );
    QEXPECT_FAIL( "Don't crash on empty response" , "Expected failure on connection abort", Continue );
    QVERIFY( result );
    QCOMPARE( job->uid(), uid );

    fakeServer.quit();
}

};

QTEST_GUILESS_MAIN( AppendJobTest )

#include "appendjobtest.moc"
