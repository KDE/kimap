/*
    SPDX-FileCopyrightText: 2009 Kevin Ottens <ervin@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "kimap_export.h"

#include "imapset.h"
#include "job.h"

#include <kmime/kmime_content.h>
#include <kmime/kmime_message.h>

namespace KIMAP
{
class Session;
struct Response;
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
 * <a href="https://tools.ietf.org/html/rfc2177">RFC 2177</a>.
 */
class KIMAP_EXPORT IdleJob : public Job
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(IdleJob)

public:
    explicit IdleJob(Session *session);
    ~IdleJob() override;

    /**
     * The last mailbox status that was reported.
     *
     * This is just the session's selected mailbox.
     */
    Q_REQUIRED_RESULT QString lastMailBox() const;
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
    Q_REQUIRED_RESULT int lastMessageCount() const;
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
    Q_REQUIRED_RESULT int lastRecentCount() const;

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
    void doStart() override;
    void handleResponse(const Response &response) override;

private:
    Q_PRIVATE_SLOT(d_func(), void emitStats())
    Q_PRIVATE_SLOT(d_func(), void resetTimeout())
};

}

