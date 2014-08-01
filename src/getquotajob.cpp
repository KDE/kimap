/*
    Copyright (c) 2009 Andras Mantia <amantia@kde.org>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

#include "getquotajob.h"

#include <KLocalizedString>

#include "quotajobbase_p.h"
#include "message_p.h"
#include "session_p.h"

namespace KIMAP
{
class GetQuotaJobPrivate : public QuotaJobBasePrivate
{
public:
    GetQuotaJobPrivate(Session *session, const QString &name) : QuotaJobBasePrivate(session, name) { }
    ~GetQuotaJobPrivate() { }

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
    //XXX: [alexmerry, 2010-07-24]: should d->root be quoted properly?
    d->tags << d->sessionInternal()->sendCommand("GETQUOTA", '\"' + d->root + '\"');
}

void GetQuotaJob::handleResponse(const Message &response)
{
    Q_D(GetQuotaJob);
    if (handleErrorReplies(response) == NotHandled) {
        if (response.content.size() >= 4 &&
                response.content[1].toString() == "QUOTA") {
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
