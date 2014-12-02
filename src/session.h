/*
    Copyright (c) 2009 Kevin Ottens <ervin@kde.org>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

#ifndef KIMAP_SESSION_H
#define KIMAP_SESSION_H

#include "kimap_export.h"

#include <QtCore/QObject>

#include "sessionuiproxy.h"


namespace KIMAP
{

class SessionPrivate;
class JobPrivate;
struct Message;

class KIMAP_EXPORT Session : public QObject
{
    Q_OBJECT
    Q_ENUMS(State)

    friend class JobPrivate;

public:
    enum State { Disconnected = 0, NotAuthenticated, Authenticated, Selected };

    Session(const QString &hostName, quint16 port, QObject *parent = Q_NULLPTR);
    ~Session();

    QString hostName() const;
    quint16 port() const;
    State state() const;

    /**
     * Returns the name that has been set with LoginJob::setUserName()
     * The user name is useful to uniquely identify an IMAP resource, in combination with the host name
     * @note If the Session was pre-authenticated, userName() will return an empty string
     * @since 4.7
     */
    QString userName() const;

    QByteArray serverGreeting() const;

    /**
     * Sets an ui proxy that displays the error messages and waits for user feedback.
     * @param proxy the ui proxy object
     */
    void setUiProxy(SessionUiProxy::Ptr proxy);

    /**
     * Sets an ui proxy that displays the error messages and waits for user feedback.
     * @param proxy the ui proxy object
     * @deprecated Use the shared pointer version instead
     */
    KIMAP_DEPRECATED void setUiProxy(SessionUiProxy *proxy);

    /**
     * Set the session timeout. The default is 30 seconds.
     * @param timeout The socket timeout in seconds, negative values disable the timeout.
     * @since 4.6
     */
    void setTimeout(int timeout);

    /**
     * Returns the session timeout.
     * @since 4.12
     */
    int timeout() const;

    /**
     * Returns the currently selected mailbox.
     * @since 4.5
     */
    QString selectedMailBox() const;

    int jobQueueSize() const;

    void close();

Q_SIGNALS:
    void jobQueueSizeChanged(int queueSize);

    /**
      @deprecated
      Emitted when we loose a previously established connection

      Likely reasons: server closed the connection, loss of internet connectivity, etc...

      For historical reasons, this signal is also emitted in the event of a failed connection, but
      you should not rely on this behavior.

      New implementations should use connectionFailed() to detect a failure to connect to the host,
      and stateChanged() to detect a loss of connectivity.
    */
    KIMAP_DEPRECATED void connectionLost();

    /**
      Emitted when the Session couldn't connect to the host.

      Likely reasons: invalid host address, no internet connectivity, firewall blocking rules,
      etc...

      Pending jobs in the queue will be deleted, and the first job in the queue will be failed. (ie:
      it will have its result signal emitted with a non-zero error code.)

      @since 4.7
    */
    void connectionFailed();

    /**
      Emitted when the session's state changes.

      You can use this signal to detect a connection loss (ie: stateChanged is emitted with newState
      == KIMAP::Session::Disconnected)

      If you want to receive the stateChanged arguments in your slot, you must register the State
      enum with @c Q_DECLARE_METATYPE(KIMAP::Session::State) and @c qRegisterMetaType<KIMAP::Session::State>();

      @since 4.7
    */
    void stateChanged(KIMAP::Session::State newState, KIMAP::Session::State oldState);

private:
    friend class SessionPrivate;
    SessionPrivate *const d;
};

}

#endif
