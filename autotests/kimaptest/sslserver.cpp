/*
   SPDX-FileCopyrightText: 2013 Christian Mollekopf <mollekopf@kolabsys.com>

   SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "sslserver.h"

#include <QDebug>
#include <QSslConfiguration>
#include <QSslKey>

// Generated on 2020-10-07 using command:
// openssl req -nodes -new -x509 -keyout key.pem -out cert.pem -days 36500
//
// The cert should be valid until 2120.

static QByteArray staticCert()
{
    // a dummy certificate
    return QByteArray(
        "-----BEGIN CERTIFICATE-----\n\
MIIDZzCCAk+gAwIBAgIUQoBjjbd//7DD9zWfru/epnVT2vAwDQYJKoZIhvcNAQEL\n\
BQAwQjELMAkGA1UEBhMCWFgxFTATBgNVBAcMDERlZmF1bHQgQ2l0eTEcMBoGA1UE\n\
CgwTRGVmYXVsdCBDb21wYW55IEx0ZDAgFw0yMDEwMDcxMzMxMTJaGA8yMTIwMDkx\n\
MzEzMzExMlowQjELMAkGA1UEBhMCWFgxFTATBgNVBAcMDERlZmF1bHQgQ2l0eTEc\n\
MBoGA1UECgwTRGVmYXVsdCBDb21wYW55IEx0ZDCCASIwDQYJKoZIhvcNAQEBBQAD\n\
ggEPADCCAQoCggEBAMIu0Osija2bIINdCjuP0T+tdlQOI8l52J+z/dI45MxecEn5\n\
LAsYWhqanpgZSM5tFf4xqWaX/1/TVDB5JiLx3voWOODTtNKzFVFCzfKHuH6cBboE\n\
E4TZ+H6hTq+YyMKUDN7XqRtBIf1FBf0lMhalWdtEDtlXcsuoaGXkr671JwR34+EU\n\
6YH/8kbBQQBoL640/gxtgevGmpqfqLW0/hogF5pOhMOfwVlYo28IqaPScUGEpopf\n\
zqL+z6NwdEsNLygwq21kU0hhRnx2UgdPcZNX3dc7sUkZjxn53t6XPeVvgGXCte1o\n\
Ad6L0rMunlda+Tj02FXn7jo6G7ELj05Bg8JNB8sCAwEAAaNTMFEwHQYDVR0OBBYE\n\
FLi6xsBAkbc06veA7t+8slAUCVebMB8GA1UdIwQYMBaAFLi6xsBAkbc06veA7t+8\n\
slAUCVebMA8GA1UdEwEB/wQFMAMBAf8wDQYJKoZIhvcNAQELBQADggEBAFpcIzOl\n\
iqMK/kre/xJyE+S1UoS85rmegZDkNdEXUS6zCozL+eO2NxHxumxfD3qrdwLvXfn0\n\
y30xUlNecB30/egXrel9z7Xop70sVCfgfb/vfMmQ5KSUZC8968rgHPl5qPmjZk6X\n\
z0M9hktTRpIk43DPuz5k1qD+LsmrKzI7twGu00EenwVZa4eLN36Ju4ZRg3I0h2zH\n\
vAnuAku0F0JU9GXdbjOW/248k+jhMP1noDaLqwVkkjgzaJwHxFNiKxnG3f7anneK\n\
HxybczmNl3a6IsZN0CSUSwLfvr1Wezk2Mb7dwzhPbycP5icB/KXWgApkRZ3MKfLN\n\
aBvBMCfr7GXmLQk=\n\
-----END CERTIFICATE-----");
}

static QByteArray staticKey()
{
    // a dummy key without password
    return QByteArray(
        "-----BEGIN PRIVATE KEY-----\n\
MIIEvQIBADANBgkqhkiG9w0BAQEFAASCBKcwggSjAgEAAoIBAQDCLtDrIo2tmyCD\n\
XQo7j9E/rXZUDiPJedifs/3SOOTMXnBJ+SwLGFoamp6YGUjObRX+Malml/9f01Qw\n\
eSYi8d76Fjjg07TSsxVRQs3yh7h+nAW6BBOE2fh+oU6vmMjClAze16kbQSH9RQX9\n\
JTIWpVnbRA7ZV3LLqGhl5K+u9ScEd+PhFOmB//JGwUEAaC+uNP4MbYHrxpqan6i1\n\
tP4aIBeaToTDn8FZWKNvCKmj0nFBhKaKX86i/s+jcHRLDS8oMKttZFNIYUZ8dlIH\n\
T3GTV93XO7FJGY8Z+d7elz3lb4BlwrXtaAHei9KzLp5XWvk49NhV5+46OhuxC49O\n\
QYPCTQfLAgMBAAECggEAFdV/xkCfX5rmv/6RLPtR0vAlMvRYD8p0khiB/MZK67Pe\n\
umKbEjBNyVoDZSTKq/NWGrvoyVFj9JlliQHGSP2bTdF72cPUPDDIv9dcIt4h35KZ\n\
jj3xF+oOZ+apW1M5l69f9xqWuCrNsJW3Q6yQAj3baa0wYj3T+ZRZEMPzASAtn/HM\n\
8r/ToZ9jzObVeNAKoHy30bn5a5NwUcpdOfj84l/qJalXEMtR05FTOa+UEjbqpxjM\n\
+HR9RhlLyxXoFkBjtBowEWZcJSik7t5Vpk4brhUZTVxR1bC5zAXxtnDg2SmR9bKO\n\
4wttLEtTTrXeTkxpYMTZWt7FplUmP92RdcW8PCbe2QKBgQDl+QYUhaQdmRC1ylny\n\
2XP0lvPBHQr7jIHYcsGgit7XKMu6wSq/NGuLNT7WL1A9UDWBCdsdqXBFjR3rwhzh\n\
krbymy1H6PGPYYMQg0DeoAG7I1YTYhrp25bO0WTXVyYrhbZiQkFontUFYIbqu+Rf\n\
glVnhEXXN6wXBjY1F0qM4KjydQKBgQDYKNspAaxX/GxnydtlxXTXWSKD/kMlbAeu\n\
+AI9PRn6LckT7pKEE4Qd/gHnrlnxlqgLTJDcGgTeNutHmaHToQNSWiNltr7igzsU\n\
nABUwlNpl2NwUqttCvsGcyxhabDIjO6Iw+jbnLsWwyMzwf/5YaP8wwmznWYDGn6Y\n\
UXbyXn5JPwKBgQCzHM/qfefDkaqdG/wQk+KnFkbFGnyqTNX4ofBCvMYwp7p9OuOz\n\
Rf2Yz6CgKvuAtY7mcKmzIXuq/+zU6TubSKyqqceLoVc6iAUPgFquycpvWWc584fo\n\
qjl73USAH4VAEoVpZBcTh9l7taF/A1YsORORa0kGBXtH93OSyBAtRvDu3QKBgHZm\n\
2rU5f2eqks6/GPhEEl4rKubWDX2gEQ1cOA3HPEV3ct8CHStPbVzoV67KJ+ZXObEG\n\
vYpyjhwSRQoxMx0y4xjm2uDKGwEEFQaS9PN4hiweio9qGRBfpWTBDZxGQll4KOit\n\
Nw2kai5rsQqWx1mYjDTVuKi0HL6ycomIhfj4nYANAoGATXXc8Ekg+w7hfzLS3BTV\n\
SovH/d6/fC5B7nD5+wB7s0dD8NqqOUKvRm/HPRzMWejSOMXWQzpWSAGTheaFArTw\n\
2k6FDQ7S1lEt6jbVBtS189LtaZBnEWZQfyjKl8h6lJyVU2kBfaYoEOO31T8n20sG\n\
NdkmACeCyNMs5V3yq1vAeN4=\n\
-----END PRIVATE KEY-----");
}

SslServer::SslServer(QSsl::SslProtocol protocol, bool waitForStartTls)
    : QTcpServer()
    , mProtocol(protocol)
    , mWaitForStartTls(waitForStartTls)
{
}

void SslServer::incomingConnection(qintptr handle)
{
    auto socket = new QSslSocket();
    socket->setSocketDescriptor(handle);

    socket->setProtocol(mProtocol);

    QSslKey ssl_key(staticKey(), QSsl::Rsa);
    QSslCertificate ssl_cert(staticCert());
    Q_ASSERT(QDateTime::currentDateTime() >= ssl_cert.effectiveDate());
    Q_ASSERT(QDateTime::currentDateTime() <= ssl_cert.expiryDate());
    Q_ASSERT(!ssl_cert.isBlacklisted());

    socket->setPrivateKey(ssl_key);
    socket->setLocalCertificate(ssl_cert);
    socket->sslConfiguration().addCaCertificates(QList<QSslCertificate>() << ssl_cert);
    socket->setPeerVerifyMode(QSslSocket::VerifyNone);
    socket->ignoreSslErrors();
    connect(socket, SIGNAL(sslErrors(QList<QSslError>)), this, SLOT(sslErrors(QList<QSslError>)));
    connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(error(QAbstractSocket::SocketError)));
    if (!mWaitForStartTls) {
        socket->startServerEncryption();
    }

    addPendingConnection(socket);
}

void SslServer::sslErrors(const QList<QSslError> &errors)
{
    for (const QSslError &error : errors) {
        qWarning() << "Received ssl error: " << error.errorString();
    }
    auto socket = qobject_cast<QSslSocket *>(QObject::sender());
    if (socket) {
        socket->disconnectFromHost();
    }
}

void SslServer::error(QAbstractSocket::SocketError error)
{
    auto socket = qobject_cast<QSslSocket *>(QObject::sender());
    if (socket) {
        qWarning() << socket->errorString();
    }
    qWarning() << error;
}
