/*
    SPDX-FileCopyrightText: 2016 Daniel Vrátil <dvratil@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "movejob.h"

#include "job_p.h"
#include "response_p.h"
#include "rfccodecs.h"
#include "session_p.h"

#include <KLocalizedString>

// TODO: when custom error codes are introduced, handle the NO [TRYCREATE] response

namespace KIMAP
{
class MoveJobPrivate : public JobPrivate
{
public:
    MoveJobPrivate(Session *session, const QString &name)
        : JobPrivate(session, name)
    {
    }

    ~MoveJobPrivate()
    {
    }

    QString mailBox;
    ImapSet set;
    ImapSet resultingUids;
    bool uidBased = false;
};
}

using namespace KIMAP;

MoveJob::MoveJob(Session *session)
    : Job(*new MoveJobPrivate(session, i18n("Move")))
{
    Q_D(MoveJob);
    d->uidBased = false;
}

MoveJob::~MoveJob()
{
}

void MoveJob::setMailBox(const QString &mailBox)
{
    Q_D(MoveJob);
    d->mailBox = mailBox;
}

QString MoveJob::mailBox() const
{
    Q_D(const MoveJob);
    return d->mailBox;
}

void MoveJob::setSequenceSet(const ImapSet &set)
{
    Q_D(MoveJob);
    d->set = set;
}

ImapSet MoveJob::sequenceSet() const
{
    Q_D(const MoveJob);
    return d->set;
}

void MoveJob::setUidBased(bool uidBased)
{
    Q_D(MoveJob);
    d->uidBased = uidBased;
}

bool MoveJob::isUidBased() const
{
    Q_D(const MoveJob);
    return d->uidBased;
}

ImapSet MoveJob::resultingUids() const
{
    Q_D(const MoveJob);
    return d->resultingUids;
}

void MoveJob::doStart()
{
    Q_D(MoveJob);

    d->set.optimize();
    QByteArray parameters = d->set.toImapSequenceSet() + ' ';
    parameters += '\"' + KIMAP::encodeImapFolderName(d->mailBox.toUtf8()) + '\"';

    QByteArray command = "MOVE";
    if (d->uidBased) {
        command = "UID " + command;
    }

    d->tags << d->sessionInternal()->sendCommand(command, parameters);
}

void MoveJob::handleResponse(const Response &response)
{
    Q_D(MoveJob);

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
