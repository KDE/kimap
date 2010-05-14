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

void FakeServer::dataAvailable()
{
    QMutexLocker locker(&m_mutex);

    readClientPart();
    writeServerPart();
}

void FakeServer::newConnection()
{
    QMutexLocker locker(&m_mutex);

    tcpServerConnection = m_tcpServer->nextPendingConnection();
    connect(tcpServerConnection, SIGNAL(readyRead()), this, SLOT(dataAvailable()));
    streamParser = new KIMAP::ImapStreamParser( tcpServerConnection );

    writeServerPart();
}

void FakeServer::run()
{
    m_tcpServer = new QTcpServer();
    if ( !m_tcpServer->listen( QHostAddress( QHostAddress::LocalHost ), 5989 ) ) {
        kFatal() << "Unable to start the server";
    }

    connect(m_tcpServer, SIGNAL(newConnection()), this, SLOT(newConnection()));

    exec();
    disconnect(tcpServerConnection, SIGNAL(readyRead()), this, SLOT(dataAvailable()));
    delete m_tcpServer;
}

void FakeServer::setScenario( const QList<QByteArray> &scenario )
{
    QMutexLocker locker(&m_mutex);

    m_scenario.clear();
    m_scenario+= scenario;
}

void FakeServer::writeServerPart()
{
    while ( !m_scenario.isEmpty()
         && m_scenario.first().startsWith( "S: " ) ) {
      QByteArray payload = m_scenario.takeFirst().mid( 3 );
      tcpServerConnection->write( payload + "\r\n" );
    }

    if ( !m_scenario.isEmpty() ) {
      QVERIFY( m_scenario.first().startsWith( "C: " ) );
    }
}

void FakeServer::readClientPart()
{
    while ( !m_scenario.isEmpty()
         && m_scenario.first().startsWith( "C: " ) ) {
        QByteArray data = streamParser->readUntilCommandEnd();
        QCOMPARE( "C: "+data.trimmed(), m_scenario.takeFirst() );
    }

    if ( !m_scenario.isEmpty() ) {
      QVERIFY( m_scenario.first().startsWith( "S: " ) );
    }
}

#include "fakeserver.moc"
