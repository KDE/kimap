/*
   Copyright (C) 2008 Omat Holding B.V. <info@omat.nl>

   Copyright (C) 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
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

// Own
#include "fakeserver.h"

// Qt
#include <QDebug>
#include <QTcpServer>

// KDE
#include <KDebug>
#include <qtest_kde.h>

#include "kimap/imapstreamparser.h"

QByteArray FakeServer::preauth()
{
    return "S: * PREAUTH localhost Test Library server ready";
}

QByteArray FakeServer::greeting()
{
    return "S: * OK localhost Test Library server ready";
}

FakeServer::FakeServer( QObject* parent ) : QThread( parent )
{
     moveToThread(this);
}


FakeServer::~FakeServer()
{
  quit();
  wait();
}

void FakeServer::startAndWait()
{
  start();
  // this will block until the event queue starts
  QMetaObject::invokeMethod( this, "started", Qt::BlockingQueuedConnection );
}

void FakeServer::dataAvailable()
{
    QMutexLocker locker(&m_mutex);

    QTcpSocket *socket = qobject_cast<QTcpSocket*>( sender() );
    Q_ASSERT( socket!=0 );

    int scenarioNumber = m_clientSockets.indexOf( socket );

    QVERIFY( !m_scenarios[scenarioNumber].isEmpty() );

    readClientPart( scenarioNumber );
    writeServerPart( scenarioNumber );
}

void FakeServer::newConnection()
{
    QMutexLocker locker(&m_mutex);

    m_clientSockets << m_tcpServer->nextPendingConnection();
    connect(m_clientSockets.last(), SIGNAL(readyRead()), this, SLOT(dataAvailable()));
    m_clientParsers << new KIMAP::ImapStreamParser( m_clientSockets.last() );

    QVERIFY( m_clientSockets.size() <= m_scenarios.size() );

    writeServerPart( m_clientSockets.size() - 1 );
}

void FakeServer::run()
{
    m_tcpServer = new QTcpServer();
    if ( !m_tcpServer->listen( QHostAddress( QHostAddress::LocalHost ), 5989 ) ) {
        kFatal() << "Unable to start the server";
    }

    connect(m_tcpServer, SIGNAL(newConnection()), this, SLOT(newConnection()));

    exec();

    qDeleteAll( m_clientParsers );
    qDeleteAll( m_clientSockets );

    delete m_tcpServer;
}

void FakeServer::started()
{
  // do nothing: this is a dummy slot used by startAndWait()
}

void FakeServer::setScenario( const QList<QByteArray> &scenario )
{
    QMutexLocker locker(&m_mutex);

    m_scenarios.clear();
    m_scenarios << scenario;
}

void FakeServer::addScenario( const QList<QByteArray> &scenario )
{
    QMutexLocker locker(&m_mutex);

    m_scenarios << scenario;
}

void FakeServer::addScenarioFromFile( const QString &fileName )
{
  QFile file( fileName );
  file.open( QFile::ReadOnly );

  QList<QByteArray> scenario;

  // When loading from files we never have the authentication phase
  // force jumping directly to authenticated state.
  scenario << preauth();

  while ( !file.atEnd() ) {
    scenario << file.readLine().trimmed();
  }

  file.close();

  addScenario( scenario );
}

bool FakeServer::isScenarioDone( int scenarioNumber ) const
{
  QMutexLocker locker(&m_mutex);

  if ( scenarioNumber < m_scenarios.size() ) {
    return m_scenarios[scenarioNumber].isEmpty();
  } else {
    return true; // Non existent hence empty, right?
  }
}

bool FakeServer::isAllScenarioDone() const
{
  QMutexLocker locker( &m_mutex );

  foreach ( const QList<QByteArray> &scenario, m_scenarios ) {
    if ( !scenario.isEmpty() ) {
      return false;
    }
  }

  return true;
}

void FakeServer::writeServerPart( int scenarioNumber )
{
    QList<QByteArray> scenario = m_scenarios[scenarioNumber];
    QTcpSocket *clientSocket = m_clientSockets[scenarioNumber];

    while ( !scenario.isEmpty()
         && scenario.first().startsWith( "S: " ) ) {
      QByteArray payload = scenario.takeFirst().mid( 3 );
      clientSocket->write( payload + "\r\n" );
    }

    if ( !scenario.isEmpty()
      && scenario.first().startsWith( "X" ) ) {
      scenario.takeFirst();
      clientSocket->close();
    }

    if ( !scenario.isEmpty() ) {
      QVERIFY( scenario.first().startsWith( "C: " ) );
    }

    m_scenarios[scenarioNumber] = scenario;
}

void FakeServer::readClientPart( int scenarioNumber )
{
    QList<QByteArray> scenario = m_scenarios[scenarioNumber];
    KIMAP::ImapStreamParser *clientParser = m_clientParsers[scenarioNumber];

    while ( !scenario.isEmpty()
         && scenario.first().startsWith( "C: " ) ) {
        QByteArray received = "C: "+clientParser->readUntilCommandEnd().trimmed();
        QByteArray expected = scenario.takeFirst();
        QCOMPARE( QString::fromUtf8( received ), QString::fromUtf8( expected ) );
        QCOMPARE( received, expected );
    }

    if ( !scenario.isEmpty() ) {
      QVERIFY( scenario.first().startsWith( "S: " ) );
    }

    m_scenarios[scenarioNumber] = scenario;
}

#include "fakeserver.moc"
