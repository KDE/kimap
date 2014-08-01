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

#ifndef KIMAP_GETQUOTAJOB_H
#define KIMAP_GETQUOTAJOB_H

#include "quotajobbase.h"

namespace KIMAP
{

class Session;
struct Message;
class GetQuotaJobPrivate;

/**
 * Gets resource limits for a quota root.
 *
 * Quotas are defined with respect to "resources" and "quota roots".
 * A resource is a numerical property that can be limited, such
 * as the octet size of all the messages in a mailbox, or the
 * number of messages in a mailbox.  Each mailbox has one or more
 * quota roots, which are where the resource limits are defined.
 * A quota root may or may not be a mailbox name, and an empty
 * string is a valid quota root.  All mailboxes with the same quota
 * root share the resource limits of the quota root.
 *
 * This job can only be run when the session is in the
 * authenticated (or selected) state.
 *
 * This job requires that the server supports the QUOTA
 * capability, defined in
 * <a href="http://www.apps.ietf.org/rfc/rfc2087.html">RFC 2087</a>.
 */
class KIMAP_EXPORT GetQuotaJob : public QuotaJobBase
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(GetQuotaJob)

    friend class SessionPrivate;

public:
    explicit GetQuotaJob(Session *session);
    virtual ~GetQuotaJob();

    /**
     * Set the quota root to get the resource limits for.
     * @param root the quota root to set
     * @see GetQuotaRootJob
     */
    void setRoot(const QByteArray &root);
    /**
     * The quota root that resource limit information will be fetched for.
     */
    QByteArray root() const;

protected:
    virtual void doStart();
    virtual void handleResponse(const Message &response);

};

}

#endif
