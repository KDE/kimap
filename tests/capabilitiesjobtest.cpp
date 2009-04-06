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
#include "kimap/capabilitiesjob.h"

#include <QTcpSocket>
#include <QtTest>
#include <KDebug>


class CapabilitiesJobTest: public QObject {
  Q_OBJECT

private Q_SLOTS:    

void testCapabilities_data() {
  QTest::addColumn<QStringList>( "response" );
  QTest::addColumn<QStringList>( "capabilities" );
  QStringList response;
  QStringList capabilities;
  response << "* CAPABILITY IMAP4rev1 STARTTLS AUTH=GSSAPI" << "A000001 OK CAPABILITY completed";
  capabilities << "IMAP4rev1" << "STARTTLS" <<  "AUTH=GSSAPI";
  QTest::newRow( "good" ) << response << capabilities;

  response.clear();
  capabilities.clear();
  response << "A000001 BAD command unknown or arguments invalid";
  QTest::newRow( "bad" ) << response << capabilities;
  
  response.clear();
  capabilities.clear();
  response << "* CAPABILITY IMAP4rev1 STARTTLS AUTH=PLAIN" << "* some response" << "A000001 OK CAPABILITY completed";
  capabilities << "IMAP4rev1" << "STARTTLS" <<  "AUTH=PLAIN";
  QTest::newRow( "extra-untagged" ) << response << capabilities;;
}

void testCapabilities()
{
    FakeServer fakeServer;
    fakeServer.start();
    KIMAP::Session session("127.0.0.1", 5989);
    QFETCH( QStringList, response );
    QFETCH( QStringList, capabilities );
    fakeServer.setResponse( response );

    KIMAP::CapabilitiesJob *job = new KIMAP::CapabilitiesJob(&session);
    QEXPECT_FAIL("bad" , "Expected failure on BAD response", Continue);
    bool result = job->exec();
    QVERIFY(result);
    if (result) {
      QCOMPARE(job->capabilities(), capabilities);
    }
    fakeServer.quit();
}


};

QTEST_KDEMAIN( CapabilitiesJobTest, NoGUI )

#include "capabilitiesjobtest.moc"
