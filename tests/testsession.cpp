/*
    This file is part of the KDE project
    Copyright (C) 2008 Kevin Ottens <ervin@kde.org>

    Copyright (c) 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
    Author: Kevin Ottens <kevin@kdab.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
  */

#include <QtCore/QEventLoop>
#include <QtCore/QObject>
#include <QtTest/QtTest>

#include "session.h"
#include "job.h"
#include "kimaptest/fakeserver.h"
#include "kimaptest/mockjob.h"

Q_DECLARE_METATYPE(KIMAP::Session::State)

class SessionTest : public QObject
{
  Q_OBJECT

  private slots:

    void initTestCase()
    {
      qRegisterMetaType<KIMAP::Session::State>();
    }

    void shouldStartDisconnected()
    {
      FakeServer fakeServer;
      fakeServer.setScenario( QList<QByteArray>()
         << FakeServer::greeting()
      );
      fakeServer.startAndWait();
      KIMAP::Session s( "127.0.0.1", 5989 );
      QSignalSpy spy(&s, SIGNAL(stateChanged(KIMAP::Session::State,KIMAP::Session::State)));
      QCOMPARE( ( int )s.state(), ( int )KIMAP::Session::Disconnected );
      QTest::qWait( 600 );
      QCOMPARE( ( int )s.state(), ( int )KIMAP::Session::NotAuthenticated );
      QCOMPARE( spy.count(), 1 ); // NotAuthenticated
      QList<QVariant> arguments = spy.takeFirst();
      QCOMPARE( ( int )qvariant_cast<KIMAP::Session::State>(arguments.at(0)), ( int )KIMAP::Session::NotAuthenticated);
      QCOMPARE( ( int )qvariant_cast<KIMAP::Session::State>(arguments.at(1)), ( int )KIMAP::Session::Disconnected);
    }

    void shouldSupportPreauth()
    {
      FakeServer fakeServer;
      fakeServer.setScenario( QList<QByteArray>()
         << FakeServer::preauth()
      );
      fakeServer.startAndWait();

      KIMAP::Session s( "127.0.0.1", 5989 );
      QSignalSpy spy(&s, SIGNAL(stateChanged(KIMAP::Session::State,KIMAP::Session::State)));
      QCOMPARE( ( int )s.state(), ( int )KIMAP::Session::Disconnected );
      QTest::qWait( 500 );
      QCOMPARE( ( int )s.state(), ( int )KIMAP::Session::Authenticated );
      QCOMPARE( spy.count(), 1 ); // Authenticated
      QList<QVariant> arguments = spy.takeFirst();
      QCOMPARE( ( int )qvariant_cast<KIMAP::Session::State>(arguments.at(0)), ( int )KIMAP::Session::Authenticated);
      QCOMPARE( ( int )qvariant_cast<KIMAP::Session::State>(arguments.at(1)), ( int )KIMAP::Session::Disconnected);
    }

    void shouldRespectStartOrder()
    {
      FakeServer fakeServer;
      fakeServer.setScenario( QList<QByteArray>()
         << FakeServer::greeting()
      );
      fakeServer.startAndWait();

      KIMAP::Session s("127.0.0.1", 5989);
      MockJob *j1 = new MockJob(&s);
      connect(j1, SIGNAL(result(KJob*)), this, SLOT(jobDone(KJob*)));
      MockJob *j2 = new MockJob(&s);
      connect(j2, SIGNAL(result(KJob*)), this, SLOT(jobDone(KJob*)));
      MockJob *j3 = new MockJob(&s);
      connect(j3, SIGNAL(result(KJob*)), this, SLOT(jobDone(KJob*)));
      MockJob *j4 = new MockJob(&s);
      connect(j4, SIGNAL(result(KJob*)), this, SLOT(jobDone(KJob*)));

      j4->start();
      j2->start();
      j3->start();
      j1->start();

      m_expectedCalls = 4;
      m_eventLoop.exec();

      QCOMPARE(m_jobs.size(), 4);
      QCOMPARE(m_jobs[0], j4);
      QCOMPARE(m_jobs[1], j2);
      QCOMPARE(m_jobs[2], j3);
      QCOMPARE(m_jobs[3], j1);
    }

    void shouldManageQueueSize()
    {
      FakeServer fakeServer;
      fakeServer.setScenario( QList<QByteArray>()
         << FakeServer::greeting()
      );
      fakeServer.startAndWait();

      KIMAP::Session s("127.0.0.1", 5989);

      QSignalSpy queueSpy(&s, SIGNAL(jobQueueSizeChanged(int)));

      QCOMPARE( s.jobQueueSize(), 0 );

      MockJob *j1 = new MockJob(&s);
      MockJob *j2 = new MockJob(&s);
      MockJob *j3 = new MockJob(&s);
      MockJob *j4 = new MockJob(&s);
      connect(j4, SIGNAL(result(KJob*)), &m_eventLoop, SLOT(quit()));

      QCOMPARE( s.jobQueueSize(), 0 );

      j1->start();
      QCOMPARE( s.jobQueueSize(), 1 );
      QCOMPARE( queueSpy.size(), 1 );
      QCOMPARE( queueSpy.at( 0 ).at( 0 ).toInt(), 1 );

      j2->start();
      QCOMPARE( s.jobQueueSize(), 2 );
      QCOMPARE( queueSpy.size(), 2 );
      QCOMPARE( queueSpy.at( 1 ).at( 0 ).toInt(), 2 );

      j3->start();
      QCOMPARE( s.jobQueueSize(), 3 );
      QCOMPARE( queueSpy.size(), 3 );
      QCOMPARE( queueSpy.at( 2 ).at( 0 ).toInt(), 3 );

      j4->start();
      QCOMPARE( s.jobQueueSize(), 4 );
      QCOMPARE( queueSpy.size(), 4 );
      QCOMPARE( queueSpy.at( 3 ).at( 0 ).toInt(), 4 );

      queueSpy.clear();
      m_eventLoop.exec();

      QCOMPARE( s.jobQueueSize(), 0 );

      QCOMPARE( queueSpy.at( 0 ).at( 0 ).toInt(), 3 );
      QCOMPARE( queueSpy.at( 1 ).at( 0 ).toInt(), 2 );
      QCOMPARE( queueSpy.at( 2 ).at( 0 ).toInt(), 1 );
      QCOMPARE( queueSpy.at( 3 ).at( 0 ).toInt(), 0 );
    }

    void shouldTimeoutOnNoReply()
    {
      FakeServer fakeServer;
      fakeServer.setScenario( QList<QByteArray>()
         << FakeServer::preauth()
         << "C: A000001 DUMMY"
         << "S: * DUMMY"
         << "S: * DUMMY"
         << "S: * DUMMY"
         << "S: * DUMMY"
         << "S: * DUMMY"
         << "S: * DUMMY"
         << "S: * DUMMY"
         << "S: * DUMMY"
         << "S: * DUMMY"
         << "S: * DUMMY"
         << "S: * DUMMY"
         << "S: * DUMMY"
         << "S: * DUMMY"
         // We never get a OK or anything, so the job can't normally complete
      );
      fakeServer.startAndWait();

      KIMAP::Session s( "127.0.0.1", 5989 );

      QSignalSpy spy(&s, SIGNAL(connectionLost()));
      QSignalSpy spyState(&s, SIGNAL(stateChanged(KIMAP::Session::State,KIMAP::Session::State)));

      MockJob *mock = new MockJob(&s);
      mock->setCommand("DUMMY");

      mock->exec();
      // We expect to get an error here due to some timeout
      QVERIFY( mock->error()!=0 );
      QCOMPARE( spy.count(), 1 );
      QCOMPARE( spyState.count(), 2 ); // Authenticated, Disconnected
    }

  public slots:
    void jobDone(KJob *job)
    {
      m_jobs << job;

      if (m_expectedCalls==m_jobs.size()) {
        m_eventLoop.quit();
      }
    }

  private:
    QEventLoop m_eventLoop;
    int m_expectedCalls;
    QList<KJob*> m_jobs;
};

QTEST_MAIN(SessionTest)

#include "testsession.moc"
