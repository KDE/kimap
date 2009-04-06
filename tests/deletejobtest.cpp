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
#include "kimap/deletejob.h"

#include <QTcpSocket>
#include <QtTest>
#include <KDebug>

Q_DECLARE_METATYPE(QList<QByteArray>);

class DeleteJobTest: public QObject {
  Q_OBJECT

private Q_SLOTS:    

void testDelete_data() {
  QTest::addColumn<QByteArray>( "mailbox" );
  QTest::addColumn<QStringList>( "response" );
  
  QStringList response;
  response << "A000001 OK DELETE completed";
  QTest::newRow( "good" ) << QByteArray("foo") << response ;

  response.clear();
  response << "A000001 BAD command unknown or arguments invalid";
  QTest::newRow( "bad" ) << QByteArray("foo-BAD") << response;
  
  response.clear();
  response << "A000001 Name \"foo\" has inferior hierarchical names";
  QTest::newRow( "no" ) << QByteArray("foo") << response ;
  
  response.clear();
  response << "A000001 OK DELETE completed";
  QTest::newRow( "hierarchical" ) << QByteArray("foo/bar") << response ;
}

void testDelete()
{
    FakeServer fakeServer;
    fakeServer.start();
    KIMAP::Session session("127.0.0.1", 5989);
    QFETCH( QByteArray, mailbox );
    QFETCH( QStringList, response );
    
    fakeServer.setResponse( response );

    KIMAP::DeleteJob *job = new KIMAP::DeleteJob(&session);
    job->setMailBox(mailbox);
    QEXPECT_FAIL("bad" , "Expected failure on BAD response", Continue);
    QEXPECT_FAIL("no" , "Expected failure on NO response", Continue);
    bool result = job->exec();
    QVERIFY(result);
    if (result) {
      QCOMPARE(job->mailBox(), mailbox);
    }

    fakeServer.quit();
}


};

QTEST_KDEMAIN( DeleteJobTest, NoGUI )

#include "deletejobtest.moc"
