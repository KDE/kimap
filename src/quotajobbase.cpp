/*
    SPDX-FileCopyrightText: 2009 Andras Mantia <amantia@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "quotajobbase.h"
#include "quotajobbase_p.h"
#include "response_p.h"
#include "session_p.h"

#include <KLocalizedString>

using namespace KIMAP;

QMap<QByteArray, QPair<qint64, qint64>> QuotaJobBasePrivate::readQuota(const Response::Part &content)
{
    QMap<QByteArray, QPair<qint64, qint64>> quotaMap;
    QList<QByteArray> quotas = content.toList();

    int i = 0;
    while (i < quotas.size() - 2) {
        QByteArray resource = quotas[i].toUpper();
        qint64 usage = quotas[i + 1].toInt();
        qint64 limit = quotas[i + 2].toInt();
        quotaMap[resource] = qMakePair(usage, limit);
        i += 3;
    }

    return quotaMap;
}

QuotaJobBase::QuotaJobBase(Session *session)
    : Job(*new QuotaJobBasePrivate(session, i18n("QuotaJobBase")))
{
}

QuotaJobBase::QuotaJobBase(JobPrivate &dd)
    : Job(dd)
{
}

QuotaJobBase::~QuotaJobBase()
{
}

qint64 QuotaJobBase::usage(const QByteArray &resource)
{
    Q_D(QuotaJobBase);

    QByteArray r = resource.toUpper();

    if (d->quota.contains(r)) {
        return d->quota[r].first;
    }
    return -1;
}

qint64 QuotaJobBase::limit(const QByteArray &resource)
{
    Q_D(QuotaJobBase);

    QByteArray r = resource.toUpper();

    if (d->quota.contains(r)) {
        return d->quota[r].second;
    }
    return -1;
}
