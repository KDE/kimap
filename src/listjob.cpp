/*
    SPDX-FileCopyrightText: 2009 Kevin Ottens <ervin@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "listjob.h"

#include <KLocalizedString>
#include <QTimer>

#include "job_p.h"
#include "kimap_debug.h"
#include "response_p.h"
#include "rfccodecs_p.h"
#include "session_p.h"

namespace KIMAP
{
class ListJobPrivate : public JobPrivate
{
public:
    ListJobPrivate(ListJob *job, Session *session, const QString &name)
        : JobPrivate(session, name)
        , q(job)
    {
    }

    void emitPendings(bool finalEmit)
    {
        if (pendingDescriptors.isEmpty()) {
            return;
        }

        // We should be missing at most one status
        Q_ASSERT(pendingStatus.size() == pendingDescriptors.size() - 1 || pendingStatus.size() == pendingDescriptors.size());

        if (finalEmit) {
            pendingStatus.resize(pendingDescriptors.size());
        }

        // If we are missing a status (emit between LIST and STATUS response)
        if (pendingStatus.size() != pendingDescriptors.size()) {
            const auto readyDescriptors = pendingDescriptors.mid(0, pendingStatus.size());
            const auto readyFlags = pendingFlags.mid(0, pendingStatus.size());
            Q_EMIT q->mailBoxesReceived(readyDescriptors, readyFlags);
            Q_EMIT q->mailBoxesStatusReceived(readyDescriptors, readyFlags, pendingStatus);
            pendingDescriptors.erase(pendingDescriptors.begin(), pendingDescriptors.begin() + pendingStatus.size());
            pendingFlags.erase(pendingFlags.begin(), pendingFlags.begin() + pendingStatus.size());
            pendingStatus.clear();
        } else {
            Q_EMIT q->mailBoxesReceived(pendingDescriptors, pendingFlags);
            Q_EMIT q->mailBoxesStatusReceived(pendingDescriptors, pendingFlags, pendingStatus);
            pendingDescriptors.clear();
            pendingFlags.clear();
            pendingStatus.clear();
        }
    }

    ListJob *const q;

    ListJob::Option option = ListJob::NoOption;
    QList<MailBoxDescriptor> namespaces;
    QByteArray command;
    QByteArrayList returnOptions;

    QTimer emitPendingsTimer;
    QList<MailBoxDescriptor> pendingDescriptors;
    QList<std::optional<ListJob::MailboxStatus>> pendingStatus;
    QList<QList<QByteArray>> pendingFlags;

    bool listExtendedEnabled = false;
};
}

using namespace KIMAP;

ListJob::ListJob(Session *session)
    : Job(*new ListJobPrivate(this, session, i18n("List")))
{
    Q_D(ListJob);
    connect(&d->emitPendingsTimer, &QTimer::timeout, this, [d]() {
        d->emitPendings(false);
    });
}

ListJob::~ListJob() = default;

void ListJob::setOption(Option option)
{
    Q_D(ListJob);
    d->option = option;
}

ListJob::Option ListJob::option() const
{
    Q_D(const ListJob);
    return d->option;
}

void ListJob::setListExtendedEnabled(bool enabled)
{
    Q_D(ListJob);
    d->listExtendedEnabled = enabled;
}

bool ListJob::listExtendedEnabled() const
{
    Q_D(const ListJob);
    return d->listExtendedEnabled;
}

void ListJob::clearReturnOptions()
{
    Q_D(ListJob);
    d->returnOptions.clear();
}

void ListJob::setQueriedNamespaces(const QList<MailBoxDescriptor> &namespaces)
{
    Q_D(ListJob);
    d->namespaces = namespaces;
}

QList<MailBoxDescriptor> ListJob::queriedNamespaces() const
{
    Q_D(const ListJob);
    return d->namespaces;
}

void ListJob::doStart()
{
    Q_D(ListJob);

    auto listOptions = QByteArray();

    switch (d->option) {
    case IncludeUnsubscribed:
        d->command = "LIST";
        break;
    case IncludeFolderRoleFlags:
        d->command = "XLIST";
        break;
    case NoOption:
        if (d->listExtendedEnabled) {
            d->command = "LIST";
            listOptions += "(SUBSCRIBED) ";
        } else {
            d->command = "LSUB";
        }
    }

    auto returnOptions = QByteArray();
    if (!d->returnOptions.isEmpty()) {
        if (d->listExtendedEnabled && d->command == "LIST") {
            returnOptions = " RETURN (" + d->returnOptions.join(' ') + ')';
        } else {
            qCWarning(KIMAP_LOG) << "Return options are only supported with LIST-EXTENDED: ignoring";
        }
    }

    d->emitPendingsTimer.start(100);

    if (d->namespaces.isEmpty()) {
        d->tags << d->sessionInternal()->sendCommand(d->command, listOptions + "\"\" *" + returnOptions);
    } else {
        auto mailboxPatterns = QList<QString>{};
        for (const MailBoxDescriptor &descriptor : std::as_const(d->namespaces)) {
            if (descriptor.name.endsWith(descriptor.separator)) {
                QString name = encodeImapFolderName(descriptor.name, d->sessionInternal()->isUtf8Enabled());
                name.chop(1);
                mailboxPatterns.append(name);
            }
            mailboxPatterns.append(descriptor.name + u'*');
        }

        if (!d->listExtendedEnabled) {
            const auto parameters = QStringLiteral("\"\" \"%1\"");
            for (const auto &pattern : mailboxPatterns) {
                d->tags << d->sessionInternal()->sendCommand(d->command, parameters.arg(pattern).toUtf8());
            }
        } else {
            for (auto &pattern : mailboxPatterns) {
                pattern = QStringLiteral("\"%1\"").arg(pattern);
            }
            auto patterns = mailboxPatterns.join(u' ').toUtf8();
            d->tags << d->sessionInternal()->sendCommand(d->command, listOptions + "\"\" (" + patterns + ')' + returnOptions);
        }
    }
}

void ListJob::handleResponse(const Response &response)
{
    Q_D(ListJob);

    // We can predict it'll be handled by handleErrorReplies() so stop
    // the timer now so that result() will really be the last emitted signal.
    if (!response.content.isEmpty() && d->tags.size() == 1 && d->tags.contains(response.content.first().toString())) {
        d->emitPendingsTimer.stop();
        d->emitPendings(true);
    }

    if (handleErrorReplies(response) == NotHandled) {
        if (response.content.size() >= 5 && response.content[1].toString() == d->command) {
            QList<QByteArray> flags = response.content[2].toList();
            for (QList<QByteArray>::iterator it = flags.begin(), itEnd = flags.end(); it != itEnd; ++it) {
                *it = it->toLower();
            }
            QByteArray separator = response.content[3].toString();
            if (separator.isEmpty()) {
                // Defaults to / for servers reporting an empty list
                // it's supposedly not a problem as servers doing that
                // only do it for mailboxes with no child.
                separator = "/"; // krazy:exclude=doublequote_chars since a QByteArray
            }
            Q_ASSERT(separator.size() == 1);
            QByteArray fullName;
            for (int i = 4; i < response.content.size(); i++) {
                fullName += response.content[i].toString() + ' ';
            }
            fullName.chop(1);

            fullName = decodeImapFolderName(fullName, d->sessionInternal()->isUtf8Enabled());

            MailBoxDescriptor mailBoxDescriptor;
            mailBoxDescriptor.separator = QLatin1Char(separator[0]);
            mailBoxDescriptor.name = QString::fromUtf8(fullName);
            convertInboxName(mailBoxDescriptor);

            // If previous LIST response was STATUS-less, update pendingStatus list
            if (d->pendingStatus.size() < d->pendingDescriptors.size()) {
                d->pendingStatus.resize(d->pendingDescriptors.size());
            }

            d->pendingDescriptors << mailBoxDescriptor;
            d->pendingFlags << flags;
            if (flags.contains(QByteArrayLiteral("\\noselect"))) {
                d->pendingStatus << std::nullopt;
            }
        } else if (response.content.size() >= 4 && response.content[1].toString() == "STATUS") {
            // RFC 5819 guarantees a STATUS, if present, will come after it's corresponding LIST
            Q_ASSERT(!d->pendingDescriptors.empty() && d->pendingDescriptors.size() - 1 == d->pendingStatus.size());
            if (d->pendingDescriptors.empty() || d->pendingDescriptors.size() - 1 != d->pendingStatus.size()) {
                qCWarning(KIMAP_LOG) << "Received an unexpected STATUS response, ignoring...";
                return;
            }

            const auto mailbox = QString::fromUtf8(response.content[2].toString());
            const auto &descriptor = d->pendingDescriptors.last();
            // Handle mailboxes starting with "inbox" that are set uppercase on our side
            const bool startsWithInbox =
                mailbox.startsWith(QStringLiteral("INBOX"), Qt::CaseInsensitive) && descriptor.name.startsWith(QStringLiteral("INBOX"));
            Q_ASSERT((startsWithInbox && mailbox.mid(5) == descriptor.name.mid(5)) || (!startsWithInbox && descriptor.name == mailbox));
            if ((startsWithInbox && mailbox.mid(5) != descriptor.name.mid(5)) || (!startsWithInbox && descriptor.name != mailbox)) {
                qCWarning(KIMAP_LOG) << "Received an unrelated STATUS mailbox `" << mailbox << "`, ignoring...";
                return;
            }

            auto status = MailboxStatus{};
            const auto responseStatus = response.content[3].toList();
            for (int i = 0; i < responseStatus.size(); i += 2) {
                status << (qMakePair(responseStatus[i], responseStatus[i + 1].toLongLong()));
            }
            d->pendingStatus << status;
        }
    }
}

void ListJob::convertInboxName(KIMAP::MailBoxDescriptor &descriptor)
{
    // Inbox must be case sensitive, according to the RFC, so make it always uppercase
    QStringList pathParts = descriptor.name.split(descriptor.separator);
    if (!pathParts.isEmpty() && pathParts[0].compare(QLatin1StringView("INBOX"), Qt::CaseInsensitive) == 0) {
        pathParts.removeAt(0);
        descriptor.name = QStringLiteral("INBOX");
        if (!pathParts.isEmpty()) {
            descriptor.name += descriptor.separator + pathParts.join(descriptor.separator);
        }
    }
}

void ListJob::setReturnOption(const ListReturnOptions::Subscribed &)
{
    Q_D(ListJob);
    d->returnOptions.append("SUBSCRIBED");
}

void ListJob::setReturnOption(const ListReturnOptions::Children &)
{
    Q_D(ListJob);
    d->returnOptions.append("CHILDREN");
}

void ListJob::setReturnOption(const ListReturnOptions::Status &opt)
{
    Q_D(ListJob);
    auto options = QByteArrayList();
    if (opt.messages) {
        options << "MESSAGES";
    }
    if (opt.uidNext) {
        options << "UIDNEXT";
    }
    if (opt.uidValidity) {
        options << "UIDVALIDITY";
    }
    if (opt.unseen) {
        options << "UNSEEN";
    }
    if (opt.deleted) {
        options << "DELETED";
    }
    if (opt.size) {
        options << "SIZE";
    }

    d->returnOptions.append(QByteArray("STATUS (") + options.join(' ') + ')');
}

#include "moc_listjob.cpp"
