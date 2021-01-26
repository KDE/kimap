/*
    SPDX-FileCopyrightText: 2009 Andras Mantia <amantia@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "subscribejob.h"

#include <KLocalizedString>

#include "job_p.h"
#include "response_p.h"
#include "rfccodecs.h"
#include "session_p.h"

namespace KIMAP
{
class SubscribeJobPrivate : public JobPrivate
{
public:
    SubscribeJobPrivate(Session *session, const QString &name)
        : JobPrivate(session, name)
    {
    }
    ~SubscribeJobPrivate()
    {
    }

    QString mailBox;
};
}

using namespace KIMAP;

SubscribeJob::SubscribeJob(Session *session)
    : Job(*new SubscribeJobPrivate(session, i18n("Subscribe")))
{
}

SubscribeJob::~SubscribeJob()
{
}

void SubscribeJob::doStart()
{
    Q_D(SubscribeJob);
    d->tags << d->sessionInternal()->sendCommand("SUBSCRIBE", '\"' + KIMAP::encodeImapFolderName(d->mailBox.toUtf8()) + '\"');
}

void SubscribeJob::setMailBox(const QString &mailBox)
{
    Q_D(SubscribeJob);
    d->mailBox = mailBox;
}

QString SubscribeJob::mailBox() const
{
    Q_D(const SubscribeJob);
    return d->mailBox;
}
