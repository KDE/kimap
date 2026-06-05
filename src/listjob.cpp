/*
    SPDX-FileCopyrightText: 2009 Kevin Ottens <ervin@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "listjob.h"

#include <KLocalizedString>
#include <QTimer>

#include "job_p.h"
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

    void emitPendings()
    {
        if (pendingDescriptors.isEmpty()) {
            return;
        }

        Q_EMIT q->mailBoxesReceived(pendingDescriptors, pendingFlags);

        pendingDescriptors.clear();
        pendingFlags.clear();
    }

    ListJob *const q;

    ListJob::Option option = ListJob::NoOption;
    QList<MailBoxDescriptor> namespaces;
    QByteArray command;

    QTimer emitPendingsTimer;
    QList<MailBoxDescriptor> pendingDescriptors;
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
        d->emitPendings();
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

    d->emitPendingsTimer.start(100);

    if (d->namespaces.isEmpty()) {
        d->tags << d->sessionInternal()->sendCommand(d->command, listOptions + "\"\" *");
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
            d->tags << d->sessionInternal()->sendCommand(d->command, listOptions + "\"\" (" + patterns + ')');
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
        d->emitPendings();
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

            d->pendingDescriptors << mailBoxDescriptor;
            d->pendingFlags << flags;
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
#include "moc_listjob.cpp"
