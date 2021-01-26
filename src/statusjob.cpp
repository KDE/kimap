/*
    SPDX-FileCopyrightText: 2016 Daniel Vrátil <dvratil@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "statusjob.h"
#include "job_p.h"
#include "kimap_debug.h"
#include "response_p.h"
#include "rfccodecs.h"
#include "session_p.h"

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

    const QByteArray params = '\"' + KIMAP::encodeImapFolderName(d->mailBox.toUtf8()) + "\" (" + d->dataItems.join(' ') + ')';

    d->tags << d->sessionInternal()->sendCommand("STATUS", params);
}

void StatusJob::handleResponse(const Response &response)
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
