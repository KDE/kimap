/*
    SPDX-FileCopyrightText: 2009 Kevin Ottens <ervin@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "logoutjob.h"

#include <KLocalizedString>

#include "job_p.h"
#include "response_p.h"
#include "session_p.h"

namespace KIMAP
{
class LogoutJobPrivate : public JobPrivate
{
public:
    LogoutJobPrivate(Session *session, const QString &name)
        : JobPrivate(session, name)
    {
    }
    ~LogoutJobPrivate()
    {
    }
};
}

using namespace KIMAP;

LogoutJob::LogoutJob(Session *session)
    : Job(*new LogoutJobPrivate(session, i18n("Logout")))
{
}

LogoutJob::~LogoutJob()
{
}

void LogoutJob::doStart()
{
    Q_D(LogoutJob);
    d->tags << d->sessionInternal()->sendCommand("LOGOUT");
}

void LogoutJob::connectionLost()
{
    emitResult();
}
