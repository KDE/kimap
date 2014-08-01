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
#include "sslserver.h"

#include <QFile>
#include <QSslKey>

#include <QDebug>

static QByteArray staticCert()
{
    //a dummy certificate
    return QByteArray(
               "-----BEGIN CERTIFICATE-----\n\
MIIB+zCCAWQCCQDBBi7xZ2944DANBgkqhkiG9w0BAQUFADBCMQswCQYDVQQGEwJY\n\
WDEVMBMGA1UEBwwMRGVmYXVsdCBDaXR5MRwwGgYDVQQKDBNEZWZhdWx0IENvbXBh\n\
bnkgTHRkMB4XDTEzMTIwNTA5MDcxNVoXDTQxMDQyMjA5MDcxNVowQjELMAkGA1UE\n\
BhMCWFgxFTATBgNVBAcMDERlZmF1bHQgQ2l0eTEcMBoGA1UECgwTRGVmYXVsdCBD\n\
b21wYW55IEx0ZDCBnzANBgkqhkiG9w0BAQEFAAOBjQAwgYkCgYEAyuZdeqTgzX2E\n\
Q+tOj8/QzT8jHOUvwleqv56hAOEbZ5pLhYPesaSqV0lADiYHKjCRVIrhJQXePf7y\n\
MrJ3zE6hbHEMoIj+ku6ttNQkfJif30wmbXxLXO+RqraYgJW730kcbi2Jyq7ciEC1\n\
SVeiIaaiV2yUFBc/ARDFBc7733Y053UCAwEAATANBgkqhkiG9w0BAQUFAAOBgQAE\n\
BmB+mGtQzdmOAPbRYegA2ybuUARnW467qMOQpj5dV2LN+bizCbqrsz2twFKWS7oK\n\
EiC1bd6EGHnF6inFksUwODqeb+rjQ85pFBWskG51LWvX2/hoS+0x2V37vUYMxnDH\n\
rOEQiDe3oerErB0x9FMWk7VivEqO5HGEdxy7fGl3vg==\n\
-----END CERTIFICATE-----");
}

static QByteArray staticKey()
{
    //a dummy key without password
    return QByteArray(
               "-----BEGIN RSA PRIVATE KEY-----\n\
MIICXgIBAAKBgQDK5l16pODNfYRD606Pz9DNPyMc5S/CV6q/nqEA4RtnmkuFg96x\n\
pKpXSUAOJgcqMJFUiuElBd49/vIysnfMTqFscQygiP6S7q201CR8mJ/fTCZtfEtc\n\
75GqtpiAlbvfSRxuLYnKrtyIQLVJV6IhpqJXbJQUFz8BEMUFzvvfdjTndQIDAQAB\n\
AoGBAIdNfXLGtl5x4BzGspn2NEBaZRjkwKdxfJzRtH34nyTEYK5FVODTdQBGCaAl\n\
vctlndRp1F+y/RQMighCuN6WZM/SdkzxkGGJVzDDuMw0Cwc48aqtMA3A3x/3bQkK\n\
kk2A5sLBc1TuC4DYSP5zkoXDbvBsHHN+tGAaC348Df6of1J1AkEA7ye3W9JUN4uK\n\
2cPrnh7EKwQ2pFypeE/UNQ+LXR9h8XK90mxwiShU9sRFlNIA+pPcZ2aBxoY9m3rJ\n\
4GHitl4ajwJBANkw6xM9IdgjMD8OQonpZTHSrKki/MaSSe9eBJ+WiCkTKL+Y9aTm\n\
28sU7I+j3V38kYf5zyWXkyWmmNaQ4VaU77sCQQDMx7BM0qPEUBtb7lQxt9x3jQsQ\n\
4DtIxupJaP8HhRjDu2Fo7evKthtSlauTC+NErRl7/J1BFa9pE9IK7SZIy/lnAkB6\n\
ssga9k5IbJi1BrlQcCpbG0mvw7RJ+hsKv3KdNc12Zvx+QUuE/WbuM8Pw4gINNsKA\n\
rv/3nMnkW1m83dxvrXRBAkEAmy3nO9HXdcKtSseseLJ6h9RB/R0XE1EvG8CKvveS\n\
IiVUnppVeiLFa7ItwHOovgqvWVbePd5xl6+yBGxUXznjWA==\n\
-----END RSA PRIVATE KEY-----");
}

SslServer::SslServer(QSsl::SslProtocol protocol)
    : QTcpServer(),
      mProtocol(protocol)
{

}

void SslServer::incomingConnection(int handle)
{
    QSslSocket *socket = new QSslSocket();
    socket->setSocketDescriptor(handle);

    socket->setProtocol(mProtocol);

    QSslKey ssl_key(staticKey(), QSsl::Rsa);
    QSslCertificate ssl_cert(staticCert());
    Q_ASSERT(ssl_cert.isValid());

    socket->setPrivateKey(ssl_key);
    socket->setLocalCertificate(ssl_cert);
    socket->setCaCertificates(QList<QSslCertificate>() << ssl_cert);
    socket->setPeerVerifyMode(QSslSocket::VerifyNone);
    socket->ignoreSslErrors();
    connect(socket, SIGNAL(sslErrors(QList<QSslError>)), this, SLOT(sslErrors(QList<QSslError>)));
    connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(error(QAbstractSocket::SocketError)));
    if (mProtocol != QSsl::TlsV1) {
        socket->startServerEncryption();
    }
    addPendingConnection(socket);
}

void SslServer::sslErrors(const QList<QSslError> &errors)
{
    foreach (const QSslError &error, errors) {
        qWarning() << "Received ssl error: " << error.errorString();
    }
    QSslSocket *socket = qobject_cast<QSslSocket *>(QObject::sender());
    if (socket) {
        socket->disconnectFromHost();
    }
}

void SslServer::error(QAbstractSocket::SocketError error)
{
    QSslSocket *socket = qobject_cast<QSslSocket *>(QObject::sender());
    if (socket) {
        qWarning() << socket->errorString();
    }
    qWarning() << error;
}
