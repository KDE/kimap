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
class SubscribeJobPrivate;

class KIMAP_EXPORT SubscribeJob : public Job
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(SubscribeJob)

    friend class SessionPrivate;

public:
    explicit SubscribeJob(Session *session);
    ~SubscribeJob() override;

    void setMailBox(const QString &mailBox);
    Q_REQUIRED_RESULT QString mailBox() const;

protected:
    void doStart() override;
};

}

