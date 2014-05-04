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

#include <qtest_kde.h>

#include "kimaptest/fakeserver.h"
#include "session.h"
#include "capabilitiesjob.h"

#include <QTcpSocket>
#include <QtTest>
#include <QDebug>

class CapabilitiesJobTest: public QObject {
  Q_OBJECT

private Q_SLOTS:

void testCapabilities_data() {
  QTest::addColumn<QList<QByteArray> >( "scenario" );
  QTest::addColumn<QStringList>( "capabilities" );
  QList<QByteArray> scenario;
  scenario << "S: * PREAUTH"
           << "C: A000001 CAPABILITY"
           << "S: * CAPABILITY IMAP4rev1 STARTTLS AUTH=GSSAPI"
           << "S: A000001 OK CAPABILITY completed";

  QStringList capabilities;
  capabilities << "IMAP4REV1" << "STARTTLS" <<  "AUTH=GSSAPI";
  QTest::newRow( "good" ) << scenario << capabilities;

  scenario.clear();
  capabilities.clear();
  scenario << "S: * PREAUTH"
           << "C: A000001 CAPABILITY"
           << "S: A000001 BAD command unknown or arguments invalid";
  QTest::newRow( "bad" ) << scenario << capabilities;

  scenario.clear();
  capabilities.clear();
  scenario << "S: * PREAUTH"
           << "C: A000001 CAPABILITY"
           << "S: * CAPABILITY IMAP4rev1 STARTTLS AUTH=PLAIN"
           << "S: * some response"
           << "S: A000001 OK CAPABILITY completed";

  capabilities << "IMAP4REV1" << "STARTTLS" <<  "AUTH=PLAIN";
  QTest::newRow( "extra-untagged" ) << scenario << capabilities;;
}

void testCapabilities()
{
    QFETCH( QList<QByteArray>, scenario );
    QFETCH( QStringList, capabilities );

    FakeServer fakeServer;
    fakeServer.setScenario( scenario );
    fakeServer.startAndWait();
    KIMAP::Session session( "127.0.0.1", 5989 );

    KIMAP::CapabilitiesJob *job = new KIMAP::CapabilitiesJob( &session );
    bool result = job->exec();
    QEXPECT_FAIL( "bad" , "Expected failure on BAD response", Continue );
    QVERIFY( result );
    if ( result ) {
      QCOMPARE( job->capabilities(), capabilities );
    }
    fakeServer.quit();
}

};

QTEST_KDEMAIN_CORE( CapabilitiesJobTest )

#include "capabilitiesjobtest.moc"
