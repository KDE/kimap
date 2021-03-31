/*
    SPDX-FileCopyrightText: 2009 Andras Mantia <amantia@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "job_p.h"
#include "response_p.h"
#include "session.h"

#include <QMap>

namespace KIMAP
{
class QuotaJobBasePrivate : public JobPrivate
{
public:
    QuotaJobBasePrivate(Session *session, const QString &name)
        : JobPrivate(session, name)
    {
    }

    ~QuotaJobBasePrivate()
    {
    }
    static QMap<QByteArray, QPair<qint64, qint64>> readQuota(const Response::Part &content);

    QMap<QByteArray, QPair<qint64, qint64>> quota;
};
}

