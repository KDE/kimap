/*
    SPDX-FileCopyrightText: 2009 Andras Mantia <amantia@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "kimap_export.h"

#include "job.h"

namespace KIMAP
{
class Session;
struct Response;
class QuotaJobBasePrivate;

/**
 * Base class for jobs that operate on mailbox quotas
 *
 * Provides support for the IMAP QUOTA extension, as defined by
 * <a href="https://tools.ietf.org/html/rfc2087" title="IMAP QUOTA extension">RFC 2087</a>.
 *
 * This class cannot be used directly, you must subclass it and reimplement
 * at least the doStart() method.
 */
class KIMAP_EXPORT QuotaJobBase : public Job
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QuotaJobBase)

    friend class SessionPrivate;

public:
    explicit QuotaJobBase(Session *session);
    ~QuotaJobBase() override;

    /**
     * Get the current usage for a resource.
     *
     * All quota jobs will normally cause the server to return
     * details of resource usage for all resources that were
     * queried or modified by the job.
     *
     * Note that RFC 2087 is slightly ambiguous about whether
     * SETQUOTA will cause this information to be sent by the
     * server.
     *
     * Note that if there is no limit for a resource, the
     * server will not provide information about resource
     * usage.
     *
     * @param resource  the resource to get the usage for
     * @return  the resource usage in appropriate units, or -1
     *          if the usage is unknown or there is no
     *          limit on the resource
     */
    Q_REQUIRED_RESULT qint64 usage(const QByteArray &resource);
    /**
     * Get the current limit for a resource.
     *
     * All quota jobs will normally cause the server to return
     * details of resource limits for all resources that were
     * queried or modified by the job.
     *
     * Note that RFC 2087 is slightly ambiguous about whether
     * SETQUOTA will cause this information to be sent by the
     * server.
     *
     * @param resource  the resource to get the limit for
     * @return  the resource limit in appropriate units, or -1
     *          if the limit is unknown or there is no limit
     *          on the resource
     */
    Q_REQUIRED_RESULT qint64 limit(const QByteArray &resource);

protected:
    QuotaJobBase(JobPrivate &dd);
};

}

