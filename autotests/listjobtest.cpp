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
#include "kimap/listjob.h"

#include <QTcpSocket>
#include <QtTest>
#include <QDebug>

Q_DECLARE_METATYPE( QList<KIMAP::MailBoxDescriptor> )
Q_DECLARE_METATYPE( QList< QList<QByteArray> > )

class ListJobTest: public QObject {
  Q_OBJECT

private Q_SLOTS:

void testList_data() {
  QTest::addColumn<bool>( "unsubscribed" );
  QTest::addColumn<QList<QByteArray> >( "scenario" );
  QTest::addColumn<QList<KIMAP::MailBoxDescriptor> >( "listresult" );

  QList<QByteArray> scenario;
  scenario << FakeServer::preauth()
           << "C: A000001 LIST \"\" *"
           << "S: * LIST ( \\HasChildren ) / INBOX"
           << "S: * LIST ( \\HasNoChildren ) / INBOX/&AOQ- &APY- &APw- @ &IKw-"
           << "S: * LIST ( \\HasChildren ) / INBOX/lost+found"
           << "S: * LIST ( \\HasNoChildren ) / \"INBOX/lost+found/Calendar Public-20080128\""
           << "S: A000001 OK LIST completed";
  KIMAP::MailBoxDescriptor descriptor;
  QList<KIMAP::MailBoxDescriptor> listresult;

  descriptor.separator = '/';
  descriptor.name = "INBOX";
  listresult << descriptor;
  descriptor.separator = '/';
  descriptor.name = QString::fromUtf8( "INBOX/ä ö ü @ €" );
  listresult << descriptor;
  descriptor.separator = '/';
  descriptor.name = "INBOX/lost+found";
  listresult << descriptor;
  descriptor.separator = '/';
  descriptor.name = "INBOX/lost+found/Calendar Public-20080128";
  listresult << descriptor;

  QTest::newRow( "normal" ) << true << scenario << listresult;

  scenario.clear();;
  scenario << FakeServer::preauth()
           << "C: A000001 LIST \"\" *"
           << "S: * LIST ( \\HasChildren ) / Inbox"
           << "S: * LIST ( \\HasNoChildren ) / Inbox/&AOQ- &APY- &APw- @ &IKw-"
           << "S: * LIST ( \\HasChildren ) / Inbox/lost+found"
           << "S: * LIST ( \\HasNoChildren ) / \"Inbox/lost+found/Calendar Public-20080128\""
           << "S: A000001 OK LIST completed";
  listresult.clear();

  descriptor.separator = '/';
  descriptor.name = "INBOX";
  listresult << descriptor;
  descriptor.separator = '/';
  descriptor.name = QString::fromUtf8( "INBOX/ä ö ü @ €" );
  listresult << descriptor;
  descriptor.separator = '/';
  descriptor.name = "INBOX/lost+found";
  listresult << descriptor;
  descriptor.separator = '/';
  descriptor.name = "INBOX/lost+found/Calendar Public-20080128";
  listresult << descriptor;

  QTest::newRow( "lowercase Inbox" ) << true << scenario << listresult;

  scenario.clear();
  scenario << FakeServer::preauth()
           << "C: A000001 LSUB \"\" *"
           << "S: * LSUB ( \\HasChildren ) / INBOX"
           << "S: * LSUB ( ) / INBOX/Calendar/3196"
           << "S: * LSUB ( \\HasChildren ) / INBOX/Calendar/ff"
           << "S: * LSUB ( ) / INBOX/Calendar/ff/hgh"
           << "S: * LSUB ( ) / user/test2/Calendar"
           << "S: A000001 OK LSUB completed";
  listresult.clear();

  descriptor.separator = '/';
  descriptor.name = "INBOX";
  listresult << descriptor;
  descriptor.separator = '/';
  descriptor.name = "INBOX/Calendar/3196";
  listresult << descriptor;
  descriptor.separator = '/';
  descriptor.name = "INBOX/Calendar/ff";
  listresult << descriptor;
  descriptor.separator = '/';
  descriptor.name = "INBOX/Calendar/ff/hgh";
  listresult << descriptor;
  descriptor.separator = '/';
  descriptor.name = "user/test2/Calendar";
  listresult << descriptor;

  QTest::newRow( "subscribed" ) << false << scenario << listresult;

 scenario.clear();
  scenario << FakeServer::preauth()
           << "C: A000001 LSUB \"\" *"
           << "S: * LSUB ( \\HasChildren ) / Inbox"
           << "S: * LSUB ( ) / Inbox/Calendar/3196"
           << "S: * LSUB ( \\HasChildren ) / Inbox/Calendar/ff"
           << "S: * LSUB ( ) / Inbox/Calendar/ff/hgh"
           << "S: * LSUB ( ) / user/test2/Calendar"
           << "S: A000001 OK LSUB completed";
  listresult.clear();

  descriptor.separator = '/';
  descriptor.name = "INBOX";
  listresult << descriptor;
  descriptor.separator = '/';
  descriptor.name = "INBOX/Calendar/3196";
  listresult << descriptor;
  descriptor.separator = '/';
  descriptor.name = "INBOX/Calendar/ff";
  listresult << descriptor;
  descriptor.separator = '/';
  descriptor.name = "INBOX/Calendar/ff/hgh";
  listresult << descriptor;
  descriptor.separator = '/';
  descriptor.name = "user/test2/Calendar";
  listresult << descriptor;

  QTest::newRow( "subscribed, lowercase Inbox" ) << false << scenario << listresult;

  scenario.clear();
  scenario << FakeServer::preauth()
           << "C: A000001 LIST \"\" *"
           << "S: * LIST ( \\HasNoChildren ) / INBOX/lost+found/Calendar Public-20080128"
           << "S: A000001 OK LIST completed";
  listresult.clear();
  descriptor.separator = '/';
  descriptor.name = "INBOX/lost+found/Calendar Public-20080128";
  listresult << descriptor;

  QTest::newRow( "unquoted-space" ) << true << scenario << listresult;

  scenario.clear();
  scenario << FakeServer::preauth()
           << "C: A000001 LIST \"\" *"
           << "S: * LIST ( \\NoInferiors ) ( ) INBOX"
           << "S: A000001 OK LIST completed";
  listresult.clear();
  descriptor.separator = '/';
  descriptor.name = "INBOX";
  listresult << descriptor;

  QTest::newRow( "separator is empty list" ) << true << scenario << listresult;

  scenario.clear();
  scenario << FakeServer::preauth()
           << "C: A000001 LIST \"\" *"
           << "S: A000001 BAD command unknown or arguments invalid";
  listresult.clear();
  QTest::newRow( "bad" ) << true << scenario << listresult;

  scenario.clear();
  scenario << FakeServer::preauth()
           << "C: A000001 LIST \"\" *"
           << "S: A000001 NO list failure";
  QTest::newRow( "no" ) << true << scenario << listresult;
}

void testList()
{
    QFETCH( bool, unsubscribed);
    QFETCH( QList<QByteArray>, scenario );
    QFETCH( QList<KIMAP::MailBoxDescriptor>, listresult );

    FakeServer fakeServer;
    fakeServer.setScenario( scenario );
    fakeServer.startAndWait();

    KIMAP::Session session( "127.0.0.1", 5989 );

    KIMAP::ListJob *job = new KIMAP::ListJob( &session );
    job->setIncludeUnsubscribed( unsubscribed );

    QSignalSpy spy( job,
                    SIGNAL(mailBoxesReceived(const QList<KIMAP::MailBoxDescriptor>&,
                                             const QList< QList<QByteArray> >&)) );

    bool result = job->exec();
    QEXPECT_FAIL( "bad" , "Expected failure on BAD response", Continue );
    QEXPECT_FAIL( "no" , "Expected failure on NO response", Continue );
    QVERIFY( result );
    if ( result ) {
      QVERIFY( spy.count() > 0 );
      QList<KIMAP::MailBoxDescriptor> mailBoxes;

      for ( int i = 0; i < spy.count(); i++ ) {
        mailBoxes+= spy.at( i ).at( 0 ).value< QList<KIMAP::MailBoxDescriptor> >();
      }

      //qDebug() << mailBoxes.first().name;
      //qDebug() << listresult.first().name;
      QCOMPARE( mailBoxes, listresult );
    }
//     QCOMPARE(job->mailBox(), mailbox);

    fakeServer.quit();
}

};

QTEST_GUILESS_MAIN( ListJobTest )

#include "listjobtest.moc"
