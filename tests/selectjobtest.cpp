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
#include "kimap/loginjob.h"
#include "kimap/session.h"
#include "kimap/selectjob.h"

#include <QTcpSocket>
#include <QtTest>
#include <KDebug>

Q_DECLARE_METATYPE(QList<QByteArray>)

class SelectJobTest: public QObject {
  Q_OBJECT

private Q_SLOTS:

void testSingleSelect_data() {
  QTest::addColumn<QStringList>( "response" );
  QTest::addColumn<QList<QByteArray> >( "flags" );
  QTest::addColumn<QList<QByteArray> >( "permanentflags" );
  QTest::addColumn<int>( "messagecount" );
  QTest::addColumn<int>( "recentcount" );
  QTest::addColumn<int>( "firstUnseenIndex" );
  QTest::addColumn<qint64>( "uidValidity" );
  QTest::addColumn<int>( "nextUid" );

  QStringList response;
  QList<QByteArray> flags;
  QList<QByteArray> permanentflags;
  response << "* 172 EXISTS" << "* 1 RECENT" << "* OK [UNSEEN 12] Message 12 is first unseen" << "* OK [UIDVALIDITY 3857529045] UIDs valid" << "* OK [UIDNEXT 4392] Predicted next UID" << "* FLAGS (\\Answered \\Flagged \\Deleted \\Seen \\Draft)" << "* OK [PERMANENTFLAGS (\\Deleted \\Seen \\*)] Limited" << "A000002 OK [READ-WRITE] SELECT completed";
  flags << "\\Answered" << "\\Flagged" << "\\Deleted" << "\\Seen" << "\\Draft";
  permanentflags << "\\Deleted" << "\\Seen" << "\\*";
  QTest::newRow( "good" ) << response << flags << permanentflags << 172 << 1 << 12 << (qint64)3857529045 << 4392;

  response.clear();
  flags.clear();
  permanentflags.clear();
  response << "A000002 BAD command unknown or arguments invalid";
  QTest::newRow( "bad" ) << response << flags << permanentflags << 0 << 0 << 0 << (qint64)0 << 0;

  response.clear();
  flags.clear();
  permanentflags.clear();
  response << "A000002 NO select failure";
  QTest::newRow( "no" ) << response << flags << permanentflags << 0 << 0 << 0 << (qint64)0 << 0;
}

void testSingleSelect()
{
    FakeServer fakeServer;
    fakeServer.start();
    KIMAP::Session session("127.0.0.1", 5989);

    fakeServer.setResponse( QStringList() << "A000001 OK User logged in" );
    KIMAP::LoginJob *login = new KIMAP::LoginJob(&session);
    login->setUserName("user");
    login->setPassword("password");
    QVERIFY(login->exec());

    QFETCH( QStringList, response );
    QFETCH( QList<QByteArray>, flags );
    QFETCH( QList<QByteArray>, permanentflags );
    QFETCH( int, messagecount);
    QFETCH( int, recentcount);
    QFETCH( int, firstUnseenIndex);
    QFETCH( qint64, uidValidity);
    QFETCH( int, nextUid);

    fakeServer.setResponse( response );

    KIMAP::SelectJob *job = new KIMAP::SelectJob(&session);
    job->setMailBox("INBOX");
    QEXPECT_FAIL("bad" , "Expected failure on BAD response", Continue);
    QEXPECT_FAIL("no" , "Expected failure on NO response", Continue);
    bool result = job->exec();
    QVERIFY(result);
    if (result) {
      QCOMPARE(job->flags(), flags);
      QCOMPARE(job->permanentFlags(), permanentflags);
      QCOMPARE(job->messageCount(), messagecount);
      QCOMPARE(job->recentCount(), recentcount);
      QCOMPARE(job->firstUnseenIndex(), firstUnseenIndex);
      QCOMPARE(job->uidValidity(), uidValidity);
      QCOMPARE(job->nextUid(), nextUid);
    }
    fakeServer.quit();
}

void testSeveralSelect()
{
    FakeServer fakeServer;
    fakeServer.start();
    KIMAP::Session session("127.0.0.1", 5989);

    fakeServer.setResponse( QStringList() << "A000001 OK User logged in" );
    KIMAP::LoginJob *login = new KIMAP::LoginJob(&session);
    login->setUserName("user");
    login->setPassword("password");
    QVERIFY(login->exec());

    fakeServer.setResponse( QStringList() << "A000002 OK [READ-WRITE] SELECT completed" );
    KIMAP::SelectJob *job = new KIMAP::SelectJob(&session);
    job->setMailBox("INBOX");
    QVERIFY(job->exec());

    fakeServer.setResponse( QStringList() << "A000003 OK [READ-WRITE] SELECT completed" );
    job = new KIMAP::SelectJob(&session);
    job->setMailBox("INBOX/Foo");
    QVERIFY(job->exec());
}


};

QTEST_KDEMAIN( SelectJobTest, NoGUI )

#include "selectjobtest.moc"
