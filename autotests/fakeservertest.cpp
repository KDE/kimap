/*
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

#include <QtTest>

class FakeServerTest: public QObject {
  Q_OBJECT

private Q_SLOTS:

void testLoadScenario() {
  KIMAP::MailBoxDescriptor descriptor;
  QList<KIMAP::MailBoxDescriptor> listresult;

  descriptor.separator = QLatin1Char('/');
  descriptor.name = QLatin1String("INBOX");
  listresult << descriptor;
  descriptor.separator = QLatin1Char('/');
  descriptor.name = QString::fromUtf8( "INBOX/ä ö ü @ €" );
  listresult << descriptor;
  descriptor.separator = QLatin1Char('/');
  descriptor.name = QLatin1String("INBOX/lost+found");
  listresult << descriptor;
  descriptor.separator = QLatin1Char('/');
  descriptor.name = QLatin1String("INBOX/lost+found/Calendar Public-20080128");
  listresult << descriptor;

  FakeServer fakeServer;
  fakeServer.addScenarioFromFile( QString(QLatin1String(TEST_DATA) + QLatin1String("/fakeserverscenario.log") ) );
  fakeServer.startAndWait();

  KIMAP::Session session( QLatin1String("127.0.0.1"), 5989 );

  KIMAP::ListJob *job = new KIMAP::ListJob( &session );
  job->setIncludeUnsubscribed( true );
  QVERIFY( job->exec() );

  fakeServer.quit();
}

};

QTEST_GUILESS_MAIN( FakeServerTest )

#include "fakeservertest.moc"
