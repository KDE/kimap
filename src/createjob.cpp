/*
    SPDX-FileCopyrightText: 2009 Andras Mantia <amantia@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "createjob.h"

#include <KLocalizedString>

#include "job_p.h"
#include "response_p.h"
#include "rfccodecs.h"
#include "session_p.h"

namespace KIMAP
{
class CreateJobPrivate : public JobPrivate
{
public:
    CreateJobPrivate(Session *session, const QString &name)
        : JobPrivate(session, name)
    {
    }
    ~CreateJobPrivate()
    {
    }

    QString mailBox;
};
}

using namespace KIMAP;

CreateJob::CreateJob(Session *session)
    : Job(*new CreateJobPrivate(session, i18n("Create")))
{
}

CreateJob::~CreateJob()
{
}

void CreateJob::doStart()
{
    Q_D(CreateJob);
    d->tags << d->sessionInternal()->sendCommand("CREATE", '\"' + KIMAP::encodeImapFolderName(d->mailBox.toUtf8()) + '\"');
}

void CreateJob::handleResponse(const Response &response)
{
    Q_D(CreateJob);

    if (!response.content.isEmpty() && d->tags.contains(response.content.first().toString())) {
        if (response.content.size() >= 2 && response.content[1].toString() == "NO") {
            for (auto it = response.responseCode.cbegin(), end = response.responseCode.cend(); it != end; ++it) {
                // ALREADYEXISTS can be considered a success during CREATE
                // cf. https://tools.ietf.org/html/rfc5530#section-3
                if (it->toString() == "ALREADYEXISTS") {
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

void CreateJob::setMailBox(const QString &mailBox)
{
    Q_D(CreateJob);
    d->mailBox = mailBox;
}

QString CreateJob::mailBox() const
{
    Q_D(const CreateJob);
    return d->mailBox;
}
