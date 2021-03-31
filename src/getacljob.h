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
class GetAclJobPrivate;

/**
 * Gets the ACL for a mailbox
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
 */
class KIMAP_EXPORT GetAclJob : public AclJobBase
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(GetAclJob)

    friend class SessionPrivate;

public:
    explicit GetAclJob(Session *session);
    ~GetAclJob() override;

    /**
     * The identifiers present in the ACL.
     *
     * This method will return an empty list if the job has
     * not yet been run.
     *
     * See the GetAclJob documentation for an explanation of
     * identifiers; in particular, identifiers starting with
     * '-' specify negative rights.
     */
    QList<QByteArray> identifiers() const;
    /**
     * Check whether an identifier has a given right set
     *
     * The result of this method is undefined if the job has
     * not yet completed.
     *
     * See the GetAclJob documentation for an explanation of
     * identifiers; in particular, identifiers starting with
     * '-' specify negative rights.
     *
     * Note that this will not tell you whether the net result
     * of all the ACL entries means that a given user has
     * a certain right.
     *
     * @param identifier  the identifier to check the rights for
     * @param right       the right to check for
     */
    bool hasRightEnabled(const QByteArray &identifier, Acl::Right right) const;
    /**
     * Get the rights associated with an identifier.
     *
     * The result of this method is undefined if the job has
     * not yet completed.
     *
     * See the GetAclJob documentation for an explanation of
     * identifiers; in particular, identifiers starting with
     * '-' specify negative rights.
     *
     * Note that this will not tell you the rights that a
     * given user will have once all the ACL entries have
     * been taken into account.
     *
     * @param identifier  the identifier to check the rights for
     */
    Acl::Rights rights(const QByteArray &identifier) const;

    /**
     * Gets the full access control list.
     *
     * The result of this method is undefined if the job has
     * not yet completed.
     *
     * See the GetAclJob documentation for an explanation of
     * identifiers; in particular, identifiers starting with
     * '-' specify negative rights.
     */
    QMap<QByteArray, Acl::Rights> allRights() const;

protected:
    void doStart() override;
    void handleResponse(const Response &response) override;
};

}

