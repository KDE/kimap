/*
   SPDX-FileCopyrightText: 2009 Andras Mantia <amantia@kde.org>

   SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
   SPDX-FileContributor: Kevin Ottens <kevin@kdab.com>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <QTest>

#include "kimap/loginjob.h"
#include "kimap/session.h"
#include "kimaptest/fakeserver.h"

#include <QTest>

class TestUiProxy : public KIMAP::SessionUiProxy
{
    bool ignoreSslError(const KSslErrorUiData &) override
    {
        return true;
    }
};

class LoginJobTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    void shouldHandleLogin_data()
    {
        QTest::addColumn<QString>("user");
        QTest::addColumn<QString>("password");
        QTest::addColumn<QList<QByteArray>>("scenario");

        QList<QByteArray> scenario;
        scenario << FakeServer::greeting() << "C: A000001 LOGIN \"user\" \"password\""
                 << "S: A000001 OK User logged in";

        QTest::newRow("success") << QStringLiteral("user") << QStringLiteral("password") << scenario;

        scenario.clear();
        scenario << FakeServer::greeting() << "C: A000001 LOGIN \"user_bad\" \"password\""
                 << "S: A000001 NO Login failed: authentication failure";

        QTest::newRow("wrong login") << "user_bad" << QStringLiteral("password") << scenario;

        scenario.clear();
        scenario << FakeServer::greeting() << "C: A000001 LOGIN \"user\" \"aa\\\"bb\\\\cc[dd ee\""
                 << "S: A000001 OK User logged in";

        QTest::newRow("special chars") << QStringLiteral("user") << "aa\"bb\\cc[dd ee" << scenario;

        scenario.clear();
        scenario << FakeServer::preauth();

        QTest::newRow("already authenticated") << QStringLiteral("user") << QStringLiteral("password") << scenario;
    }

    void shouldHandleLogin()
    {
        QFETCH(QString, user);
        QFETCH(QString, password);
        QFETCH(QList<QByteArray>, scenario);

        FakeServer fakeServer;
        fakeServer.setScenario(scenario);
        fakeServer.startAndWait();

        KIMAP::Session *session = new KIMAP::Session(QStringLiteral("127.0.0.1"), 5989);

        KIMAP::LoginJob *login = new KIMAP::LoginJob(session);
        login->setUserName(user);
        login->setPassword(password);
        bool result = login->exec();

        QEXPECT_FAIL("wrong login", "Login with bad user name", Continue);
        QEXPECT_FAIL("already authenticated", "Trying to log on an already authenticated session", Continue);
        QVERIFY(result);

        fakeServer.quit();
        delete session;
    }

    void shouldHandleProxyLogin_data()
    {
        QTest::addColumn<QString>("user");
        QTest::addColumn<QString>("proxy");
        QTest::addColumn<QString>("password");
        QTest::addColumn<QList<QByteArray>>("scenario");

        QList<QByteArray> scenario;
        scenario << FakeServer::greeting() << "C: A000001 AUTHENTICATE PLAIN"
                 << "S: A000001 OK (success)"
                 << "C: A000001 LOGIN \"proxy\" \"user\" \"password\""
                 << "S: A000001 OK User logged in";

        QTest::newRow("success") << QStringLiteral("user") << "proxy" << QStringLiteral("password") << scenario;
    }

    void shouldHandleProxyLogin()
    {
        QFETCH(QString, user);
        QFETCH(QString, proxy);
        QFETCH(QString, password);
        QFETCH(QList<QByteArray>, scenario);

        FakeServer fakeServer;
        fakeServer.setScenario(scenario);
        fakeServer.startAndWait();

        KIMAP::Session *session = new KIMAP::Session(QStringLiteral("127.0.0.1"), 5989);

        KIMAP::LoginJob *login = new KIMAP::LoginJob(session);
        login->setAuthenticationMode(KIMAP::LoginJob::Plain);
        login->setUserName(user);
        login->setAuthorizationName(proxy);
        login->setPassword(password);
        bool result = login->exec();

        QVERIFY(result);

        fakeServer.quit();
        delete session;
    }

    void shouldSaveServerGreeting_data()
    {
        QTest::addColumn<QString>("greeting");
        QTest::addColumn<QList<QByteArray>>("scenario");

        QList<QByteArray> scenario;
        scenario << FakeServer::greeting() << "C: A000001 LOGIN \"user\" \"password\""
                 << "S: A000001 OK Welcome John Smith";

        QTest::newRow("greeting") << "Welcome John Smith" << scenario;

        scenario.clear();
        scenario << FakeServer::greeting() << "C: A000001 LOGIN \"user\" \"password\""
                 << "S: A000001 OK Welcome John Smith (last login: Feb 21, 2010)";

        QTest::newRow("greeting with parenthesis") << "Welcome John Smith (last login: Feb 21, 2010)" << scenario;

        scenario.clear();
        scenario << FakeServer::greeting() << "C: A000001 LOGIN \"user\" \"password\""
                 << "S: A000001 OK";

        QTest::newRow("no greeting") << "" << scenario;

        scenario.clear();
        scenario << FakeServer::greeting() << "C: A000001 LOGIN \"user\" \"password\""
                 << "S: A000001 NO Login failed: authentication failure";

        QTest::newRow("login failed") << "" << scenario;
    }

    void shouldSaveServerGreeting()
    {
        QFETCH(QString, greeting);
        QFETCH(QList<QByteArray>, scenario);

        FakeServer fakeServer;
        fakeServer.setScenario(scenario);
        fakeServer.startAndWait();

        KIMAP::Session *session = new KIMAP::Session(QStringLiteral("127.0.0.1"), 5989);

        KIMAP::LoginJob *login = new KIMAP::LoginJob(session);
        login->setUserName(QStringLiteral("user"));
        login->setPassword(QStringLiteral("password"));
        login->exec();

        QCOMPARE(login->serverGreeting(), greeting);

        fakeServer.quit();
        delete session;
    }

    void shouldUseSsl_data()
    {
        QTest::addColumn<QList<QByteArray>>("scenario");
        QTest::addColumn<int>("serverEncryption");

        {
            QList<QByteArray> scenario;
            scenario << FakeServer::greeting() << "C: A000001 CAPABILITY"
                     << "S: A000001 OK"
                     << "C: A000002 LOGIN \"user\" \"password\""
                     << "S: A000002 OK";

            // SSLv2 support was removed from openssl 1.1
            // QTest::newRow("sslv2") << scenario << static_cast<int>(QSsl::SslV2);

            // FIXME: SSLv3-only server is failing, likely openssl configuration problem
            // QTest::newRow("sslv3") << scenario << static_cast<int>(QSsl::SslV3);

            // AnySslVersion doesn't mean the server can force a specific version (e.g. openssl always starts with a tls12 hello)
            QTest::newRow("any protocol with anyssl version") << scenario << static_cast<int>(QSsl::AnyProtocol);

            QTest::newRow("tlsv10") << scenario << static_cast<int>(QSsl::TlsV1_0);
            QTest::newRow("tlsv11") << scenario << static_cast<int>(QSsl::TlsV1_1);
            QTest::newRow("tlsv12") << scenario << static_cast<int>(QSsl::TlsV1_2);
        }
    }

    void shouldUseSsl()
    {
        QFETCH(QList<QByteArray>, scenario);
        QFETCH(int, serverEncryption);

        FakeServer fakeServer;
        fakeServer.setEncrypted(static_cast<QSsl::SslProtocol>(serverEncryption));
        fakeServer.setScenario(scenario);
        fakeServer.startAndWait();

        KIMAP::Session *session = new KIMAP::Session(QStringLiteral("127.0.0.1"), 5989);

        KIMAP::SessionUiProxy::Ptr uiProxy(new TestUiProxy);
        session->setUiProxy(uiProxy);

        KIMAP::LoginJob *login = new KIMAP::LoginJob(session);
        login->setUserName(QStringLiteral("user"));
        login->setPassword(QStringLiteral("password"));
        login->setEncryptionMode(KIMAP::LoginJob::SSLorTLS);
        QVERIFY(login->exec());

        fakeServer.quit();
        delete session;
    }

    void shouldUseStartTls_data()
    {
        QTest::addColumn<QList<QByteArray>>("scenario");
        QTest::addColumn<bool>("success");

        {
            QList<QByteArray> scenario;
            scenario << FakeServer::greeting() << "C: A000001 CAPABILITY"
                     << "S: * CAPABILITY IMAP4rev1 STARTTLS"
                     << "S: A000001 OK CAPABILITY completed"
                     << "C: A000002 STARTTLS"
                     << "S: A000002 OK"
                     << "C: A000003 CAPABILITY"
                     << "S: * CAPABILITY IMAP4rev1"
                     << "S: A000003 OK CAPABILITY completed"
                     << "C: A000004 LOGIN \"user\" \"password\""
                     << "S: A000004 OK";
            QTest::newRow("STARTTLS supported") << scenario << true;
        }

        {
            QList<QByteArray> scenario;
            scenario << FakeServer::greeting() << "C: A000001 CAPABILITY"
                     << "S: * CAPABILITY IMAP4rev1"
                     << "S: A000001 OK CAPABILITY completed";

            QTest::newRow("STARTTLS not supported") << scenario << false;
        }
    }

    void shouldUseStartTls()
    {
        QFETCH(QList<QByteArray>, scenario);
        QFETCH(bool, success);

        FakeServer fakeServer;
        fakeServer.setEncrypted(QSsl::AnyProtocol);
        fakeServer.setWaitForStartTls(true);
        fakeServer.setScenario(scenario);
        fakeServer.startAndWait();

        KIMAP::Session session(QStringLiteral("127.0.0.1"), 5989);

        KIMAP::SessionUiProxy::Ptr uiProxy(new TestUiProxy);
        session.setUiProxy(uiProxy);

        KIMAP::LoginJob login(&session);
        login.setUserName(QStringLiteral("user"));
        login.setPassword(QStringLiteral("password"));
        login.setEncryptionMode(KIMAP::LoginJob::STARTTLS);
        QCOMPARE(login.exec(), success);

        fakeServer.quit();
    }
};

QTEST_GUILESS_MAIN(LoginJobTest)

#include "loginjobtest.moc"
