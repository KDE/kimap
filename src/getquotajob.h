/*
    SPDX-FileCopyrightText: 2009 Andras Mantia <amantia@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

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
 * <a href="https://tools.ietf.org/html/rfc2087">RFC 2087</a>.
 */
class KIMAP_EXPORT GetQuotaJob : public QuotaJobBase
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(GetQuotaJob)

    friend class SessionPrivate;

public:
    explicit GetQuotaJob(Session *session);
    ~GetQuotaJob() override;

    /**
     * Set the quota root to get the resource limits for.
     * @param root the quota root to set
     * @see GetQuotaRootJob
     */
    void setRoot(const QByteArray &root);
    /**
     * The quota root that resource limit information will be fetched for.
     */
    Q_REQUIRED_RESULT QByteArray root() const;

protected:
    void doStart() override;
    void handleResponse(const Response &response) override;
};

}

