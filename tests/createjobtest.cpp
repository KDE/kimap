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
#include "kimap/createjob.h"

#include <QTcpSocket>
#include <QtTest>
#include <KDebug>

Q_DECLARE_METATYPE(QList<QByteArray>)

class CreateJobTest: public QObject {
  Q_OBJECT

private Q_SLOTS:

void testCreate_data() {
  QTest::addColumn<QString>( "mailbox" );
  QTest::addColumn<QStringList>( "response" );

  QStringList response;
  response << "A000001 OK CREATE completed";
  QTest::newRow( "good" ) << "INBOX" << response;

  response.clear();
  response << "A000001 BAD command unknown or arguments invalid";
  QTest::newRow( "bad" ) << "INBOX-FAIL-BAD" << response;

  response.clear();
  response << "A000001 NO create failure";
  QTest::newRow( "no" ) << "INBOX-FAIL-NO" << response;
}

void testCreate()
{
    FakeServer fakeServer;
    fakeServer.start();
    KIMAP::Session session("127.0.0.1", 5989);
    QFETCH( QString, mailbox );
    QFETCH( QStringList, response );

    fakeServer.setResponse( response );

    KIMAP::CreateJob *job = new KIMAP::CreateJob(&session);
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

QTEST_KDEMAIN( CreateJobTest, NoGUI )

#include "createjobtest.moc"
