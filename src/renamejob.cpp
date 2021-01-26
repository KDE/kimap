/*
    SPDX-FileCopyrightText: 2009 Andras Mantia <amantia@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "renamejob.h"

#include <KLocalizedString>

#include "job_p.h"
#include "response_p.h"
#include "rfccodecs.h"
#include "session_p.h"

namespace KIMAP
{
class RenameJobPrivate : public JobPrivate
{
public:
    RenameJobPrivate(Session *session, const QString &name)
        : JobPrivate(session, name)
    {
    }
    ~RenameJobPrivate()
    {
    }

    QString sourceMailBox;
    QString destinationMailBox;
};
}

using namespace KIMAP;

RenameJob::RenameJob(Session *session)
    : Job(*new RenameJobPrivate(session, i18n("Rename")))
{
}

RenameJob::~RenameJob()
{
}

void RenameJob::doStart()
{
    Q_D(RenameJob);
    d->tags << d->sessionInternal()->sendCommand("RENAME",
                                                 '\"' + KIMAP::encodeImapFolderName(d->sourceMailBox.toUtf8()) + "\" \""
                                                     + KIMAP::encodeImapFolderName(d->destinationMailBox.toUtf8()) + '\"');
}

void RenameJob::setSourceMailBox(const QString &mailBox)
{
    Q_D(RenameJob);
    d->sourceMailBox = mailBox;
}

QString RenameJob::sourceMailBox() const
{
    Q_D(const RenameJob);
    return d->sourceMailBox;
}

void RenameJob::setDestinationMailBox(const QString &mailBox)
{
    Q_D(RenameJob);
    d->destinationMailBox = mailBox;
}

QString RenameJob::destinationMailBox() const
{
    Q_D(const RenameJob);
    return d->destinationMailBox;
}
