/*
   Copyright (C) 2009 Kevin Ottens <ervin@kde.org>

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
#include "session.h"
#include "storejob.h"

#include <QTcpSocket>
#include <QtTest>
#include <QDebug>

class StoreJobTest: public QObject {
  Q_OBJECT

private Q_SLOTS:

void testStore_data() {
  QTest::addColumn<bool>( "uidBased" );
  QTest::addColumn<qint64>( "id" );
  QTest::addColumn<qint64>( "uid" );
  QTest::addColumn< QList<QByteArray> >( "flags" );
  QTest::addColumn< QList<QByteArray> >( "scenario" );

  QList<QByteArray> scenario;
  scenario << FakeServer::preauth()
           << "C: A000001 STORE 3 FLAGS (\\Seen \\Foo)"
           << "S: * 3 FETCH (FLAGS (\\Seen \\Foo) UID 1096)"
           << "S: A000001 OK STORE completed";

  QTest::newRow( "not uid based" ) << false << qint64(3) << qint64(1096)
                                   << ( QList<QByteArray>() << "\\Seen" << "\\Foo" )
                                   << scenario;

  scenario.clear();
  scenario << FakeServer::preauth()
           << "C: A000001 UID STORE 1096 FLAGS (\\Seen \\Foo)"
           << "S: * 3 FETCH (FLAGS (\\Seen \\Foo) UID 1096)"
           << "S: A000001 OK STORE completed";

  QTest::newRow( "uid based" ) << true << qint64(3) << qint64(1096)
                               << ( QList<QByteArray>() << "\\Seen" << "\\Foo" )
                               << scenario;
}

void testStore()
{
    QFETCH( bool, uidBased );
    QFETCH( qint64, id );
    QFETCH( qint64, uid );
    QFETCH( QList<QByteArray>, flags );
    QFETCH( QList<QByteArray>, scenario );

    FakeServer fakeServer;
    fakeServer.setScenario( scenario );
    fakeServer.startAndWait();

    KIMAP::Session session( "127.0.0.1", 5989 );

    KIMAP::StoreJob *job = new KIMAP::StoreJob( &session );
    job->setUidBased( uidBased );
    job->setSequenceSet( KIMAP::ImapSet( uidBased ? uid : id ) );
    job->setFlags( flags );
    job->setMode( KIMAP::StoreJob::SetFlags );
    bool result = job->exec();
    QVERIFY( result );
    if ( uidBased ) {
      QVERIFY( job->resultingFlags().contains( uid ) );
    } else {
      QVERIFY( job->resultingFlags().contains( id ) );
    }

    fakeServer.quit();
}

};

QTEST_KDEMAIN_CORE( StoreJobTest )

#include "storejobtest.moc"
