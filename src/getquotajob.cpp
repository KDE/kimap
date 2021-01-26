/*
    SPDX-FileCopyrightText: 2009 Andras Mantia <amantia@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "getquotajob.h"

#include <KLocalizedString>

#include "quotajobbase_p.h"
#include "response_p.h"
#include "session_p.h"

namespace KIMAP
{
class GetQuotaJobPrivate : public QuotaJobBasePrivate
{
public:
    GetQuotaJobPrivate(Session *session, const QString &name)
        : QuotaJobBasePrivate(session, name)
    {
    }
    ~GetQuotaJobPrivate()
    {
    }

    QByteArray root;
};
}

using namespace KIMAP;

GetQuotaJob::GetQuotaJob(Session *session)
    : QuotaJobBase(*new GetQuotaJobPrivate(session, i18n("GetQuota")))
{
}

GetQuotaJob::~GetQuotaJob()
{
}

void GetQuotaJob::doStart()
{
    Q_D(GetQuotaJob);
    // XXX: [alexmerry, 2010-07-24]: should d->root be quoted properly?
    d->tags << d->sessionInternal()->sendCommand("GETQUOTA", '\"' + d->root + '\"');
}

void GetQuotaJob::handleResponse(const Response &response)
{
    Q_D(GetQuotaJob);
    if (handleErrorReplies(response) == NotHandled) {
        if (response.content.size() >= 4 && response.content[1].toString() == "QUOTA") {
            d->quota = d->readQuota(response.content[3]);
        }
    }
}

void GetQuotaJob::setRoot(const QByteArray &root)
{
    Q_D(GetQuotaJob);
    d->root = root;
}

QByteArray GetQuotaJob::root() const
{
    Q_D(const GetQuotaJob);
    return d->root;
}
