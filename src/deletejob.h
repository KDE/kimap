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
class DeleteJobPrivate;

/**
 * Delete a mailbox
 *
 * Note that some servers will refuse to delete a
 * mailbox unless it is empty (ie: all mails have
 * had their \Deleted flag set, and then the
 * mailbox has been expunged).
 *
 * This job can only be run when the session is in the
 * authenticated (or selected) state.
 *
 * If the server supports ACLs, you will need the
 * Acl::DeleteMailbox right on the mailbox.
 */
class KIMAP_EXPORT DeleteJob : public Job
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(DeleteJob)

    friend class SessionPrivate;

public:
    explicit DeleteJob(Session *session);
    ~DeleteJob() override;

    /**
     * Set the mailbox to delete.
     */
    void setMailBox(const QString &mailBox);
    /**
     * The mailbox that will be deleted.
     */
    QString mailBox() const;

protected:
    void doStart() override;
    void handleResponse(const Response &response) override;
};

}

