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
#include "kimap/subscribejob.h"

#include <QTcpSocket>
#include <QtTest>
#include <QDebug>

class SubscribeJobTest: public QObject {
  Q_OBJECT

private Q_SLOTS:

void testSubscribe_data() {
  QTest::addColumn<QString>( "mailbox" );
  QTest::addColumn< QList<QByteArray> >( "scenario" );

  QList<QByteArray> scenario;
  scenario << FakeServer::preauth()
           << "C: A000001 SUBSCRIBE \"INBOX/foo\""
           << "S: A000001 OK CREATE completed";
  QTest::newRow( "good" ) << "INBOX/foo"  << scenario ;

  scenario.clear();
  scenario << FakeServer::preauth()
           << "C: A000001 SUBSCRIBE \"INBOX-FAIL-BAD\""
           << "S: A000001 BAD command unknown or arguments invalid";
  QTest::newRow( "bad" ) << "INBOX-FAIL-BAD" << scenario;

  scenario.clear();
  scenario << FakeServer::preauth()
           << "C: A000001 SUBSCRIBE \"INBOX-FAIL-NO\""
           << "S: A000001 NO subscribe failure";
  QTest::newRow( "no" ) << "INBOX-FAIL-NO" << scenario ;
}

void testSubscribe()
{
    QFETCH( QString, mailbox );
    QFETCH( QList<QByteArray>, scenario );

    FakeServer fakeServer;
    fakeServer.setScenario( scenario );
    fakeServer.startAndWait();

    KIMAP::Session session( "127.0.0.1", 5989 );

    KIMAP::SubscribeJob *job = new KIMAP::SubscribeJob( &session );
    job->setMailBox( mailbox );
    bool result = job->exec();
    QEXPECT_FAIL( "bad" , "Expected failure on BAD scenario", Continue );
    QEXPECT_FAIL( "no" , "Expected failure on NO scenario", Continue );
    QVERIFY( result );
    QCOMPARE( job->mailBox(), mailbox );

    fakeServer.quit();
}

};

QTEST_KDEMAIN_CORE( SubscribeJobTest )

#include "subscribejobtest.moc"
