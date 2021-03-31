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
class SetAclJobPrivate;

/**
 * Sets the rights that correspond to an identifier on a mailbox
 *
 * This job can only be run when the session is in the
 * authenticated (or selected) state.
 *
 * This job requires that the server supports the ACL
 * capability, defined in
 * <a href="https://tools.ietf.org/html/rfc4314">RFC 4314</a>.
 */
class KIMAP_EXPORT SetAclJob : public AclJobBase
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(SetAclJob)

    friend class SessionPrivate;

public:
    explicit SetAclJob(Session *session);
    ~SetAclJob() override;

    /**
     * Sets the rights that will be changed for the identifier
     *
     * Note that multiple calls to this method will have a
     * non-intuitive effect: the @p modifier value of the most
     * recent call will be used, but the OR'd-together values
     * of all calls to setRights() will be used.
     *
     * If the server does not recognise any of the rights,
     * the job will fail and the ACL for the mailbox will
     * remain unchanged.
     *
     * Note that some rights may be tied together, and must be set
     * or removed as a group.  See ListRightsJob::possibleRights()
     * for more details.  The server will only set a tied group
     * of rights if you have requested that all the rights in that
     * group should be set.
     *
     * @param modifier  determines whether the rights will be
     *                  added to the identifier, removed from
     *                  the identifier or will replace any
     *                  existing rights assigned to the
     *                  identifier
     * @param rights    the rights to be added, removed or set
     */
    void setRights(AclModifier modifier, Acl::Rights rights);

    /**
     * Sets the identifier the rights will be modified for
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
     * @param identifier the identifier to set
     */
    void setIdentifier(const QByteArray &identifier);
    /**
     * The identifier that rights will be associated with
     */
    Q_REQUIRED_RESULT QByteArray identifier();

protected:
    void doStart() override;
};

}

