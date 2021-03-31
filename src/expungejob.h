/*
    SPDX-FileCopyrightText: 2009 Andras Mantia <amantia@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "kimap_export.h"

#include "job.h"

namespace KIMAP
{
class Session;
struct Response;
class ImapSet;
class ExpungeJobPrivate;

/**
 * Expunges the deleted messages in the selected mailbox.
 *
 * This permanently removes any messages that have the
 * \Deleted flag set in the selected mailbox.
 *
 * This job can only be run when the session is in the
 * selected state.
 *
 * If the server supports ACLs, the user will need the
 * Acl::Expunge right on the mailbox.
 */
class KIMAP_EXPORT ExpungeJob : public Job
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(ExpungeJob)

    friend class SessionPrivate;

public:
    explicit ExpungeJob(Session *session);
    ~ExpungeJob() override = default;

    /**
     * Returns UIDs of messages that have been expunged.
     *
     * This feature is only available when QRESYNC capability (RFC5162) is
     * supported by the server and have been enabled on the current session.
     *
     * @see KIMAP::EnableJob
     * @since 5.16
     */
    KIMAP::ImapSet vanishedMessages() const;

    /**
     * Returns new highest modification sequence number.
     *
     * This feature is only available when QRESYNC capability (RFC5162) is
     * supported by the server and have been enabled on the current session.
     *
     * @see KIMAP::EnableJob
     * @since 5.16
     */
    quint64 newHighestModSeq() const;

protected:
    void doStart() override;
    void handleResponse(const Response &response) override;
};

}

