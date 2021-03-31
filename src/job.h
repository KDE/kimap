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

class KIMAP_EXPORT Job : public KJob
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(Job)

    friend class SessionPrivate;

public:
    ~Job() override;

    Session *session() const;

    void start() override;

private:
    virtual void doStart() = 0;
    virtual void handleResponse(const Response &response);
    virtual void connectionLost();

protected:
    enum HandlerResponse { Handled = 0, NotHandled };

    HandlerResponse handleErrorReplies(const Response &response);

    explicit Job(Session *session);
    explicit Job(JobPrivate &dd);

    JobPrivate *const d_ptr;
};

}

