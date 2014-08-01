/*
    Copyright (c) 2009 Andras Mantia <amantia@kde.org>

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

#include "deletejob.h"

#include <KLocalizedString>

#include "job_p.h"
#include "message_p.h"
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
