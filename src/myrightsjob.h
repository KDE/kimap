/*
    SPDX-FileCopyrightText: 2009 Andras Mantia <amantia@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "kimap_export.h"

#include "acljobbase.h"

namespace KIMAP
{
class Session;
struct Response;
class MyRightsJobPrivate;

/**
 * Determine the rights the currently-logged-in user
 * has on the current mailbox.
 *
 * This should take into account the full access control
 * list.
 *
 * This job can only be run when the session is in the
 * authenticated (or selected) state.
 *
 * The current user must have one of the following rights
 * on the mailbox for this job to succeed:
 * - Acl::Lookup
 * - Acl::Read
 * - Acl::Insert
 * - Acl::CreateMailbox
 * - Acl::DeleteMailbox
 * - Acl::Admin
 *
 * This job requires that the server supports the ACL
 * capability, defined in
 * <a href="https://tools.ietf.org/html/rfc4314">RFC 4314</a>.
 */
class KIMAP_EXPORT MyRightsJob : public AclJobBase
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(MyRightsJob)

    friend class SessionPrivate;

public:
    explicit MyRightsJob(Session *session);
    ~MyRightsJob() override;

    /**
     * Check whether the current user has the a particular right
     * on the mailbox.
     *
     * The result of this method is undefined if the job has
     * not yet completed.
     *
     * @param right       the right to check for
     */
    Q_REQUIRED_RESULT bool hasRightEnabled(Acl::Right right);
    /**
     * Get the rights for the current user on the mailbox.
     *
     * The result of this method is undefined if the job has
     * not yet completed.
     */
    Q_REQUIRED_RESULT Acl::Rights rights();

protected:
    void doStart() override;
    void handleResponse(const Response &response) override;
};

}

