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

#ifndef KIMAP_QUOTAJOBBASE_H
#define KIMAP_QUOTAJOBBASE_H

#include "kimap_export.h"

#include "job.h"

namespace KIMAP
{

class Session;
struct Message;
class QuotaJobBasePrivate;

/**
 * Base class for jobs that operate on mailbox quotas
 *
 * Provides support for the IMAP QUOTA extension, as defined by
 * <a href="http://www.apps.ietf.org/rfc/rfc2087.html" title="IMAP QUOTA extension">RFC 2087</a>.
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
    virtual ~QuotaJobBase();

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
    qint64 usage(const QByteArray &resource);
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
    qint64 limit(const QByteArray &resource);

protected:
    QuotaJobBase(JobPrivate &dd);

};

}

#endif
