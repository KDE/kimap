/*
    SPDX-FileCopyrightText: 2009 Kevin Ottens <ervin@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "idlejob.h"

#include <KLocalizedString>
#include <QTimer>

#include "job_p.h"
#include "response_p.h"
#include "session_p.h"

namespace KIMAP
{
class IdleJobPrivate : public JobPrivate
{
public:
    IdleJobPrivate(IdleJob *job, Session *session, const QString &name)
        : JobPrivate(session, name)
        , q(job)
    {
    }
    ~IdleJobPrivate()
    {
    }

    void emitStats()
    {
        emitStatsTimer.stop();

        Q_EMIT q->mailBoxStats(q, m_session->selectedMailBox(), messageCount, recentCount);

        lastMessageCount = messageCount;
        lastRecentCount = recentCount;

        messageCount = -1;
        recentCount = -1;
    }

    void resetTimeout()
    {
        sessionInternal()->setSocketTimeout(originalSocketTimeout);
    }

    IdleJob *const q;

    QTimer emitStatsTimer;

    int messageCount = -1;
    int recentCount = -1;

    int lastMessageCount = -1;
    int lastRecentCount = -1;

    int originalSocketTimeout = -1;
};
}

using namespace KIMAP;

IdleJob::IdleJob(Session *session)
    : Job(*new IdleJobPrivate(this, session, i18nc("name of the idle job", "Idle")))
{
    Q_D(IdleJob);
    connect(&d->emitStatsTimer, SIGNAL(timeout()), this, SLOT(emitStats()));

    connect(this, SIGNAL(result(KJob *)), this, SLOT(resetTimeout()));
}

IdleJob::~IdleJob()
{
}

void KIMAP::IdleJob::stop()
{
    Q_D(IdleJob);
    d->sessionInternal()->setSocketTimeout(d->originalSocketTimeout);
    d->sessionInternal()->sendData("DONE");
}

void IdleJob::doStart()
{
    Q_D(IdleJob);
    d->originalSocketTimeout = d->sessionInternal()->socketTimeout();
    d->sessionInternal()->setSocketTimeout(-1);
    d->tags << d->sessionInternal()->sendCommand("IDLE");
}

void IdleJob::handleResponse(const Response &response)
{
    Q_D(IdleJob);

    // We can predict it'll be handled by handleErrorReplies() so Q_EMIT
    // pending signals now (if needed) so that result() will really be
    // the last emitted signal.
    if (!response.content.isEmpty() && d->tags.size() == 1 && d->tags.contains(response.content.first().toString())
        && (d->messageCount >= 0 || d->recentCount >= 0)) {
        d->emitStats();
    }

    if (handleErrorReplies(response) == NotHandled) {
        if (!response.content.isEmpty() && response.content[0].toString() == "+") {
            // Got the continuation all is fine
            return;

        } else if (response.content.size() > 2) {
            const QByteArray ba = response.content[2].toString();
            if (ba == "EXISTS") {
                if (d->messageCount >= 0) {
                    d->emitStats();
                }

                d->messageCount = response.content[1].toString().toInt();
            } else if (ba == "RECENT") {
                if (d->recentCount >= 0) {
                    d->emitStats();
                }

                d->recentCount = response.content[1].toString().toInt();
            } else if (ba == "FETCH") {
                const qint64 uid = response.content[1].toString().toLongLong();
                Q_EMIT mailBoxMessageFlagsChanged(this, uid);
            }
        }

        if (d->messageCount >= 0 && d->recentCount >= 0) {
            d->emitStats();
        } else if (d->messageCount >= 0 || d->recentCount >= 0) {
            d->emitStatsTimer.start(200);
        }
    }
}

QString KIMAP::IdleJob::lastMailBox() const
{
    Q_D(const IdleJob);
    return d->m_session->selectedMailBox();
}

int KIMAP::IdleJob::lastMessageCount() const
{
    Q_D(const IdleJob);
    return d->lastMessageCount;
}

int KIMAP::IdleJob::lastRecentCount() const
{
    Q_D(const IdleJob);
    return d->lastRecentCount;
}

#include "moc_idlejob.cpp"
