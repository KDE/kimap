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

namespace KIMAP {

class SessionPrivate;
class JobPrivate;
class Message;

class KIMAP_EXPORT Session : public QObject
{
  Q_OBJECT
  Q_ENUMS(State)

  friend class JobPrivate;

  public:
    enum State { Disconnected = 0, NotAuthenticated, Authenticated, Selected };

    explicit Session( const QString &hostName, quint16 port, QObject *parent=0 );
    ~Session();

    QString hostName() const;
    quint16 port() const;
    State state() const;

  private:
    Q_PRIVATE_SLOT( d, void doStartNext() )
    Q_PRIVATE_SLOT( d, void jobDone( KJob* ) )
    Q_PRIVATE_SLOT( d, void jobDestroyed( QObject* ) )
    Q_PRIVATE_SLOT( d, void responseReceived( const KIMAP::Message& ) )

    Q_PRIVATE_SLOT( d, void socketDisconnected() )
    Q_PRIVATE_SLOT( d, void socketError() )

    SessionPrivate *const d;
};

}

#endif
