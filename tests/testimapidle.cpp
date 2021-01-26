/**
 * This file is part of the KDE project
 * SPDX-FileCopyrightText: 2009 Kevin Ottens <ervin@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include <QCoreApplication>
#include <QDebug>

#include "capabilitiesjob.h"
#include "closejob.h"
#include "idlejob.h"
#include "loginjob.h"
#include "logoutjob.h"
#include "selectjob.h"
#include "session.h"
#include "sessionuiproxy.h"

using namespace KIMAP;

class UiProxy : public SessionUiProxy
{
public:
    bool ignoreSslError(const KSslErrorUiData &errorData) override
    {
        Q_UNUSED(errorData)
        return true;
    }
};

int main(int argc, char **argv)
{
    QCoreApplication::setApplicationName(QStringLiteral("TestImapIdle"));

    if (argc < 4) {
        qCritical() << "Not enough parameters, expecting: <server> <user> <password>";
    }

    QString server = QString::fromLocal8Bit(argv[1]);
    int port = 143;
    if (server.count(QLatin1Char(':')) == 1) {
        const QStringList lstSplit = server.split(QLatin1Char(':'));
        port = lstSplit.last().toInt();
        server = lstSplit.first();
    }
    QString user = QString::fromLocal8Bit(argv[2]);
    QString password = QString::fromLocal8Bit(argv[3]);

    qDebug() << "Listening:" << server << port << user << password;
    qDebug();

    QCoreApplication app(argc, argv);
    Session session(server, port);
    auto proxy = new UiProxy();
    session.setUiProxy(UiProxy::Ptr(proxy));

    qDebug() << "Logging in...";
    auto login = new LoginJob(&session);
    login->setUserName(user);
    login->setPassword(password);
    login->exec();
    Q_ASSERT_X(login->error() == 0, "LoginJob", login->errorString().toLocal8Bit().constData());
    Q_ASSERT(session.state() == Session::Authenticated);
    qDebug();

    qDebug() << "Asking for capabilities:";
    auto capabilities = new CapabilitiesJob(&session);
    capabilities->exec();
    Q_ASSERT_X(capabilities->error() == 0, "CapabilitiesJob", capabilities->errorString().toLocal8Bit().constData());
    Q_ASSERT(session.state() == Session::Authenticated);
    qDebug() << capabilities->capabilities();
    qDebug();

    Q_ASSERT(capabilities->capabilities().contains(QLatin1String("IDLE")));

    qDebug() << "Selecting INBOX:";
    auto select = new SelectJob(&session);
    select->setMailBox(QStringLiteral("INBOX"));
    select->exec();
    Q_ASSERT_X(select->error() == 0, "SelectJob", select->errorString().toLocal8Bit().constData());
    Q_ASSERT(session.state() == Session::Selected);
    qDebug() << "Flags:" << select->flags();
    qDebug() << "Permanent flags:" << select->permanentFlags();
    qDebug() << "Total Number of Messages:" << select->messageCount();
    qDebug() << "Number of recent Messages:" << select->recentCount();
    qDebug() << "First Unseen Message Index:" << select->firstUnseenIndex();
    qDebug() << "UID validity:" << select->uidValidity();
    qDebug() << "Next UID:" << select->nextUid();
    qDebug();

    qDebug() << "Start idling...";
    auto idle = new IdleJob(&session);
    QObject::connect(idle, SIGNAL(mailBoxStats(KIMAP::IdleJob *, QString, int, int)), idle, SLOT(stop()));
    idle->exec();
    qDebug() << "Idling done for" << idle->lastMailBox() << "message count:" << idle->lastMessageCount() << "recent count:" << idle->lastRecentCount();

    qDebug() << "Closing INBOX:";
    auto close = new CloseJob(&session);
    close->exec();
    Q_ASSERT(session.state() == Session::Authenticated);
    qDebug();

    qDebug() << "Logging out...";
    auto logout = new LogoutJob(&session);
    logout->exec();
    Q_ASSERT_X(logout->error() == 0, "LogoutJob", logout->errorString().toLocal8Bit().constData());
    Q_ASSERT(session.state() == Session::Disconnected);

    return 0;
}
