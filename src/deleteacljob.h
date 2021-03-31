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
class DeleteAclJobPrivate;

/**
 * Removes an identifier from the ACL of a mailbox.
 *
 * This job can only be run when the session is in the
 * authenticated (or selected) state.
 *
 * The user must have the Acl::Admin permission
 * on the mailbox for this job to succeed (see
 * MyRightsJob).
 *
 * This job requires that the server supports the ACL
 * capability, defined in
 * <a href="https://tools.ietf.org/html/rfc4314">RFC 4314</a>.
 */
class KIMAP_EXPORT DeleteAclJob : public AclJobBase
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(DeleteAclJob)

    friend class SessionPrivate;

public:
    explicit DeleteAclJob(Session *session);
    ~DeleteAclJob() override;

    /**
     * Sets the identifier to remove
     */
    void setIdentifier(const QByteArray &identifier);
    /**
     * The identifier that will be removed
     */
    Q_REQUIRED_RESULT QByteArray identifier();

protected:
    void doStart() override;
};

}

