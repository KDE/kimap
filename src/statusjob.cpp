/*
    Copyright (c) 2016 Daniel Vrátil <dvratil@kde.org>

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

#include "statusjob.h"
#include "job_p.h"
#include "message_p.h"
#include "session_p.h"
#include "rfccodecs.h"
#include "kimap_debug.h"

#include <KLocalizedString>

namespace KIMAP
{

class StatusJobPrivate : public JobPrivate
{
public:
    explicit StatusJobPrivate(Session *session, const QString &name)
        : JobPrivate(session, name)
    {
    }

    ~StatusJobPrivate()
    {
    }

    QString mailBox;
    QList<QByteArray> dataItems;
    QList<QPair<QByteArray, qint64>> status;
};

}

using namespace KIMAP;

StatusJob::StatusJob(Session *session)
    : Job(*new StatusJobPrivate(session, i18nc("name of the status job", "Status")))
{
}

StatusJob::~StatusJob()
{
}

void StatusJob::setMailBox(const QString &mailBox)
{
    Q_D(StatusJob);
    d->mailBox = mailBox;
}

QString StatusJob::mailBox() const
{
    Q_D(const StatusJob);
    return d->mailBox;
}

void StatusJob::setDataItems(const QList<QByteArray> &dataItems)
{
    Q_D(StatusJob);
    d->dataItems = dataItems;
}

QList<QByteArray> StatusJob::dataItems() const
{
    Q_D(const StatusJob);
    return d->dataItems;
}

QList<QPair<QByteArray, qint64>> StatusJob::status() const
{
    Q_D(const StatusJob);
    return d->status;
}

void StatusJob::doStart()
{
    Q_D(StatusJob);

    const QByteArray params = '\"' + KIMAP::encodeImapFolderName(d->mailBox.toUtf8()) + "\" ("
                            + d->dataItems.join(' ') + ')';

    d->tags << d->sessionInternal()->sendCommand("STATUS", params);
}

void StatusJob::handleResponse(const Message &response)
{
    Q_D(StatusJob);

    if (handleErrorReplies(response) == NotHandled) {
        if (response.content.size() >= 3) {
            const QByteArray code = response.content[1].toString();
            if (code == "STATUS") {

                const QList<QByteArray> resp = response.content[3].toList();
                for (int i = 0; i < resp.size(); i += 2) {
                    d->status << (qMakePair(resp[i], resp[i + 1].toLongLong()));
                }

            } else if (code == "OK") {
                return;
            } else {
                qCDebug(KIMAP_LOG) << response.toString();
            }
        }
    }
}
