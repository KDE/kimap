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
class GetQuotaRootJobPrivate;

/*!
 * Gets the quota root and resource limits for a mailbox.
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
class KIMAP_EXPORT GetQuotaRootJob : public QuotaJobBase
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(GetQuotaRootJob)

    friend class SessionPrivate;

public:
    explicit GetQuotaRootJob(Session *session);
    ~GetQuotaRootJob() override;

    /*!
     * Set the mailbox to get the quota roots for.
     *
     * \a mailBox  the name of an existing mailbox
     */
    void setMailBox(const QString &mailBox);
    /*!
     * The mailbox that the quota roots will be fetched for.
     */
    [[nodiscard]] QString mailBox() const;

    /*!
     * The quota roots for the mailbox.
     */
    [[nodiscard]] QList<QByteArray> roots() const;
    /*!
     * Get the current usage for a resource.
     *
     * Note that if there is no limit for a resource, the
     * server will not provide information about resource
     * usage.
     *
     * \a root      the quota root to get the resource usage for
     * \a resource  the resource to get the usage for
     * Returns  the resource usage in appropriate units, or -1
     *          if the usage is unknown or there is no
     *          limit on the resource
     */
    [[nodiscard]] qint64 usage(const QByteArray &root, const QByteArray &resource) const;
    /*!
     * Get the current limit for a resource.
     *
     * \a root      the quota root to get the resource limit for
     * \a resource  the resource to get the limit for
     * Returns  the resource limit in appropriate units, or -1
     *          if the limit is unknown or there is no
     *          limit on the resource
     */
    [[nodiscard]] qint64 limit(const QByteArray &root, const QByteArray &resource) const;

    /*!
     * Get a map containing all resource usage figures for a quota root.
     *
     * \a root  the quota root to get resource usage figures for
     * Returns  a map from resource names to usage figures
     */
    [[nodiscard]] QMap<QByteArray, qint64> allUsages(const QByteArray &root) const;
    /*!
     * Get a map containing all resource limits for a quota root.
     *
     * \a root  the quota root to get resource limits for
     * Returns  a map from resource names to limits
     */
    [[nodiscard]] QMap<QByteArray, qint64> allLimits(const QByteArray &root) const;

protected:
    void doStart() override;
    void handleResponse(const Response &response) override;
};

}
