/*
    SPDX-FileCopyrightText: 2009 Andras Mantia <amantia@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "deletejob.h"

#include <KLocalizedString>

#include "job_p.h"
#include "response_p.h"
#include "session_p.h"
#include "rfccodecs.h"

namespace KIMAP
{
class DeleteJobPrivate : public JobPrivate
{
public:
    DeleteJobPrivate(Session *session, const QString &name) : JobPrivate(session, name) { }
    ~DeleteJobPrivate() { }

    QString mailBox;
};
}

using namespace KIMAP;

DeleteJob::DeleteJob(Session *session)
    : Job(*new DeleteJobPrivate(session, i18n("Delete")))
{
}

DeleteJob::~DeleteJob()
{
}

void DeleteJob::doStart()
{
    Q_D(DeleteJob);
    d->tags << d->sessionInternal()->sendCommand("DELETE", '\"' + KIMAP::encodeImapFolderName(d->mailBox.toUtf8()) + '\"');
}

void DeleteJob::setMailBox(const QString &mailBox)
{
    Q_D(DeleteJob);
    d->mailBox = mailBox;
}

QString DeleteJob::mailBox() const
{
    Q_D(const DeleteJob);
    return d->mailBox;
}
