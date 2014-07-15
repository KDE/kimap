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
#include <QSsl>

namespace KIMAP
{
  class ImapStreamParser;
}

Q_DECLARE_METATYPE( QList<QByteArray> )

/**
 * Pretends to be an IMAP server for the purposes of unit tests.
 *
 * FakeServer does not really understand the IMAP protocol.  Instead,
 * you give it a script, or scenario, that lists how an IMAP session
 * exchange should go.  When it receives the client parts of the
 * scenario, it will respond with the following server parts.
 *
 * The server can be furnished with several scenarios.  The first
 * scenario will be played out to the first client that connects, the
 * second scenario to the second client connection and so on.
 *
 * The fake server runs as a separate thread in the same process it
 * is started from, and listens for connections on port 5989 on the
 * local machine.
 *
 * Scenarios are in the form of protocol messages, with a tag at the
 * start to indicate whether it is message that will be sent by the
 * client ("C:") or a response that should be sent by the server
 * ("S:").  For example:
 * @code
 * C: A000001 LIST "" *
 * S: * LIST ( \HasChildren ) / INBOX
 * S: * LIST ( \HasNoChildren ) / INBOX/&AOQ- &APY- &APw- @ &IKw-
 * S: * LIST ( \HasChildren ) / INBOX/lost+found
 * S: * LIST ( \HasNoChildren ) / "INBOX/lost+found/Calendar Public-20080128"
 * S: A000001 OK LIST completed
 * @endcode
 *
 * A line starting with X indicates that the connection should be
 * closed by the server.  This should be the last line in the
 * scenario.  For example, the following simulates the server closing
 * the connection after receiving too many bad commands:
 * @code
 * C: A000001 madhatter
 * S: A000001 BAD Command madhatter
 * X
 * @endcode
 *
 * FakeServer::preauth() and FakeServer::greeting() provide standard
 * PREAUTH and OK responses, respectively, that can be used (unmodified)
 * as the first line of a scenario.
 *
 * A typical usage is something like
 * @code
 * QList<QByteArray> scenario;
 * scenario << FakeServer::preauth()
 *          << "C: A000001 CAPABILITY"
 *          << "S: * CAPABILITY IMAP4rev1 STARTTLS AUTH=GSSAPI"
 *          << "S: A000001 OK CAPABILITY completed";
 *
 * FakeServer fakeServer;
 * fakeServer.setScenario( scenario );
 * fakeServer.startAndWait();
 *
 * KIMAP::Session session( QLatin1String("127.0.0.1"), 5989 );
 * KIMAP::CapabilitiesJob *job = new KIMAP::CapabilitiesJob(&session);
 * QVERIFY( job->exec() );
 * // check the returned capabilities
 *
 * fakeServer.quit();
 * @endcode
 */
class FakeServer : public QThread
{
    Q_OBJECT

public:
    /**
     * Get the default PREAUTH response
     *
     * This is the initial PREAUTH message that the server
     * sends at the start of a session to indicate that the
     * user is already authenticated by some other mechanism.
     *
     * Can be used as the first line in a scenario where
     * you want to skip the LOGIN stage of the protocol.
     */
    static QByteArray preauth();
    /**
     * Get the default greeting
     *
     * This is the initial OK message that the server sends at the
     * start of a session to indicate that a LOGIN is required.
     *
     * Can be used as the first line in a scenario where
     * you want to use the LOGIN command.
     */
    static QByteArray greeting();

    FakeServer( QObject* parent = 0 );
    ~FakeServer();

    /**
     * Sets the encryption mode used by the server socket.
     */
    void setEncrypted( QSsl::SslProtocol protocol );

    /**
     * Starts the server and waits for it to be ready
     *
     * You should use this instead of start() to avoid race conditions.
     */
    void startAndWait();

    /**
     * Starts the fake IMAP server
     *
     * You should not call this directly.  Use start() instead.
     *
     * @reimp
     */
    virtual void run();

    /**
     * Removes any previously-added scenarios, and adds a new one
     *
     * After this, there will only be one scenario, and so the fake
     * server will only be able to service a single request.  More
     * scenarios can be added with addScenario, though.
     *
     * @see addScenario()\n
     * addScenarioFromFile()
     */
    void setScenario( const QList<QByteArray> &scenario );

    /**
     * Adds a new scenario
     *
     * Note that scenarios will be used in the order that clients
     * connect.  If this is the 5th scenario that has been added
     * (bearing in mind that setScenario() resets the scenario
     * count), it will be used to service the 5th client that
     * connects.
     *
     * @see addScenarioFromFile()
     *
     * @param scenario  the scenario as a list of messages
     */
    void addScenario( const QList<QByteArray> &scenario );
    /**
     * Adds a new scenario from a local file
     *
     * Note that scenarios will be used in the order that clients
     * connect.  If this is the 5th scenario that has been added
     * (bearing in mind that setScenario() resets the scenario
     * count), it will be used to service the 5th client that
     * connects.
     *
     * @see addScenario()
     *
     * @param fileName  the name of the file that contains the
     *                  scenario; it will be split at line
     *                  boundaries, and excess whitespace will
     *                  be trimmed from the start and end of lines
     */
    void addScenarioFromFile( const QString &fileName );

    /**
     * Checks whether a particular scenario has completed
     *
     * @param scenarioNumber  the number of the scenario to check,
     *                        in order of addition/client connection
     */
    bool isScenarioDone( int scenarioNumber ) const;
    /**
     * Whether all the scenarios that were added to the fake
     * server have been completed.
     */
    bool isAllScenarioDone() const;

protected:
    /**
     * Whether the received content is the same as the expected.
     * Use QCOMPARE, if creating subclasses.
     */
    virtual void compareReceived(const QByteArray& received, const QByteArray& expected) const;

private Q_SLOTS:
    void newConnection();
    void dataAvailable();
    void started();

private:
    void writeServerPart( int scenarioNumber );
    void readClientPart( int scenarioNumber );

    QList< QList<QByteArray> > m_scenarios;
    QTcpServer *m_tcpServer;
    mutable QMutex m_mutex;
    QList<QTcpSocket*> m_clientSockets;
    QList<KIMAP::ImapStreamParser*> m_clientParsers;
    bool m_encrypted;
    bool m_starttls;
    QSsl::SslProtocol m_sslProtocol;
};

#endif
