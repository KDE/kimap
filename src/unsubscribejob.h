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
class UnsubscribeJobPrivate;

class KIMAP_EXPORT UnsubscribeJob : public Job
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(UnsubscribeJob)

    friend class SessionPrivate;

public:
    explicit UnsubscribeJob(Session *session);
    ~UnsubscribeJob() override;

    void setMailBox(const QString &mailBox);
    Q_REQUIRED_RESULT QString mailBox() const;

protected:
    void doStart() override;
};

}

