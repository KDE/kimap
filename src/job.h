/*
    SPDX-FileCopyrightText: 2009 Kevin Ottens <ervin@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "kimap_export.h"

#include <KJob>

namespace KIMAP
{
class Session;
class SessionPrivate;
class JobPrivate;
struct Response;

/*!
 * \class KIMAP::Job
 * \inmodule KIMAP
 * \inheaderfile KIMAP/Job
 *
 * \brief Base class for all KIMAP jobs.
 */
class KIMAP_EXPORT Job : public KJob
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(Job)

    friend class SessionPrivate;

public:
    ~Job() override;

    [[nodiscard]] Session *session() const;

    void start() override;

private:
    virtual void doStart() = 0;
    virtual void handleResponse(const Response &response);
    virtual void connectionLost();

protected:
    enum HandlerResponse {
        Handled = 0,
        NotHandled
    };

    HandlerResponse handleErrorReplies(const Response &response);

    KIMAP_NO_EXPORT explicit Job(Session *session);
    // exported for MockJob
    explicit Job(JobPrivate &dd);

    JobPrivate *const d_ptr;
};

}
