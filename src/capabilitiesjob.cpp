/*
    SPDX-FileCopyrightText: 2009 Kevin Ottens <ervin@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "capabilitiesjob.h"

#include <KLocalizedString>

#include "job_p.h"
#include "response_p.h"
#include "session_p.h"

namespace KIMAP
{
class CapabilitiesJobPrivate : public JobPrivate
{
public:
    CapabilitiesJobPrivate(Session *session, const QString &name)
        : JobPrivate(session, name)
    {
    }
    ~CapabilitiesJobPrivate()
    {
    }

    QStringList capabilities;
};
}

using namespace KIMAP;

CapabilitiesJob::CapabilitiesJob(Session *session)
    : Job(*new CapabilitiesJobPrivate(session, i18n("Capabilities")))
{
}

CapabilitiesJob::~CapabilitiesJob()
{
}

QStringList CapabilitiesJob::capabilities() const
{
    Q_D(const CapabilitiesJob);
    return d->capabilities;
}

void CapabilitiesJob::doStart()
{
    Q_D(CapabilitiesJob);
    d->tags << d->sessionInternal()->sendCommand("CAPABILITY");
}

void CapabilitiesJob::handleResponse(const Response &response)
{
    Q_D(CapabilitiesJob);
    if (handleErrorReplies(response) == NotHandled) {
        const int responseSize(response.content.size());
        if (responseSize >= 2 && response.content[1].toString() == "CAPABILITY") {
            for (int i = 2; i < responseSize; ++i) {
                d->capabilities << QLatin1String(response.content[i].toString().toUpper());
            }
            Q_EMIT capabilitiesReceived(d->capabilities);
        }
    }
}
