/*
    SPDX-FileCopyrightText: 2009 Andras Mantia <amantia@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "closejob.h"

#include <KLocalizedString>

#include "job_p.h"
#include "response_p.h"
#include "session_p.h"

namespace KIMAP
{
class CloseJobPrivate : public JobPrivate
{
public:
    CloseJobPrivate(Session *session, const QString &name)
        : JobPrivate(session, name)
    {
    }

    quint64 highestModSeq = 0;
};
}

using namespace KIMAP;

CloseJob::CloseJob(Session *session)
    : Job(*new CloseJobPrivate(session, i18n("Close")))
{
}

void CloseJob::doStart()
{
    Q_D(CloseJob);
    d->tags << d->sessionInternal()->sendCommand("CLOSE");
}

quint64 CloseJob::newHighestModSeq() const
{
    Q_D(const CloseJob);
    return d->highestModSeq;
}

void CloseJob::handleResponse(const Response &response)
{
    Q_D(CloseJob);

    if (response.responseCode.size() >= 2 && response.responseCode[0].toString() == "HIGHESTMODSEQ") {
        d->highestModSeq = response.responseCode[1].toString().toULongLong();
    }

    Job::handleErrorReplies(response);
}
