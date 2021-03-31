/*
    SPDX-FileCopyrightText: 2009 Kevin Ottens <ervin@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "kimap_export.h"

#include "job.h"

namespace KIMAP
{
class Session;
struct Response;
class CapabilitiesJobPrivate;

/**
 * Checks server capabilities.
 *
 * This job can be run in any open session.
 *
 * This simply asks the server what capabilities it supports
 * (using the CAPABILITY command) and returns the list
 * provided by the server.  The list may, therefore, be
 * inaccurate: the server may claim to support something
 * it does not implement properly, or it may omit a feature
 * that it does, in reality, support.
 */
class KIMAP_EXPORT CapabilitiesJob : public Job
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(CapabilitiesJob)

    friend class SessionPrivate;

public:
    CapabilitiesJob(Session *session);
    ~CapabilitiesJob() override;

    /**
     * The capabilities the server claims to support.
     *
     * This will return an empty list until the job has completed.
     */
    Q_REQUIRED_RESULT QStringList capabilities() const;

Q_SIGNALS:
    /**
     * Notifies listeners that the capabilities have been fetched.
     *
     * @param capabilities  The capabilities the server claims to support.
     */
    void capabilitiesReceived(const QStringList &capabilities);

protected:
    void doStart() override;
    void handleResponse(const Response &response) override;
};

}

