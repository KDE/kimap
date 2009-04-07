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
#include "kimap/listjob.h"

#include <QTcpSocket>
#include <QtTest>
#include <KDebug>

Q_DECLARE_METATYPE(QList<QByteArray>)
Q_DECLARE_METATYPE(QList<QList<QByteArray> >)

class ListJobTest: public QObject {
  Q_OBJECT

private Q_SLOTS:    

void testList_data() {
  QTest::addColumn<bool>( "unsubscribed" );
  QTest::addColumn<QStringList>( "response" );
  QTest::addColumn<QList< QList<QByteArray> > >( "listresult" );
  
  QStringList response;
  response << "* LIST ( \\HasChildren ) / INBOX "<< "* LIST ( \\HasNoChildren ) / INBOX/&AOQ- &APY- &APw- @ &IKw- "<< "* LIST ( \\HasChildren ) / INBOX/lost+found " << "* LIST ( \\HasNoChildren ) / \"INBOX/lost+found/Calender Public-20080128\" " << "A000001 OK LIST completed";
  QList<QByteArray> resultPair;
  QList<QList<QByteArray> > listresult;
    
  resultPair << "/" << "INBOX";
  listresult << resultPair;
  resultPair.clear();
  resultPair << "/" << "INBOX" << "lost+found";
  listresult << resultPair;
  resultPair.clear();
  resultPair << "/" << "INBOX" << "lost+found" << "Calender Public-20080128";
  listresult << resultPair;
  resultPair.clear();

  QTest::newRow( "normal" ) << true << response << listresult;
  
  response.clear();
  response << "* LSUB ( \\HasChildren ) / INBOX " <<  "* LSUB ( ) / INBOX/Calendar/3196 " << "* LSUB ( \\HasChildren ) / INBOX/Calendar/ff " << "* LSUB ( ) / INBOX/Calendar/ff/hgh "<< "* LSUB ( ) / user/test2/Calendar " << "A000001 OK LSUB completed";
  listresult.clear();
  
   resultPair << "/" << "INBOX";
  listresult << resultPair;
  resultPair.clear();
  resultPair << "/" << "INBOX" << "Calendar" << "3196";
  listresult << resultPair;
  resultPair.clear();
  resultPair << "/" << "INBOX" << "Calendar" << "ff";
  listresult << resultPair;
  resultPair.clear();
  resultPair << "/" << "INBOX" << "Calendar" << "ff" << "hgh";
  listresult << resultPair;
  resultPair.clear();
  resultPair << "/" << "user" << "test2" << "Calendar";
  listresult << resultPair;
  resultPair.clear();

  QTest::newRow( "subscribed" ) << false << response << listresult;
  
  response.clear();
  response << "* LIST ( \\HasNoChildren ) / INBOX/lost+found/Calender Public-20080128 " << "A000001 OK LIST completed";
    
  QTest::newRow( "unquoted-space" ) << true << response << listresult;
  
  response.clear();
  response << "A000001 BAD command unknown or arguments invalid";
  listresult.clear();
  QTest::newRow( "bad" ) << true << response << listresult;
  
  response.clear();
  response << "A000001 NO list failure";
  QTest::newRow( "no" ) << true << response << listresult;
}

void testList()
{
    FakeServer fakeServer;
    fakeServer.start();
    KIMAP::Session session("127.0.0.1", 5989);
    QFETCH( bool, unsubscribed);
    QFETCH( QStringList, response );
    QFETCH( QList< QList<QByteArray> >, listresult );
    
    fakeServer.setResponse( response );

    KIMAP::ListJob *job = new KIMAP::ListJob(&session);
    job->setIncludeUnsubscribed(unsubscribed);
    QEXPECT_FAIL("bad" , "Expected failure on BAD response", Continue);
    QEXPECT_FAIL("no" , "Expected failure on NO response", Continue);
    bool result = job->exec();
    QVERIFY(result);
    if (result) {
      QCOMPARE(job->mailBoxes(), listresult);
      //       kDebug() << job->flags();
    }
//     QCOMPARE(job->mailBox(), mailbox);

    fakeServer.quit();
}


};

QTEST_KDEMAIN( ListJobTest, NoGUI )

#include "listjobtest.moc"
