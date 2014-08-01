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

#ifndef KIMAP_MYRIGHTSJOB_H
#define KIMAP_MYRIGHTSJOB_H

#include "kimap_export.h"

#include "acljobbase.h"

namespace KIMAP
{

class Session;
struct Message;
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
 * <a href="http://www.apps.ietf.org/rfc/rfc4314.html">RFC 4314</a>.
 */
class KIMAP_EXPORT MyRightsJob : public AclJobBase
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(MyRightsJob)

    friend class SessionPrivate;

public:
    explicit MyRightsJob(Session *session);
    virtual ~MyRightsJob();

    /**
     * Check whether the current user has the a particular right
     * on the mailbox.
     *
     * The result of this method is undefined if the job has
     * not yet completed.
     *
     * @param right       the right to check for
     */
    bool hasRightEnabled(Acl::Right right);
    /**
     * Get the rights for the current user on the mailbox.
     *
     * The result of this method is undefined if the job has
     * not yet completed.
     */
    Acl::Rights rights();

protected:
    virtual void doStart();
    virtual void handleResponse(const Message &response);

};

}

#endif
