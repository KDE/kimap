/*
    SPDX-FileCopyrightText: 2009 Andras Mantia <amantia@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "quotajobbase.h"

namespace KIMAP
{
class Session;
struct Response;
class SetQuotaJobPrivate;

/**
 * Sets resource limits on a quota root.
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
class KIMAP_EXPORT SetQuotaJob : public QuotaJobBase
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(SetQuotaJob)

    friend class SessionPrivate;

public:
    explicit SetQuotaJob(Session *session);
    ~SetQuotaJob() override;

    /**
     * Set a limit for a quota resource.
     *
     * For example, you might set the limit for "STORAGE" to
     * 512 to limit the sum of the messages' RFC822.SIZE to
     * 512*1024 octets (ie: 512 kb), or the limit for "MESSAGE"
     * to 100 to limit the number of messages to 100.
     *
     * Note that although RFC 2087 allows a resource name to
     * be any string, this API actually limits resource names
     * to upper-case atoms.  In practice, resource names will
     * almost certainly be composed entirely of upper-case latin
     * letters (A-Z).
     *
     * @param resource  the resource name
     * @param limit     the maximum value the resource may take
     */
    void setQuota(const QByteArray &resource, qint64 limit);

    /**
     * Set the quota root the resource limits should be set for.
     *
     * Note: if the quota root does not already exist, the server
     * may create it and change the quota roots for any number of
     * existing mailboxes in an implementation-defined manner.
     *
     * @param root the quota root to set, in bytes
     * @see GetQuotaRootJob
     */
    void setRoot(const QByteArray &root);
    /**
     * The quota root that will be modified.
     */
    Q_REQUIRED_RESULT QByteArray root() const;

protected:
    void doStart() override;
    void handleResponse(const Response &response) override;
};

}

