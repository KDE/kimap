/*
   Copyright (C) 2009 Kevin Ottens <ervin@kde.org>

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
#include "kimap/storejob.h"

#include <QTcpSocket>
#include <QtTest>
#include <KDebug>

Q_DECLARE_METATYPE(QList<QByteArray>)

class StoreJobTest: public QObject {
  Q_OBJECT

private Q_SLOTS:

void testStore_data() {
  QTest::addColumn<bool>( "uidBased" );
  QTest::addColumn<qint64>( "id" );
  QTest::addColumn<qint64>( "uid" );
  QTest::addColumn< QList<QByteArray> >( "flags" );
  QTest::addColumn<QStringList>( "response" );

  QStringList response;
  response << "* 3 FETCH (FLAGS (\\Seen \\Foo) UID 1096)";
  response << "A000001 OK STORE completed";

  QTest::newRow( "not uid based" ) << false << qint64(3) << qint64(1096)
                                   << ( QList<QByteArray>() << "\\Seen" << "\\Foo" )
                                   << response;

  QTest::newRow( "uid based" ) << true << qint64(3) << qint64(1096)
                               << ( QList<QByteArray>() << "\\Seen" << "\\Foo" )
                               << response;
}

void testStore()
{
    FakeServer fakeServer;
    fakeServer.start();
    KIMAP::Session session("127.0.0.1", 5989);
    QFETCH( bool, uidBased );
    QFETCH( qint64, id );
    QFETCH( qint64, uid );
    QFETCH( QList<QByteArray>, flags );
    QFETCH( QStringList, response );

    fakeServer.setResponse( response );

    KIMAP::StoreJob *job = new KIMAP::StoreJob(&session);
    job->setUidBased( uidBased );
    job->setSequenceSet( KIMAP::ImapSet( uidBased ? uid : id ) );
    job->setFlags( flags );
    job->setMode( KIMAP::StoreJob::SetFlags );
    bool result = job->exec();
    QVERIFY(result);
    if ( uidBased ) {
      QVERIFY( job->resultingFlags().contains( uid ) );
    } else {
      QVERIFY( job->resultingFlags().contains( id ) );
    }

    fakeServer.quit();
}


};

QTEST_KDEMAIN( StoreJobTest, NoGUI )

#include "storejobtest.moc"
