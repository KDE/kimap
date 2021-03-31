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
class CloseJobPrivate;

/**
 * Closes the current mailbox.
 *
 * This job can only be run when the session is in the selected state.
 *
 * Permanently removes all messages that have the \\Deleted
 * flag set from the currently selected mailbox, and returns
 * to the authenticated state from the selected state.
 *
 * The server will not provide any notifications of which
 * messages were expunged, so this is quicker than doing
 * an expunge and then implicitly closing the mailbox
 * (by selecting or examining another mailbox or logging
 * out). If the QRESYNC extension (RFC5162) is available on the
 * server and has been enabled, the job will provide a new
 * modification sequence after expunging the deleted messages.
 *
 * No messages are removed if the mailbox is open in a read-only
 * state, or if the server supports ACLs and the user does not
 * have the Acl::Expunge right on the mailbox.
 */
class KIMAP_EXPORT CloseJob : public Job
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(CloseJob)

    friend class SessionPrivate;

public:
    explicit CloseJob(Session *session);
    ~CloseJob() override = default;

    /**
     * Returns new modification sequence number after expunging messages.
     *
     * This value is only valid when server supports the QRESYNC extension
     * (RFC5162) and it has been explicitly enabled on this session.
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

