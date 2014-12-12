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

#ifndef KIMAP_DELETEACLJOB_H
#define KIMAP_DELETEACLJOB_H

#include "kimap_export.h"

#include "acljobbase.h"

namespace KIMAP
{

class Session;
struct Message;
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
 * <a href="http://www.apps.ietf.org/rfc/rfc4314.html">RFC 4314</a>.
 */
class KIMAP_EXPORT DeleteAclJob : public AclJobBase
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(DeleteAclJob)

    friend class SessionPrivate;

public:
    explicit DeleteAclJob(Session *session);
    virtual ~DeleteAclJob();

    /**
     * Sets the identifier to remove
     */
    void setIdentifier(const QByteArray &identifier);
    /**
     * The identifier that will be removed
     */
    QByteArray identifier();

protected:
    void doStart() Q_DECL_OVERRIDE;

};

}

#endif
