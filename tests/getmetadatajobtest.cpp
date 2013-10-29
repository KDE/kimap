/*
   Copyright (C) 2013 Christian Mollekopf <mollekopf@kolabsys.com>

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
#include "kimap/session.h"
#include "kimap/getmetadatajob.h"

#include <QTcpSocket>
#include <QtTest>
#include <KDebug>

typedef QMap<QByteArray,QByteArray> MAP;
Q_DECLARE_METATYPE(MAP)

class GetMetadataJobTest: public QObject {
  Q_OBJECT

private Q_SLOTS:

void metadata_data()
{
  QTest::addColumn<QList<QByteArray> >( "scenario" );
  QTest::addColumn<QString>( "mailbox" );
  QTest::addColumn<QMap<QByteArray, QByteArray> >( "expectedAnnotations" );

  {
    QList<QByteArray> scenario;
    scenario << FakeServer::preauth()
            << "C: A000001 GETMETADATA \"Folder1\" (DEPTH infinity) (/shared)"
            << "S: * METADATA \"Folder1\" (/shared/comment \"Shared comment\")"
            << "S: * METADATA \"Folder1\" (/private/comment \"My own comment\")"
            << "S: A000001 OK GETMETADATA complete";
    QMap<QByteArray, QByteArray> expected;
    expected.insert("/shared/comment", "Shared comment");
    expected.insert("/private/comment", "My own comment");
    QTest::newRow( "normal" ) << scenario << "Folder1" << expected;
  }
  {
    QList<QByteArray> scenario;
    scenario << FakeServer::preauth()
            << "C: A000001 GETMETADATA \"Folder1\" (DEPTH infinity) (/shared)"
            << "S: * METADATA \"Folder1\" (/shared/comment \"Shared comment\" /private/comment \"My own comment\")"
            << "S: A000001 OK GETMETADATA complete";
    QMap<QByteArray, QByteArray> expected;
    expected.insert("/shared/comment", "Shared comment");
    expected.insert("/private/comment", "My own comment");
    QTest::newRow( "combined response" ) << scenario << "Folder1" << expected;
  }
}

void metadata()
{
  QFETCH( QList<QByteArray>, scenario );
  QFETCH( QString, mailbox );
  QFETCH( MAP, expectedAnnotations );

  FakeServer fakeServer;
  fakeServer.setScenario( scenario );
  fakeServer.startAndWait();

  KIMAP::Session session( "127.0.0.1", 5989 );

  KIMAP::GetMetaDataJob *getMetadataJob = new KIMAP::GetMetaDataJob( &session );
  getMetadataJob->setServerCapability( KIMAP::MetaDataJobBase::Metadata );
  getMetadataJob->setMailBox( mailbox );
  getMetadataJob->setDepth( KIMAP::GetMetaDataJob::AllLevels );
  getMetadataJob->addEntry( "/shared" );

  QVERIFY( getMetadataJob->exec() );
  QCOMPARE( getMetadataJob->allMetaData( mailbox ).size(), expectedAnnotations.size() );
  foreach ( const QByteArray &entry, expectedAnnotations.keys() ) {
    QCOMPARE( getMetadataJob->metaData(mailbox, entry), expectedAnnotations.value(entry) );
  }

  fakeServer.quit();
}

};

QTEST_KDEMAIN_CORE( GetMetadataJobTest )

#include "getmetadatajobtest.moc"
