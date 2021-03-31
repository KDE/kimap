/*
    SPDX-FileCopyrightText: 2015 Christian Mollekopf <mollekopf@kolabsys.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "kimap_export.h"

#include "job.h"

namespace KIMAP
{
class Session;
struct Message;
class IdJobPrivate;

/**
 * Reports client id.
 *
 * This job can be run in any open session.
 */
class KIMAP_EXPORT IdJob : public Job
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(IdJob)

    friend class SessionPrivate;

public:
    IdJob(Session *session);
    ~IdJob() override;

    void setField(const QByteArray &name, const QByteArray &field);

protected:
    void doStart() override;
    void handleResponse(const Response &response) override;
};

}

