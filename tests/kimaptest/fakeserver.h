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


#ifndef FAKESERVER_H
#define FAKESERVER_H

#include <QStringList>
#include <QTcpSocket>
#include <QTcpServer>
#include <QThread>
#include <QMutex>

namespace KIMAP
{
  class ImapStreamParser;
}

Q_DECLARE_METATYPE( QList<QByteArray> )

class FakeServer : public QThread
{
    Q_OBJECT

public:
    static QByteArray preauth();
    static QByteArray greeting();

    FakeServer( QObject* parent = 0 );
    ~FakeServer();
    virtual void run();

    void setScenario( const QList<QByteArray> &scenario );

    void addScenario( const QList<QByteArray> &scenario );
    void addScenarioFromFile( const QString &fileName );

    bool isScenarioDone( int scenarioNumber ) const;
    bool isAllScenarioDone() const;

private Q_SLOTS:
    void newConnection();
    void dataAvailable();

private:
    void writeServerPart( int scenarioNumber );
    void readClientPart( int scenarioNumber );

    QList< QList<QByteArray> > m_scenarios;
    QTcpServer *m_tcpServer;
    mutable QMutex m_mutex;
    QList<QTcpSocket*> m_clientSockets;
    QList<KIMAP::ImapStreamParser*> m_clientParsers;
};

#endif

