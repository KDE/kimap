/*
    SPDX-FileCopyrightText: 2009 Kevin Ottens <ervin@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KIMAP_LOGOUTJOB_H
#define KIMAP_LOGOUTJOB_H

#include "kimap_export.h"

#include "job.h"

namespace KIMAP
{
class Session;
class LogoutJobPrivate;

class KIMAP_EXPORT LogoutJob : public Job
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(LogoutJob)

    friend class SessionPrivate;

public:
    explicit LogoutJob(Session *session);
    ~LogoutJob() override;

protected:
    void doStart() override;
    void connectionLost() override;
};

}

#endif
