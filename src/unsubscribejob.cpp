/*
    SPDX-FileCopyrightText: 2009 Andras Mantia <amantia@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "unsubscribejob.h"

#include <KLocalizedString>

#include "job_p.h"
#include "response_p.h"
#include "rfccodecs.h"
#include "session_p.h"

namespace KIMAP
{
class UnsubscribeJobPrivate : public JobPrivate
{
public:
    UnsubscribeJobPrivate(Session *session, const QString &name)
        : JobPrivate(session, name)
    {
    }
    ~UnsubscribeJobPrivate()
    {
    }

    QString mailBox;
};
}

using namespace KIMAP;

UnsubscribeJob::UnsubscribeJob(Session *session)
    : Job(*new UnsubscribeJobPrivate(session, i18n("Unsubscribe")))
{
}

UnsubscribeJob::~UnsubscribeJob()
{
}

void UnsubscribeJob::doStart()
{
    Q_D(UnsubscribeJob);
    d->tags << d->sessionInternal()->sendCommand("UNSUBSCRIBE", '\"' + KIMAP::encodeImapFolderName(d->mailBox.toUtf8()) + '\"');
}

void UnsubscribeJob::setMailBox(const QString &mailBox)
{
    Q_D(UnsubscribeJob);
    d->mailBox = mailBox;
}

QString UnsubscribeJob::mailBox() const
{
    Q_D(const UnsubscribeJob);
    return d->mailBox;
}
