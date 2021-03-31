/*
    SPDX-FileCopyrightText: 2009 Kevin Ottens <ervin@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "kimap_export.h"

#include <QObject>

#include "sessionuiproxy.h"

namespace KIMAP
{
class SessionPrivate;
class JobPrivate;
struct Response;

class KIMAP_EXPORT Session : public QObject
{
    Q_OBJECT

    friend class JobPrivate;

public:
    enum State { Disconnected = 0, NotAuthenticated, Authenticated, Selected };
    Q_ENUM(State)
    Session(const QString &hostName, quint16 port, QObject *parent = nullptr);
    ~Session();

    Q_REQUIRED_RESULT QString hostName() const;
    Q_REQUIRED_RESULT quint16 port() const;
    Q_REQUIRED_RESULT State state() const;

    /**
     * Returns the name that has been set with LoginJob::setUserName()
     * The user name is useful to uniquely identify an IMAP resource, in combination with the host name
     * @note If the Session was pre-authenticated, userName() will return an empty string
     * @since 4.7
     */
    Q_REQUIRED_RESULT QString userName() const;

    Q_REQUIRED_RESULT QByteArray serverGreeting() const;

    /**
     * Sets an ui proxy that displays the error messages and waits for user feedback.
     * @param proxy the ui proxy object
     */
    void setUiProxy(const SessionUiProxy::Ptr &proxy);

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
    Q_REQUIRED_RESULT int timeout() const;

    /**
     * Returns the currently selected mailbox.
     * @since 4.5
     */
    Q_REQUIRED_RESULT QString selectedMailBox() const;

    /**
     * Sets whether the IMAP network connection should use the system proxy settings.
     *
     * @param useProxy @c true if the proxy is to be used
     * The default is to not use the proxy.
     * @since 5.11.41
     *
     * @note If the session is currently connected to the IMAP server, calling this
     * function will disconnect and reconnect to it with the changed proxy setting.
     */
    void setUseNetworkProxy(bool useProxy);

    Q_REQUIRED_RESULT int jobQueueSize() const;

    void close();

Q_SIGNALS:
    void jobQueueSizeChanged(int queueSize);

    /**
      Emitted when we lose a previously established connection

      Likely reasons: server closed the connection, loss of internet connectivity, etc...
    */
    void connectionLost();

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

      Not very useful after all... :-)

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

