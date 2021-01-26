/*
    SPDX-FileCopyrightText: 2009 Kevin Ottens <ervin@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "storejob.h"

#include "kimap_debug.h"
#include <KLocalizedString>

#include "job_p.h"
#include "response_p.h"
#include "session_p.h"

namespace KIMAP
{
class StoreJobPrivate : public JobPrivate
{
public:
    StoreJobPrivate(Session *session, const QString &name)
        : JobPrivate(session, name)
        , uidBased(false)
    {
    }
    ~StoreJobPrivate()
    {
    }

    QByteArray addFlags(const QByteArray &param, const MessageFlags &flags)
    {
        QByteArray parameters;
        switch (mode) {
        case StoreJob::SetFlags:
            parameters += param;
            break;
        case StoreJob::AppendFlags:
            parameters += "+" + param;
            break;
        case StoreJob::RemoveFlags:
            parameters += "-" + param;
            break;
        }

        parameters += " (";
        for (const QByteArray &flag : flags) {
            parameters += flag + ' ';
        }
        if (!flags.isEmpty()) {
            parameters.chop(1);
        }
        parameters += ')';

        return parameters;
    }

    ImapSet set;
    bool uidBased;
    StoreJob::StoreMode mode;
    MessageFlags flags;
    MessageFlags gmLabels;

    QMap<int, MessageFlags> resultingFlags;
};
}

using namespace KIMAP;

StoreJob::StoreJob(Session *session)
    : Job(*new StoreJobPrivate(session, i18n("Store")))
{
    Q_D(StoreJob);
    d->uidBased = false;
    d->mode = SetFlags;
}

StoreJob::~StoreJob()
{
}

void StoreJob::setSequenceSet(const ImapSet &set)
{
    Q_D(StoreJob);
    d->set = set;
}

ImapSet StoreJob::sequenceSet() const
{
    Q_D(const StoreJob);
    return d->set;
}

void StoreJob::setUidBased(bool uidBased)
{
    Q_D(StoreJob);
    d->uidBased = uidBased;
}

bool StoreJob::isUidBased() const
{
    Q_D(const StoreJob);
    return d->uidBased;
}

void StoreJob::setFlags(const MessageFlags &flags)
{
    Q_D(StoreJob);
    d->flags = flags;
}

MessageFlags StoreJob::flags() const
{
    Q_D(const StoreJob);
    return d->flags;
}

void StoreJob::setGMLabels(const MessageFlags &gmLabels)
{
    Q_D(StoreJob);
    d->gmLabels = gmLabels;
}

MessageFlags StoreJob::gmLabels() const
{
    Q_D(const StoreJob);
    return d->gmLabels;
}

void StoreJob::setMode(StoreMode mode)
{
    Q_D(StoreJob);
    d->mode = mode;
}

StoreJob::StoreMode StoreJob::mode() const
{
    Q_D(const StoreJob);
    return d->mode;
}

QMap<int, MessageFlags> StoreJob::resultingFlags() const
{
    Q_D(const StoreJob);
    return d->resultingFlags;
}

void StoreJob::doStart()
{
    Q_D(StoreJob);

    if (d->set.isEmpty()) {
        qCWarning(KIMAP_LOG) << "Empty uid set passed to store job";
        setError(KJob::UserDefinedError);
        setErrorText(QStringLiteral("Empty uid set passed to store job"));
        emitResult();
        return;
    }

    d->set.optimize();
    QByteArray parameters = d->set.toImapSequenceSet() + ' ';

    if (!d->flags.isEmpty()) {
        parameters += d->addFlags("FLAGS", d->flags);
    }
    if (!d->gmLabels.isEmpty()) {
        if (!d->flags.isEmpty()) {
            parameters += ' ';
        }
        parameters += d->addFlags("X-GM-LABELS", d->gmLabels);
    }

    qCDebug(KIMAP_LOG) << parameters;

    QByteArray command = "STORE";
    if (d->uidBased) {
        command = "UID " + command;
    }

    d->tags << d->sessionInternal()->sendCommand(command, parameters);
}

void StoreJob::handleResponse(const Response &response)
{
    Q_D(StoreJob);

    if (handleErrorReplies(response) == NotHandled) {
        if (response.content.size() == 4 && response.content[2].toString() == "FETCH" && response.content[3].type() == Response::Part::List) {
            int id = response.content[1].toString().toInt();
            qint64 uid = 0;
            bool uidFound = false;
            QList<QByteArray> resultingFlags;

            QList<QByteArray> content = response.content[3].toList();

            for (QList<QByteArray>::ConstIterator it = content.constBegin(); it != content.constEnd(); ++it) {
                QByteArray str = *it;
                ++it;

                if (str == "FLAGS") {
                    if ((*it).startsWith('(') && (*it).endsWith(')')) {
                        QByteArray str = *it;
                        str.chop(1);
                        str.remove(0, 1);
                        resultingFlags = str.split(' ');
                    } else {
                        resultingFlags << *it;
                    }
                } else if (str == "UID") {
                    uid = it->toLongLong(&uidFound);
                }
            }

            if (!d->uidBased) {
                d->resultingFlags[id] = resultingFlags;
            } else if (uidFound) {
                d->resultingFlags[uid] = resultingFlags;
            } else {
                qCWarning(KIMAP_LOG) << "We asked for UID but the server didn't give it back, resultingFlags not stored.";
            }
        }
    }
}
