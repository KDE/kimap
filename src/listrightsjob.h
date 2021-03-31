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
class ListRightsJobPrivate;

/**
 * Lists the possible and automatic rights for
 * an identifier on a mailbox
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
class KIMAP_EXPORT ListRightsJob : public AclJobBase
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(ListRightsJob)

    friend class SessionPrivate;

public:
    explicit ListRightsJob(Session *session);
    ~ListRightsJob() override;

    /**
     * Sets the identifier that should be looked up
     *
     * The meaning of identifiers depends on the server implementation,
     * with the following restrictions:
     *
     * - "anyone" means any authenticated user, including anonymous
     * - an identifier starting with a minus sign ('-') indicates
     *   "negative rights": rights that should be taken away from
     *   matching users
     *
     * Other than the above restrictions, ACL identifiers are usually
     * IMAP usernames, but could potentially be group names as well.
     *
     * Note that negative rights override positive rights: if
     * "fred" and "-fred" are both assigned the 'w' right, the
     * user "fred" will not have the 'w' right.
     *
     * @param identifier  the identifier to list the rights for
     */
    void setIdentifier(const QByteArray &identifier);
    /**
     * The identifier that will be looked up
     */
    Q_REQUIRED_RESULT QByteArray identifier();

    /**
     * The rights that will always be assigned to the identifier,
     * regardless of the access control list.
     *
     * For example, under the UNIX permission model, the owner
     * of a mailbox will always have the Acl::Admin right.
     */
    Q_REQUIRED_RESULT Acl::Rights defaultRights();

    /**
     * The rights it is possible to assign to the identifier.
     *
     * The rights are grouped by those that are tied together.
     * For each set of rights in the returned list, either all
     * or none of those rights may be set, but not only some of
     * them.
     *
     * For example, under the UNIX permission model, the following
     * rights are all controlled by the "write" flag, and hence
     * must either all be set or all be not set:
     * - Acl::KeepSeen
     * - Acl::Write
     * - Acl::Insert
     * - Acl::DeleteMessage
     * - Acl::Expunge
     */
    Q_REQUIRED_RESULT QList<Acl::Rights> possibleRights();

protected:
    void doStart() override;
    void handleResponse(const Response &response) override;
};

}

