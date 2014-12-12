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

#ifndef KIMAP_IDLEJOB_H
#define KIMAP_IDLEJOB_H

#include "kimap_export.h"

#include "imapset.h"
#include "job.h"

#include "kmime/kmime_content.h"
#include "kmime/kmime_message.h"

#include <boost/shared_ptr.hpp>

namespace KIMAP
{

class Session;
struct Message;
class IdleJobPrivate;

/**
 * Idles the connection to the IMAP server.
 *
 * This job can be run while the client has no other use
 * for the connection, and the server will send updates
 * about the selected mailbox.
 *
 * Note that although the server may send a variety of
 * responses while the job is running (including EXPUNGE,
 * for example), only RECENT and EXISTS responses are
 * actually reported by this job.
 *
 * The job also processes updates in pairs - if the server
 * sends an EXISTS update but not a RECENT one (because
 * another client is changing the mailbox contents), this
 * job will not report the update.
 *
 * It only makes sense to run this job when the session is
 * in the selected state.
 *
 * This job requires that the server supports the IDLE
 * capability, defined in
 * <a href="http://www.apps.ietf.org/rfc/rfc2177.html">RFC 2177</a>.
 */
class KIMAP_EXPORT IdleJob : public Job
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(IdleJob)

public:
    explicit IdleJob(Session *session);
    virtual ~IdleJob();

    /**
     * The last mailbox status that was reported.
     *
     * This is just the session's selected mailbox.
     */
    QString lastMailBox() const;
    /**
     * The last message count that was reported.
     *
     * The server will send updates about the number of
     * messages in the mailbox when that number changes.
     * This is the last number it reported.
     *
     * @return  the last message count the server reported,
     *          or -1 if it has not reported a message count
     *          since the job started.
     */
    int lastMessageCount() const;
    /**
     * The last recent message count that was reported.
     *
     * The server will send updates about the number of
     * messages in the mailbox that are tagged with \Recent
     * when that number changes. This is the last number it
     * reported.
     *
     * @return  the last recent message count the server reported,
     *          or -1 if it has not reported a recent message count
     *          since the job started.
     */
    int lastRecentCount() const;

public Q_SLOTS:
    /**
     * Stops the idle job.
     */
    void stop();

Q_SIGNALS:
    /**
     * Signals that the server has notified that the total and
     * recent message counts have changed.
     *
     * @param job           this object
     * @param mailBox       the selected mailbox
     * @param messageCount  the new total message count reported by the server
     * @param recentCount   the new "recent message" count reported by the server
     */
    void mailBoxStats(KIMAP::IdleJob *job, const QString &mailBox, int messageCount, int recentCount);

    /**
     * Signals that the server has notified that the some messages flags
     * have changed
     *
     * @param job this object
     * @param uid UID of message that has changed
     * @since 4.12
     */
    void mailBoxMessageFlagsChanged(KIMAP::IdleJob *job, qint64 uid);

protected:
    void doStart() Q_DECL_OVERRIDE;
    void handleResponse(const Message &response) Q_DECL_OVERRIDE;

private:
    Q_PRIVATE_SLOT(d_func(), void emitStats())
    Q_PRIVATE_SLOT(d_func(), void resetTimeout())
};

}

#endif
