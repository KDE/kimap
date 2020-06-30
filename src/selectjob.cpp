/*
    SPDX-FileCopyrightText: 2009 Kevin Ottens <ervin@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "selectjob.h"

#include <KLocalizedString>
#include "kimap_debug.h"

#include "job_p.h"
#include "response_p.h"
#include "session_p.h"
#include "rfccodecs.h"

namespace KIMAP
{
class SelectJobPrivate : public JobPrivate
{
public:
    SelectJobPrivate(Session *session, const QString &name)
        : JobPrivate(session, name), readOnly(false), messageCount(-1), recentCount(-1),
          firstUnseenIndex(-1), uidValidity(-1), nextUid(-1), highestmodseq(0),
          condstoreEnabled(false) { }
    ~SelectJobPrivate() { }

    QString mailBox;
    bool readOnly;

    QList<QByteArray> flags;
    QList<QByteArray> permanentFlags;
    int messageCount;
    int recentCount;
    int firstUnseenIndex;
    qint64 uidValidity;
    qint64 nextUid;
    quint64 highestmodseq;
    bool condstoreEnabled;
};
}

using namespace KIMAP;

SelectJob::SelectJob(Session *session)
    : Job(*new SelectJobPrivate(session, i18nc("name of the select job", "Select")))
{
}

SelectJob::~SelectJob()
{
}

void SelectJob::setMailBox(const QString &mailBox)
{
    Q_D(SelectJob);
    d->mailBox = mailBox;
}

QString SelectJob::mailBox() const
{
    Q_D(const SelectJob);
    return d->mailBox;
}

void SelectJob::setOpenReadOnly(bool readOnly)
{
    Q_D(SelectJob);
    d->readOnly = readOnly;
}

bool SelectJob::isOpenReadOnly() const
{
    Q_D(const SelectJob);
    return d->readOnly;
}

QList<QByteArray> SelectJob::flags() const
{
    Q_D(const SelectJob);
    return d->flags;
}

QList<QByteArray> SelectJob::permanentFlags() const
{
    Q_D(const SelectJob);
    return d->permanentFlags;
}

int SelectJob::messageCount() const
{
    Q_D(const SelectJob);
    return d->messageCount;
}

int SelectJob::recentCount() const
{
    Q_D(const SelectJob);
    return d->recentCount;
}

int SelectJob::firstUnseenIndex() const
{
    Q_D(const SelectJob);
    return d->firstUnseenIndex;
}

qint64 SelectJob::uidValidity() const
{
    Q_D(const SelectJob);
    return d->uidValidity;
}

qint64 SelectJob::nextUid() const
{
    Q_D(const SelectJob);
    return d->nextUid;
}

quint64 SelectJob::highestModSequence() const
{
    Q_D(const SelectJob);
    return d->highestmodseq;
}

void SelectJob::setCondstoreEnabled(bool enable)
{
    Q_D(SelectJob);
    d->condstoreEnabled = enable;
}

bool SelectJob::condstoreEnabled() const
{
    Q_D(const SelectJob);
    return d->condstoreEnabled;
}

void SelectJob::doStart()
{
    Q_D(SelectJob);

    QByteArray command = "SELECT";
    if (d->readOnly) {
        command = "EXAMINE";
    }

    QByteArray params = '\"' + KIMAP::encodeImapFolderName(d->mailBox.toUtf8()) + '\"';

    if (d->condstoreEnabled) {
        params += " (CONDSTORE)";
    }

    d->tags << d->sessionInternal()->sendCommand(command, params);
}

void SelectJob::handleResponse(const Response &response)
{
    Q_D(SelectJob);

    // Check for [READ-ONLY] response in final tagged OK
    // This must be checked before handleErrorReplies(), because that calls emitResult()
    // right away
    if (!response.content.isEmpty() && d->tags.contains(response.content.first().toString())) {
        if (response.responseCode.size() >= 1 && response.responseCode[0].toString() == "READ-ONLY") {
            d->readOnly = true;
        }
    }

    if (handleErrorReplies(response) == NotHandled) {
        if (response.content.size() >= 2) {
            QByteArray code = response.content[1].toString();

            if (code == "OK") {
                if (response.responseCode.size() < 2) {
                    return;
                }

                code = response.responseCode[0].toString();
                if (code == "PERMANENTFLAGS") {
                    d->permanentFlags = response.responseCode[1].toList();
                } else if (code == "HIGHESTMODSEQ") {
                    bool isInt;
                    quint64 value = response.responseCode[1].toString().toULongLong(&isInt);
                    if (!isInt) {
                        return;
                    }
                    d->highestmodseq = value;
                } else {
                    bool isInt;
                    qint64 value = response.responseCode[1].toString().toLongLong(&isInt);
                    if (!isInt) {
                        return;
                    }
                    if (code == "UIDVALIDITY") {
                        d->uidValidity = value;
                    } else if (code == "UNSEEN") {
                        d->firstUnseenIndex = value;
                    } else if (code == "UIDNEXT") {
                        d->nextUid = value;
                    }
                }
            } else if (code == "FLAGS") {
                d->flags = response.content[2].toList();
            } else {
                bool isInt;
                int value = response.content[1].toString().toInt(&isInt);
                if (!isInt || response.content.size() < 3) {
                    return;
                }

                code = response.content[2].toString();
                if (code == "EXISTS") {
                    d->messageCount = value;
                } else if (code == "RECENT") {
                    d->recentCount = value;
                }
            }
        } else {
            qCDebug(KIMAP_LOG) << response.toString();
        }
    } else {
        Q_ASSERT(error() || d->m_session->selectedMailBox() == d->mailBox);
    }
}
