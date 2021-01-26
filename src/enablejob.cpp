/*
    SPDX-FileCopyrightText: 2020 Daniel Vrátil <dvratil@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "enablejob.h"

#include <KLocalizedString>

#include "job_p.h"
#include "kimap_debug.h"
#include "response_p.h"
#include "session_p.h"

namespace KIMAP
{
class EnableJobPrivate : public JobPrivate
{
public:
    using JobPrivate::JobPrivate;

    QStringList reqCapabilities;
    QStringList enabledCapabilities;
};
}

using namespace KIMAP;

EnableJob::EnableJob(Session *session)
    : Job(*new EnableJobPrivate(session, i18n("Enable")))
{
}

EnableJob::~EnableJob() = default;

void EnableJob::setCapabilities(const QStringList &capabilities)
{
    Q_D(EnableJob);
    d->reqCapabilities = capabilities;
}

QStringList EnableJob::enabledCapabilities() const
{
    Q_D(const EnableJob);
    return d->enabledCapabilities;
}

void EnableJob::doStart()
{
    Q_D(EnableJob);
    d->tags << d->sessionInternal()->sendCommand("ENABLE", d->reqCapabilities.join(QLatin1Char{' '}).toLatin1());
}

void EnableJob::handleResponse(const Response &response)
{
    Q_D(EnableJob);

    if (handleErrorReplies(response) == NotHandled) {
        if (response.content.size() >= 2) {
            for (int i = 2; i < response.content.size(); ++i) {
                d->enabledCapabilities.push_back(QString::fromLatin1(response.content[i].toString()));
            }
        } else {
            qCDebug(KIMAP_LOG) << response.toString();
        }
    }
}
