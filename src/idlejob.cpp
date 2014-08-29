/*
    Copyright (c) 2009 Kevin Ottens <ervin@kde.org>

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

#include "idlejob.h"

#include <QtCore/QTimer>
#include <KLocalizedString>

#include "job_p.h"
#include "message_p.h"
#include "session_p.h"

namespace KIMAP
{
class IdleJobPrivate : public JobPrivate
{
public:
    IdleJobPrivate(IdleJob *job, Session *session, const QString &name)
        : JobPrivate(session, name), q(job),
          messageCount(-1), recentCount(-1),
          lastMessageCount(-1), lastRecentCount(-1),
          originalSocketTimeout(-1) { }
    ~IdleJobPrivate() { }

    void emitStats()
    {
        emitStatsTimer.stop();

        emit q->mailBoxStats(q, m_session->selectedMailBox(),
                             messageCount, recentCount);

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

    int messageCount;
    int recentCount;

    int lastMessageCount;
    int lastRecentCount;

    int originalSocketTimeout;
};
}

using namespace KIMAP;

IdleJob::IdleJob(Session *session)
    : Job(*new IdleJobPrivate(this, session, i18nc("name of the idle job", "Idle")))
{
    Q_D(IdleJob);
    connect(&d->emitStatsTimer, SIGNAL(timeout()),
            this, SLOT(emitStats()));

    connect(this, SIGNAL(result(KJob*)),
            this, SLOT(resetTimeout()));
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

void IdleJob::handleResponse(const Message &response)
{
    Q_D(IdleJob);

    // We can predict it'll be handled by handleErrorReplies() so emit
    // pending signals now (if needed) so that result() will really be
    // the last emitted signal.
    if (!response.content.isEmpty() &&
            d->tags.size() == 1 &&
            d->tags.contains(response.content.first().toString()) &&
            (d->messageCount >= 0 || d->recentCount >= 0)) {
        d->emitStats();
    }

    if (handleErrorReplies(response) == NotHandled) {
        if (response.content.size() > 0 && response.content[0].toString() == "+") {
            // Got the continuation all is fine
            return;

        } else if (response.content.size() > 2) {
            if (response.content[2].toString() == "EXISTS") {
                if (d->messageCount >= 0) {
                    d->emitStats();
                }

                d->messageCount = response.content[1].toString().toInt();
            } else if (response.content[2].toString() == "RECENT") {
                if (d->recentCount >= 0) {
                    d->emitStats();
                }

                d->recentCount = response.content[1].toString().toInt();
            } else if (response.content[2].toString() == "FETCH") {
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
