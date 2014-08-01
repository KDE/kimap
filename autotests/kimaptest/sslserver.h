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

#ifndef SSLSERVER_H
#define SSLSERVER_H

#include <QTcpServer>
#include <QSslSocket>

class SslServer: public QTcpServer
{
    Q_OBJECT
public:
    SslServer(QSsl::SslProtocol);
    virtual void incomingConnection(int handle);

private slots:
    void sslErrors(const QList<QSslError> &errors);
    void error(QAbstractSocket::SocketError);

private:
    QSsl::SslProtocol mProtocol;
    QSslSocket mSocket;
};

#endif
