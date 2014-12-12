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

#ifndef KIMAP_LISTRIGHTSJOB_H
#define KIMAP_LISTRIGHTSJOB_H

#include "kimap_export.h"

#include "acljobbase.h"

namespace KIMAP
{

class Session;
struct Message;
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
 * <a href="http://www.apps.ietf.org/rfc/rfc4314.html">RFC 4314</a>.
 */
class KIMAP_EXPORT ListRightsJob : public AclJobBase
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(ListRightsJob)

    friend class SessionPrivate;

public:
    explicit ListRightsJob(Session *session);
    virtual ~ListRightsJob();

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
    QByteArray identifier();

    /**
     * The rights that will always be assigned to the identifier,
     * regardless of the access control list.
     *
     * For example, under the UNIX permission model, the owner
     * of a mailbox will always have the Acl::Admin right.
     */
    Acl::Rights defaultRights();

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
    QList<Acl::Rights> possibleRights();

protected:
    void doStart() Q_DECL_OVERRIDE;
    void handleResponse(const Message &response) Q_DECL_OVERRIDE;

};

}

#endif
