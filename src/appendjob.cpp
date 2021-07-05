/*
    SPDX-FileCopyrightText: 2009 Kevin Ottens <ervin@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "appendjob.h"

#include <KLocalizedString>

#include "job_p.h"
#include "response_p.h"
#include "rfccodecs.h"
#include "session_p.h"

namespace KIMAP
{
class AppendJobPrivate : public JobPrivate
{
public:
    AppendJobPrivate(Session *session, const QString &name)
        : JobPrivate(session, name)
    {
    }
    ~AppendJobPrivate()
    {
    }

    QString mailBox;
    QList<QByteArray> flags;
    QDateTime internalDate;
    QByteArray content;
    qint64 uid = 0;
};
}

using namespace KIMAP;

AppendJob::AppendJob(Session *session)
    : Job(*new AppendJobPrivate(session, i18n("Append")))
{
}

AppendJob::~AppendJob()
{
}

void AppendJob::setMailBox(const QString &mailBox)
{
    Q_D(AppendJob);
    d->mailBox = mailBox;
}

QString AppendJob::mailBox() const
{
    Q_D(const AppendJob);
    return d->mailBox;
}

void AppendJob::setFlags(const QList<QByteArray> &flags)
{
    Q_D(AppendJob);
    d->flags = flags;
}

QList<QByteArray> AppendJob::flags() const
{
    Q_D(const AppendJob);
    return d->flags;
}

void AppendJob::setInternalDate(const QDateTime &internalDate)
{
    Q_D(AppendJob);
    d->internalDate = internalDate;
}

QDateTime AppendJob::internalDate() const
{
    Q_D(const AppendJob);
    return d->internalDate;
}

void AppendJob::setContent(const QByteArray &content)
{
    Q_D(AppendJob);
    d->content = content;
}

QByteArray AppendJob::content() const
{
    Q_D(const AppendJob);
    return d->content;
}

qint64 AppendJob::uid() const
{
    Q_D(const AppendJob);
    return d->uid;
}

void AppendJob::doStart()
{
    Q_D(AppendJob);

    QByteArray parameters = '\"' + KIMAP::encodeImapFolderName(d->mailBox.toUtf8()) + '\"';

    if (!d->flags.isEmpty()) {
        parameters += " (";
        for (const QByteArray &flag : std::as_const(d->flags)) {
            parameters += flag + ' ';
        }
        parameters.chop(1);
        parameters += ')';
    }

    if (!d->internalDate.isNull()) {
        const QDateTime utcDateTime = d->internalDate.toUTC();
        parameters += " \"" + QLocale::c().toString(utcDateTime, QStringLiteral("dd-MMM-yyyy hh:mm:ss")).toLatin1() + " +0000" + '\"';
    }

    parameters += " {" + QByteArray::number(d->content.size()) + '}';

    d->tags << d->sessionInternal()->sendCommand("APPEND", parameters);
}

void AppendJob::handleResponse(const Response &response)
{
    Q_D(AppendJob);
    const QList<Response::Part>::ConstIterator end(response.responseCode.end());
    for (QList<Response::Part>::ConstIterator it = response.responseCode.begin(); it != end; ++it) {
        if (it->toString() == "APPENDUID") {
            it = it + 2;
            if (it != end) {
                d->uid = it->toString().toLongLong();
            }
            break;
        }
    }

    if (handleErrorReplies(response) == NotHandled) {
        if (!response.content.isEmpty() && response.content[0].toString() == "+") {
            d->sessionInternal()->sendData(d->content);
        }
    }
}
