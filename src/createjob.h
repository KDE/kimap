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
class CreateJobPrivate;

/**
 * Creates a new mailbox
 *
 * This job can only be run when the session is in the
 * authenticated (or selected) state.
 *
 * This job will fail if the mailbox already exists.
 *
 * If the server supports ACLs, the user must have the
 * Acl::CreateMailbox permission on the parent
 * mailbox.  Note that what is meant by "parent mailbox"
 * depends on the server: . and / are typical hierachy
 * delimiters.
 */
class KIMAP_EXPORT CreateJob : public Job
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(CreateJob)

    friend class SessionPrivate;

public:
    explicit CreateJob(Session *session);
    ~CreateJob() override;

    /**
     * Set the name of the new mailbox
     *
     * @param mailBox  an (unquoted) identifier that does not correspond
     *                 to an existing mailbox name
     */
    void setMailBox(const QString &mailBox);
    /**
     * The name of the mailbox that will be created
     */
    Q_REQUIRED_RESULT QString mailBox() const;

protected:
    void doStart() override;
    void handleResponse(const Response &response) override;
};

}

