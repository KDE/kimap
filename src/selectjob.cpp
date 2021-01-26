/*
    SPDX-FileCopyrightText: 2009 Kevin Ottens <ervin@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "selectjob.h"

#include "kimap_debug.h"

#include "imapset.h"
#include "job_p.h"
#include "response_p.h"
#include "rfccodecs.h"
#include "session_p.h"

#include <KLocalizedString>

#include <QMap>
#include <QTimer>

namespace KIMAP
{
class SelectJobPrivate : public JobPrivate
{
public:
    SelectJobPrivate(SelectJob *q, Session *session, const QString &name)
        : JobPrivate(session, name)
        , q(q)
    {
        QObject::connect(&emitPendingsTimer, &QTimer::timeout, [this]() {
            emitPendings();
        });
    }

    void emitPendings()
    {
        if (pendingMessages.empty()) {
            return;
        }

        Q_EMIT q->modified(pendingMessages);
        pendingMessages.clear();
    }

    QString mailBox;
    bool readOnly = false;

    QMap<qint64, Message> pendingMessages;
    QTimer emitPendingsTimer;

    QList<QByteArray> flags;
    QList<QByteArray> permanentFlags;
    int messageCount = -1;
    int recentCount = -1;
    int firstUnseenIndex = -1;
    qint64 uidValidity = -1;
    qint64 nextUid = -1;
    quint64 highestmodseq = 0;
    qint64 lastUidvalidity = -1;
    quint64 lastModseq = 0;
    ImapSet knownUids;

    bool condstoreEnabled = false;

    SelectJob *const q;
};
}

using namespace KIMAP;

SelectJob::SelectJob(Session *session)
    : Job(*new SelectJobPrivate(this, session, i18nc("name of the select job", "Select")))
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

void SelectJob::setQResync(qint64 lastUidvalidity, quint64 lastModseq, const ImapSet &knownUids)
{
    Q_D(SelectJob);
    d->lastUidvalidity = lastUidvalidity;
    d->lastModseq = lastModseq;
    d->knownUids = knownUids;
    setCondstoreEnabled(true);
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
        // Check whether we only do CONDSTORE, or QRESYNC
        if (d->lastUidvalidity == -1 && d->lastModseq == 0) {
            params += " (CONDSTORE)";
        } else {
            params += " (QRESYNC (" + QByteArray::number(d->lastUidvalidity) + " " + QByteArray::number(d->lastModseq);
            if (!d->knownUids.isEmpty()) {
                params += " " + d->knownUids.toImapSequenceSet();
            }
            params += "))";
        }
    }

    d->emitPendingsTimer.start(100);
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

    // We can predict it'll be handled by handleErrorReplies() so stop
    // the timer now so that result() will really be the last emitted signal.
    if (!response.content.isEmpty() && d->tags.size() == 1 && d->tags.contains(response.content.first().toString())) {
        d->emitPendingsTimer.stop();
        d->emitPendings();
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
            } else if (code == "VANISHED" && response.content.size() == 4) { // VANISHED response in SELECT is RFC5162 (QRESYNC) extension
                const auto vanishedSet = ImapSet::fromImapSequenceSet(response.content[3].toString());
                Q_EMIT vanished(vanishedSet);
            } else {
                bool isInt = false;
                int value = response.content[1].toString().toInt(&isInt);
                if (!isInt || response.content.size() < 3) {
                    return;
                }

                code = response.content[2].toString();
                if (code == "FETCH") { // FETCH response in SELECT is RFC5162 (QRESYNC) extension
                    Message msg{};
                    const auto content = response.content[3].toList();
                    for (auto it = content.cbegin(), end = content.cend(); it != end; ++it) {
                        const auto name = *it;
                        ++it;

                        if (it == content.constEnd()) { // Uh oh, message was truncated?
                            qCWarning(KIMAP_LOG) << "SELECT reply got truncated, skipping.";
                            break;
                        }

                        if (name == "UID") {
                            msg.uid = it->toLongLong();
                        } else if (name == "FLAGS") {
                            if ((*it).startsWith('(') && (*it).endsWith(')')) {
                                QByteArray str = *it;
                                str.chop(1);
                                str.remove(0, 1);
                                const auto flags = str.split(' ');
                                msg.flags = flags;
                            } else {
                                msg.flags << *it;
                            }
                        } else if (name == "MODSEQ") {
                            QByteArray modseq = *it;
                            if (modseq.startsWith('(') && modseq.endsWith(')')) {
                                modseq.chop(1);
                                modseq.remove(0, 1);
                            }

                            msg.attributes.insert(name, modseq.toULongLong());
                        }
                    }

                    d->pendingMessages.insert(value, msg);
                } else if (code == "EXISTS") {
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
