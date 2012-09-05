/*
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
#include "kimap/selectjob.h"
#include "kimap/idlejob.h"

#include <QTcpSocket>
#include <QtTest>
#include <KDebug>

Q_DECLARE_METATYPE( QList<int> )
Q_DECLARE_METATYPE( KIMAP::IdleJob* )

class IdleJobTest: public QObject {
  Q_OBJECT

public:
explicit IdleJobTest( QObject *parent = 0 )
  : QObject( parent )
{
  qRegisterMetaType<KIMAP::IdleJob*>();
}

private Q_SLOTS:
void shouldReactToIdle_data()
{
  QTest::addColumn<QList<QByteArray> >( "scenario" );
  QTest::addColumn<QString>( "expectedMailBox" );
  QTest::addColumn< QList<int> >( "expectedMessageCounts" );
  QTest::addColumn< QList<int> >( "expectedRecentCounts" );

  QList<QByteArray> scenario;
  QString expectedMailBox;
  QList<int> expectedMessageCounts;
  QList<int> expectedRecentCounts;


  scenario.clear();
  scenario << FakeServer::preauth()
           << "C: A000001 SELECT \"INBOX/Foo\""
           << "S: A000001 OK SELECT done"
           << "C: A000002 IDLE"
           << "S: + OK"
           << "S: * 0 RECENT"
           << "S: * 1 EXISTS"
           << "S: * 1 RECENT"
           << "S: * 2 EXISTS"
           << "S: A000002 OK done idling";

  expectedMailBox = "INBOX/Foo";

  expectedMessageCounts.clear();
  expectedRecentCounts.clear();

  expectedMessageCounts << 1 << 2;
  expectedRecentCounts << 0 << 1;

  QTest::newRow( "normal" ) << scenario << expectedMailBox << expectedMessageCounts << expectedRecentCounts;


  scenario.clear();
  scenario << FakeServer::preauth()
           << "C: A000001 SELECT \"INBOX/Foo\""
           << "S: A000001 OK SELECT done"
           << "C: A000002 IDLE"
           << "S: + OK"
           << "S: * 0 RECENT"
           << "S: * 1 RECENT"
           << "S: A000002 OK done idling";

  expectedMailBox = "INBOX/Foo";

  expectedMessageCounts.clear();
  expectedRecentCounts.clear();

  expectedMessageCounts << -1 << -1;
  expectedRecentCounts << 0 << 1;

  QTest::newRow( "only RECENT" ) << scenario << expectedMailBox << expectedMessageCounts << expectedRecentCounts;


  scenario.clear();
  scenario << FakeServer::preauth()
           << "C: A000001 SELECT \"INBOX/Foo\""
           << "S: A000001 OK SELECT done"
           << "C: A000002 IDLE"
           << "S: + OK"
           << "S: * 1 EXISTS"
           << "S: * 2 EXISTS"
           << "S: A000002 OK done idling";

  expectedMailBox = "INBOX/Foo";

  expectedMessageCounts.clear();
  expectedRecentCounts.clear();

  expectedMessageCounts << 1 << 2;
  expectedRecentCounts << -1 << -1;

  QTest::newRow( "only EXISTS" ) << scenario << expectedMailBox << expectedMessageCounts << expectedRecentCounts;


  scenario.clear();
  scenario << FakeServer::preauth()
           << "C: A000001 SELECT \"INBOX/Foo\""
           << "S: A000001 OK SELECT done"
           << "C: A000002 IDLE"
           << "S: + OK"
           << "S: * 2 EXISTS"
           << "W: 150"
           << "S: * 1 RECENT"
           << "S: A000002 OK done idling";

  expectedMailBox = "INBOX/Foo";

  expectedMessageCounts.clear();
  expectedRecentCounts.clear();

  expectedMessageCounts << 2;
  expectedRecentCounts << 1;

  QTest::newRow( "under 200ms, same notification" ) << scenario << expectedMailBox << expectedMessageCounts << expectedRecentCounts;


  scenario.clear();
  scenario << FakeServer::preauth()
           << "C: A000001 SELECT \"INBOX/Foo\""
           << "S: A000001 OK SELECT done"
           << "C: A000002 IDLE"
           << "S: + OK"
           << "S: * 2 EXISTS"
           << "W: 250"
           << "S: * 1 RECENT"
           << "S: A000002 OK done idling";

  expectedMailBox = "INBOX/Foo";

  expectedMessageCounts.clear();
  expectedRecentCounts.clear();

  expectedMessageCounts << 2 << -1;
  expectedRecentCounts << -1 << 1;

  QTest::newRow( "above 200ms, two notifications" ) << scenario << expectedMailBox << expectedMessageCounts << expectedRecentCounts;
}

void shouldReactToIdle()
{
    QFETCH( QList<QByteArray>, scenario );
    QFETCH( QString, expectedMailBox );
    QFETCH( QList<int>, expectedMessageCounts );
    QFETCH( QList<int>, expectedRecentCounts );

    QVERIFY( expectedMessageCounts.size() == expectedRecentCounts.size() );

    FakeServer fakeServer;
    fakeServer.setScenario( scenario );
    fakeServer.startAndWait();

    KIMAP::Session session( "127.0.0.1", 5989 );

    KIMAP::SelectJob *select = new KIMAP::SelectJob( &session );
    select->setMailBox( expectedMailBox );
    QVERIFY( select->exec() );

    KIMAP::IdleJob *idle = new KIMAP::IdleJob( &session );

    QSignalSpy spy( idle, SIGNAL(mailBoxStats(KIMAP::IdleJob*,QString,int,int)) );

    bool result = idle->exec();

    if ( result ) {
      QCOMPARE( spy.count(), expectedMessageCounts.size() );

      for ( int i=0; i<spy.count(); i++ ) {
        const KIMAP::IdleJob *job = spy.at( i ).at( 0 ).value<KIMAP::IdleJob*>();
        const QString mailBox = spy.at( i ).at( 1 ).toString();
        const int messageCount = spy.at( i ).at( 2 ).toInt();
        const int recentCount = spy.at( i ).at( 3 ).toInt();

        QCOMPARE( job, idle );
        QCOMPARE( mailBox, expectedMailBox );
        QCOMPARE( messageCount, expectedMessageCounts.at( i ) );
        QCOMPARE( recentCount, expectedRecentCounts.at( i ) );
      }
    }

    fakeServer.quit();
}


};

QTEST_KDEMAIN_CORE( IdleJobTest )

#include "idlejobtest.moc"
