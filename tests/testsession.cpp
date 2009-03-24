/**
  * This file is part of the KDE project
  * Copyright (C) 2008 Kevin Ottens <ervin@kde.org>
  *
  * This library is free software; you can redistribute it and/or
  * modify it under the terms of the GNU Library General Public
  * License as published by the Free Software Foundation; either
  * version 2 of the License, or (at your option) any later version.
  *
  * This library is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  * Library General Public License for more details.
  *
  * You should have received a copy of the GNU Library General Public License
  * along with this library; see the file COPYING.LIB.  If not, write to
  * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  * Boston, MA 02110-1301, USA.
  */

#include <QtCore/QEventLoop>
#include <QtCore/QObject>
#include <QtTest/QtTest>

#include "session.h"
#include "job.h"

class MockJob : public KIMAP::Job
{
  Q_OBJECT

  public:
    MockJob(KIMAP::Session *session)
      : KIMAP::Job(session) { }

    virtual void doStart()
    {
      QTimer::singleShot(1000, this, SLOT(done()));
    }

  private slots:
    void done()
    {
      emitResult();
    }
};

class SessionTest : public QObject
{
  Q_OBJECT

  private slots:
    void shouldRespectStartOrder()
    {
      KIMAP::Session s("mail.kdab.net", 143);
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
