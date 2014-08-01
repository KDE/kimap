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

#include "logoutjob.h"

#include <KLocalizedString>

#include "job_p.h"
#include "message_p.h"
#include "session_p.h"

namespace KIMAP
{
class LogoutJobPrivate : public JobPrivate
{
public:
    LogoutJobPrivate(Session *session, const QString &name) : JobPrivate(session, name) { }
    ~LogoutJobPrivate() { }
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
