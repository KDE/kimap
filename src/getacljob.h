/*
    Copyright (c) 2009 Andras Mantia <amantia@kde.org>

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

#ifndef KIMAP_GETACLJOB_H
#define KIMAP_GETACLJOB_H

#include "kimap_export.h"

#include "acljobbase.h"

namespace KIMAP
{

class Session;
struct Message;
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
 * <a href="http://www.apps.ietf.org/rfc/rfc4314.html">RFC 4314</a>.
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
    virtual ~GetAclJob();

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
    virtual void doStart() Q_DECL_OVERRIDE;
    virtual void handleResponse(const Message &response) Q_DECL_OVERRIDE;

};

}

#endif
