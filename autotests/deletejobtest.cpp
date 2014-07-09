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
#include "kimap/deletejob.h"

#include <QTcpSocket>
#include <QtTest>
#include <QDebug>

class DeleteJobTest: public QObject {
  Q_OBJECT

private Q_SLOTS:

void testDelete_data() {
  QTest::addColumn<QString>( "mailbox" );
  QTest::addColumn<QList<QByteArray> >( "scenario" );

  QList<QByteArray> scenario;
  scenario << FakeServer::preauth()
           << "C: A000001 DELETE \"foo\""
           << "S: A000001 OK DELETE completed";
  QTest::newRow( "good" ) << "foo" << scenario;

  scenario.clear();
  scenario << FakeServer::preauth()
           << "C: A000001 DELETE \"foo-BAD\""
           << "S: A000001 BAD command unknown or arguments invalid";
  QTest::newRow( "bad" ) << "foo-BAD" << scenario;

  scenario.clear();
  scenario << FakeServer::preauth()
           << "C: A000001 DELETE \"foo\""
           << "S: A000001 Name \"foo\" has inferior hierarchical names";
  QTest::newRow( "no" ) << "foo" << scenario;

  scenario.clear();
  scenario << FakeServer::preauth()
           << "C: A000001 DELETE \"foo/bar\""
           << "S: A000001 OK DELETE completed";
  QTest::newRow( "hierarchical" ) << "foo/bar" << scenario;
}

void testDelete()
{
    QFETCH( QString, mailbox );
    QFETCH( QList<QByteArray>, scenario );

    FakeServer fakeServer;
    fakeServer.setScenario( scenario );
    fakeServer.startAndWait();

    KIMAP::Session session( "127.0.0.1", 5989 );

    KIMAP::DeleteJob *job = new KIMAP::DeleteJob( &session );
    job->setMailBox( mailbox );
    bool result = job->exec();
    QEXPECT_FAIL( "bad" , "Expected failure on BAD response", Continue );
    QEXPECT_FAIL( "no" , "Expected failure on NO response", Continue );
    QVERIFY( result );
    if ( result ) {
      QCOMPARE( job->mailBox(), mailbox );
    }

    fakeServer.quit();
}

};

QTEST_GUILESS_MAIN( DeleteJobTest )

#include "deletejobtest.moc"
