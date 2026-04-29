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

/*!
 * \class KIMAP::GetQuotaJob
 * \inmodule KIMAP
 * \inheaderfile KIMAP/GetQuotaJob
 *
 * \brief Gets resource limits for a quota root.
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
 * \l{https://tools.ietf.org/html/rfc2087}{RFC 2087}.
 */
class KIMAP_EXPORT GetQuotaJob : public QuotaJobBase
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(GetQuotaJob)

    friend class SessionPrivate;

public:
    /*!
     *
     */
    explicit GetQuotaJob(Session *session);
    ~GetQuotaJob() override;

    /*!
     * Set the quota root to get the resource limits for.
     *
     * A quota root may be a mailbox name (RFC 9208/2087); pass it in
     * decoded Unicode form. The job encodes for the wire as needed,
     * using mUTF-7 (RFC 3501) for plain IMAP4rev1 sessions and raw
     * UTF-8 for IMAP4rev2 (RFC 9051) or sessions with UTF8=ACCEPT
     * enabled (RFC 9755).
     *
     * \a root the quota root to set
     *
     * \sa GetQuotaRootJob
     */
    void setRoot(const QString &root);
    /*!
     * The quota root that resource limit information will be fetched for,
     * in decoded Unicode form.
     */
    [[nodiscard]] QString root() const;

protected:
    void doStart() override;
    void handleResponse(const Response &response) override;
};

}
