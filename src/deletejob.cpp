/*
    SPDX-FileCopyrightText: 2009 Andras Mantia <amantia@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "deletejob.h"

#include <KLocalizedString>

#include "job_p.h"
#include "response_p.h"
#include "rfccodecs.h"
#include "session_p.h"

namespace KIMAP
{
class DeleteJobPrivate : public JobPrivate
{
public:
    DeleteJobPrivate(Session *session, const QString &name)
        : JobPrivate(session, name)
    {
    }
    ~DeleteJobPrivate()
    {
    }

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

void DeleteJob::handleResponse(const Response &response)
{
    Q_D(DeleteJob);

    if (!response.content.isEmpty() && d->tags.contains(response.content.first().toString())) {
        if (response.content.size() >= 2 && response.content[1].toString() == "NO") {
            for (auto it = response.responseCode.cbegin(), end = response.responseCode.cend(); it != end; ++it) {
                // NONEXISTENT can be considered a success during DELETE
                // cf. https://tools.ietf.org/html/rfc5530#section-3
                if (it->toString() == "NONEXISTENT") {
                    // Code copied from handleErrorReplies:
                    d->tags.removeAll(response.content.first().toString());
                    if (d->tags.isEmpty()) { // Only emit result when the last command returned
                        emitResult();
                    }
                    return;
                }
            }
        }
    }

    handleErrorReplies(response);
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
