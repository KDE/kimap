/*
    SPDX-FileCopyrightText: 2009 Kevin Ottens <ervin@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "job.h"
#include "job_p.h"
#include "response_p.h"
#include "session_p.h"

#include "kimap_debug.h"
#include <KLocalizedString>

using namespace KIMAP;

Job::Job(Session *session)
    : KJob(session)
    , d_ptr(new JobPrivate(session, i18n("Job")))
{
}

Job::Job(JobPrivate &dd)
    : KJob(dd.m_session)
    , d_ptr(&dd)
{
}

Job::~Job()
{
    delete d_ptr;
}

Session *Job::session() const
{
    Q_D(const Job);
    return d->m_session;
}

void Job::start()
{
    Q_D(Job);
    d->sessionInternal()->addJob(this);
}

void Job::handleResponse(const Response &response)
{
    handleErrorReplies(response);
}

void Job::connectionLost()
{
    setError(KJob::UserDefinedError);
    setErrorText(i18n("Connection to server lost."));
    emitResult();
}

Job::HandlerResponse Job::handleErrorReplies(const Response &response)
{
    Q_D(Job);
    //   qCDebug(KIMAP_LOG) << response.toString();

    if (!response.content.isEmpty() && d->tags.contains(response.content.first().toString())) {
        if (response.content.size() < 2) {
            setErrorText(i18n("%1 failed, malformed reply from the server.", d->m_name));
        } else if (response.content[1].toString() != "OK") {
            setError(UserDefinedError);
            setErrorText(i18n("%1 failed, server replied: %2", d->m_name, QLatin1String(response.toString().constData())));
        }
        d->tags.removeAll(response.content.first().toString());
        if (d->tags.isEmpty()) { // Only emit result when the last command returned
            emitResult();
        }
        return Handled;
    }

    return NotHandled;
}
