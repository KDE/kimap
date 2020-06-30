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
    CloseJobPrivate(Session *session, const QString &name) : JobPrivate(session, name) { }
    ~CloseJobPrivate() { }
};
}

using namespace KIMAP;

CloseJob::CloseJob(Session *session)
    : Job(*new CloseJobPrivate(session, i18n("Close")))
{
}

CloseJob::~CloseJob()
{
}

void CloseJob::doStart()
{
    Q_D(CloseJob);
    d->tags << d->sessionInternal()->sendCommand("CLOSE");
}
