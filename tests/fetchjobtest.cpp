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

#include <qtest_kde.h>

#include "kimaptest/fakeserver.h"
#include "kimap/session.h"
#include "kimap/fetchjob.h"

#include <QTcpSocket>
#include <QtTest>
#include <KDebug>

Q_DECLARE_METATYPE(KIMAP::FetchJob::FetchScope)

class FetchJobTest: public QObject {
  Q_OBJECT

private:
  QStringList m_signals;

  QMap<qint64, qint64> m_uids;
  QMap<qint64, qint64> m_sizes;
  QMap<qint64, KIMAP::MessageFlags> m_flags;
  QMap<qint64, KIMAP::MessagePtr> m_messages;

public slots:
void onHeadersReceived( const QString &/*mailBox*/,
                        const QMap<qint64, qint64> &uids,
                        const QMap<qint64, qint64> &sizes,
                        const QMap<qint64, KIMAP::MessageFlags> &flags,
                        const QMap<qint64, KIMAP::MessagePtr> &messages )
  {
    m_signals << "headersReceived";
    m_uids.unite( uids );
    m_sizes.unite( sizes );
    m_flags.unite( flags );
    m_messages.unite( messages );
  }


private Q_SLOTS:

void testFetch_data() {
  qRegisterMetaType<KIMAP::FetchJob::FetchScope>();

  QTest::addColumn<bool>( "uidBased" );
  QTest::addColumn< KIMAP::ImapSet >( "set" );
  QTest::addColumn<int>( "expectedMessageCount" );
  QTest::addColumn< QList<QByteArray> >( "scenario" );
  QTest::addColumn<KIMAP::FetchJob::FetchScope>( "scope" );

  KIMAP::FetchJob::FetchScope scope;
  scope.mode = KIMAP::FetchJob::FetchScope::Flags;

  QList<QByteArray> scenario;
  scenario << FakeServer::preauth()
           << "C: A000001 FETCH 1:4 (FLAGS UID)"
           << "S: * 1 FETCH ( FLAGS () UID 1 )"
           << "S: * 2 FETCH ( FLAGS () UID 2 )"
           << "S: * 3 FETCH ( FLAGS () UID 3 )"
           << "S: * 4 FETCH ( FLAGS () UID 4 )"
           << "S: A000001 OK fetch done";

  QTest::newRow( "messages have empty flags" ) << false << KIMAP::ImapSet( 1, 4 ) << 4
                                               << scenario << scope;

  scenario.clear();
  // kill the connection part-way through a list, with carriage returns at end
  // BUG 253619
  // this should fail, but it shouldn't crash
  scenario << FakeServer::preauth()
           << "C: A000001 FETCH 11 (RFC822.SIZE INTERNALDATE BODY.PEEK[HEADER.FIELDS (TO FROM MESSAGE-ID REFERENCES IN-REPLY-TO SUBJECT DATE)] FLAGS UID)"
           << "S: * 11 FETCH (RFC822.SIZE 770 INTERNALDATE \"11-Oct-2010 03:33:50 +0100\" BODY[HEADER.FIELDS (TO FROM MESSAGE-ID REFERENCES IN-REPLY-TO SUBJECT DATE)] {246}"
           << "S: From: John Smith <jonathanr.smith@foobarbaz.com>\r\nTo: \"amagicemailaddress@foobarbazbarfoo.com\"\r\n\t<amagicemailaddress@foobarbazbarfoo.com>\r\nDate: Mon, 11 Oct 2010 03:34:48 +0100\r\nSubject: unsubscribe\r\nMessage-ID: <ASDFFDSASDFFDS@foobarbaz.com>\r\n\r\n"
           << "X";
  scope.mode = KIMAP::FetchJob::FetchScope::Headers;
  QTest::newRow( "partial" ) << false << KIMAP::ImapSet( 11, 11 ) << 1 << scenario << scope;
}

void testFetch()
{
    QFETCH( bool, uidBased );
    QFETCH( KIMAP::ImapSet, set );
    QFETCH( int, expectedMessageCount );
    QFETCH( QList<QByteArray>, scenario );
    QFETCH( KIMAP::FetchJob::FetchScope, scope );

    FakeServer fakeServer;
    fakeServer.setScenario( scenario );
    fakeServer.startAndWait();

    KIMAP::Session session("127.0.0.1", 5989);

    KIMAP::FetchJob *job = new KIMAP::FetchJob(&session);
    job->setUidBased( uidBased );
    job->setSequenceSet( set );
    job->setScope( scope );

    connect( job, SIGNAL(headersReceived(QString,
                                         QMap<qint64, qint64>,
                                         QMap<qint64, qint64>,
                                         QMap<qint64, KIMAP::MessageFlags>,
                                         QMap<qint64, KIMAP::MessagePtr>)),
             this, SLOT(onHeadersReceived(QString,
                                          QMap<qint64, qint64>,
                                          QMap<qint64, qint64>,
                                          QMap<qint64, KIMAP::MessageFlags>,
                                          QMap<qint64, KIMAP::MessagePtr>)) );


    bool result = job->exec();
    QEXPECT_FAIL("partial" , "Expected failure on partial response", Continue);
    QVERIFY( result );
    if ( result ) {
      QVERIFY( m_signals.count()>0 );
      QCOMPARE( m_uids.count(), expectedMessageCount );
    }

    QVERIFY( fakeServer.isAllScenarioDone() );
    fakeServer.quit();

    m_signals.clear();
    m_uids.clear();
    m_sizes.clear();
    m_flags.clear();
    m_messages.clear();
}


};

QTEST_KDEMAIN_CORE( FetchJobTest )

#include "fetchjobtest.moc"
