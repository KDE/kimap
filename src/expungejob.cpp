/*
    SPDX-FileCopyrightText: 2009 Andras Mantia <amantia@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "expungejob.h"

#include "kimap_debug.h"
#include <KLocalizedString>

#include "imapset.h"
#include "job_p.h"
#include "response_p.h"
#include "session_p.h"

namespace KIMAP
{
class ExpungeJobPrivate : public JobPrivate
{
public:
    ExpungeJobPrivate(Session *session, const QString &name)
        : JobPrivate(session, name)
    {
    }
#if 0
    QList< int > items;
#endif
    KIMAP::ImapSet vanished;
    quint64 highestModSeq = 0;
};
}

using namespace KIMAP;

ExpungeJob::ExpungeJob(Session *session)
    : Job(*new ExpungeJobPrivate(session, i18n("Expunge")))
{
}

KIMAP::ImapSet ExpungeJob::vanishedMessages() const
{
    Q_D(const ExpungeJob);
    return d->vanished;
}

quint64 ExpungeJob::newHighestModSeq() const
{
    Q_D(const ExpungeJob);
    return d->highestModSeq;
}

void ExpungeJob::doStart()
{
    Q_D(ExpungeJob);
    d->tags << d->sessionInternal()->sendCommand("EXPUNGE");
}

void ExpungeJob::handleResponse(const Response &response)
{
    Q_D(ExpungeJob);

    // Must be handler before handleErrorReplies(), so the value is available
    // before the result is emitted.
    if (response.responseCode.size() >= 2) {
        if (response.responseCode[0].toString() == "HIGHESTMODSEQ") {
            d->highestModSeq = response.responseCode[1].toString().toULongLong();
        }
    }

    if (handleErrorReplies(response) == NotHandled) {
        if (response.content.size() >= 3) {
            if (response.content[1].toString() == "VANISHED") {
                d->vanished = KIMAP::ImapSet::fromImapSequenceSet(response.content[2].toString());
                return;
            } else if (response.content[2].toString() == "EXPUNGE") {
#if 0
                QByteArray s = response.content[1].toString();
                bool ok = true;
                int id = s.toInt(&ok);
                if (ok) {
                    d->items.append(id);
                }
                //TODO error handling
#endif
                return;
            }
        }
        qCDebug(KIMAP_LOG) << "Unhandled response: " << response.toString().constData();
    }
}
