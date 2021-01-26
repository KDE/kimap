/*
    SPDX-FileCopyrightText: 2009 Andras Mantia <amantia@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "copyjob.h"

#include <KLocalizedString>

#include "job_p.h"
#include "response_p.h"
#include "rfccodecs.h"
#include "session_p.h"

// TODO: when custom error codes are introduced, handle the NO [TRYCREATE] response

namespace KIMAP
{
class CopyJobPrivate : public JobPrivate
{
public:
    CopyJobPrivate(Session *session, const QString &name)
        : JobPrivate(session, name)
    {
    }
    ~CopyJobPrivate()
    {
    }

    QString mailBox;
    ImapSet set;
    bool uidBased = false;
    ImapSet resultingUids;
};
}

using namespace KIMAP;

CopyJob::CopyJob(Session *session)
    : Job(*new CopyJobPrivate(session, i18n("Copy")))
{
    Q_D(CopyJob);
    d->uidBased = false;
}

CopyJob::~CopyJob()
{
}

void CopyJob::setMailBox(const QString &mailBox)
{
    Q_D(CopyJob);
    d->mailBox = mailBox;
}

QString CopyJob::mailBox() const
{
    Q_D(const CopyJob);
    return d->mailBox;
}

void CopyJob::setSequenceSet(const ImapSet &set)
{
    Q_D(CopyJob);
    d->set = set;
}

ImapSet CopyJob::sequenceSet() const
{
    Q_D(const CopyJob);
    return d->set;
}

void CopyJob::setUidBased(bool uidBased)
{
    Q_D(CopyJob);
    d->uidBased = uidBased;
}

bool CopyJob::isUidBased() const
{
    Q_D(const CopyJob);
    return d->uidBased;
}

ImapSet CopyJob::resultingUids() const
{
    Q_D(const CopyJob);
    return d->resultingUids;
}

void CopyJob::doStart()
{
    Q_D(CopyJob);

    d->set.optimize();
    QByteArray parameters = d->set.toImapSequenceSet() + ' ';
    parameters += '\"' + KIMAP::encodeImapFolderName(d->mailBox.toUtf8()) + '\"';

    QByteArray command = "COPY";
    if (d->uidBased) {
        command = "UID " + command;
    }

    d->tags << d->sessionInternal()->sendCommand(command, parameters);
}

void CopyJob::handleResponse(const Response &response)
{
    Q_D(CopyJob);
    for (auto it = response.responseCode.cbegin(), end = response.responseCode.cend(); it != end; ++it) {
        if (it->toString() == "COPYUID") {
            it = it + 3;
            if (it < end) {
                d->resultingUids = ImapSet::fromImapSequenceSet(it->toString());
            }
            break;
        }
    }

    handleErrorReplies(response);
}
