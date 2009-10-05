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

class KSslErrorUiData;

namespace KIMAP {

class SessionUiProxy;
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

    Session( const QString &hostName, quint16 port, QObject *parent=0 );
    ~Session();

    QString hostName() const;
    quint16 port() const;
    State state() const;

    QByteArray serverGreeting() const;

    /**
     * Sets an ui proxy that displays the error messages and waits for user feedback.
     * @param proxy the ui proxy object
     */
    void setUiProxy(SessionUiProxy *proxy);

  private:
    Q_PRIVATE_SLOT( d, void doStartNext() )
    Q_PRIVATE_SLOT( d, void jobDone( KJob* ) )
    Q_PRIVATE_SLOT( d, void jobDestroyed( QObject* ) )
    Q_PRIVATE_SLOT( d, void responseReceived( const KIMAP::Message& ) )

    Q_PRIVATE_SLOT( d, void socketConnected() )
    Q_PRIVATE_SLOT( d, void socketDisconnected() )
    Q_PRIVATE_SLOT( d, void socketError() )

    Q_PRIVATE_SLOT( d, void handleSslError( const KSslErrorUiData &errorData ) )

    friend class SessionPrivate;
    SessionPrivate *const d;
};

}

#endif
