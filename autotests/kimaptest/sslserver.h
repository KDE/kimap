/*
   SPDX-FileCopyrightText: 2013 Christian Mollekopf <mollekopf@kolabsys.com>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SSLSERVER_H
#define SSLSERVER_H

#include <QTcpServer>
#include <QSslSocket>

class SslServer: public QTcpServer
{
    Q_OBJECT
public:
    SslServer(QSsl::SslProtocol, bool waitForStartTls);
    void incomingConnection(qintptr handle) override;

private Q_SLOTS:
    void sslErrors(const QList<QSslError> &errors);
    void error(QAbstractSocket::SocketError);

private:
    QSsl::SslProtocol mProtocol;
    QSslSocket mSocket;
    bool mWaitForStartTls = false;
};

#endif
