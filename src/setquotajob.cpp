/*
    SPDX-FileCopyrightText: 2009 Andras Mantia <amantia@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "setquotajob.h"

#include "kimap_debug.h"
#include <KLocalizedString>

#include "quotajobbase_p.h"
#include "response_p.h"
#include "session_p.h"

namespace KIMAP
{
class SetQuotaJobPrivate : public QuotaJobBasePrivate
{
public:
    SetQuotaJobPrivate(Session *session, const QString &name)
        : QuotaJobBasePrivate(session, name)
    {
    }
    ~SetQuotaJobPrivate()
    {
    }

    QMap<QByteArray, qint64> setList;
    QByteArray root;
};
}

using namespace KIMAP;

SetQuotaJob::SetQuotaJob(Session *session)
    : QuotaJobBase(*new SetQuotaJobPrivate(session, i18n("SetQuota")))
{
}

SetQuotaJob::~SetQuotaJob()
{
}

void SetQuotaJob::doStart()
{
    Q_D(SetQuotaJob);
    QByteArray s;
    s += '(';
    for (QMap<QByteArray, qint64>::ConstIterator it = d->setList.constBegin(), end = d->setList.constEnd(); it != end; ++it) {
        s += it.key() + ' ' + QByteArray::number(it.value()) + ' ';
    }
    if (d->setList.isEmpty()) {
        s += ')';
    } else {
        s[s.length() - 1] = ')';
    }

    qCDebug(KIMAP_LOG) << "SETQUOTA " << '\"' + d->root + "\" " + s;
    // XXX: [alexmerry, 2010-07-24]: should d->root be quoted properly?
    d->tags << d->sessionInternal()->sendCommand("SETQUOTA", '\"' + d->root + "\" " + s);
}

void SetQuotaJob::handleResponse(const Response &response)
{
    Q_D(SetQuotaJob);
    if (handleErrorReplies(response) == NotHandled) {
        if (response.content.size() >= 4 && response.content[1].toString() == "QUOTA") {
            d->quota = d->readQuota(response.content[3]);
        }
    }
}

void SetQuotaJob::setQuota(const QByteArray &resource, qint64 limit)
{
    Q_D(SetQuotaJob);

    d->setList[resource.toUpper()] = limit;
}

void SetQuotaJob::setRoot(const QByteArray &root)
{
    Q_D(SetQuotaJob);

    d->root = root;
}

QByteArray SetQuotaJob::root() const
{
    Q_D(const SetQuotaJob);

    return d->root;
}
