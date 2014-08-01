/*
    Copyright (c) 2009 Kevin Ottens <ervin@kde.org>

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

#include "listjob.h"

#include <boost/bind.hpp>
#include <QtCore/QTimer>
#include <KLocalizedString>

#include "job_p.h"
#include "message_p.h"
#include "rfccodecs.h"
#include "session_p.h"

namespace KIMAP
{
class ListJobPrivate : public JobPrivate
{
public:
    ListJobPrivate(ListJob *job, Session *session, const QString &name) : JobPrivate(session, name), q(job), option(ListJob::NoOption) { }
    ~ListJobPrivate() { }

    void emitPendings()
    {
        if (pendingDescriptors.isEmpty()) {
            return;
        }

        emit q->mailBoxesReceived(pendingDescriptors, pendingFlags);

        pendingDescriptors.clear();
        pendingFlags.clear();
    }

    ListJob *const q;

    ListJob::Option option;
    QList<MailBoxDescriptor> namespaces;
    QByteArray command;

    QTimer emitPendingsTimer;
    QList<MailBoxDescriptor> pendingDescriptors;
    QList< QList<QByteArray> > pendingFlags;
};
}

using namespace KIMAP;

ListJob::ListJob(Session *session)
    : Job(*new ListJobPrivate(this, session, i18n("List")))
{
    Q_D(ListJob);
    connect(&d->emitPendingsTimer, SIGNAL(timeout()),
            this, SLOT(emitPendings()));
}

ListJob::~ListJob()
{
}

void ListJob::setIncludeUnsubscribed(bool include)
{
    Q_D(ListJob);
    if (include) {
        d->option = ListJob::IncludeUnsubscribed;
    } else {
        d->option = ListJob::NoOption;
    }
}

bool ListJob::isIncludeUnsubscribed() const
{
    Q_D(const ListJob);
    return (d->option == ListJob::IncludeUnsubscribed);
}

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

QList<MailBoxDescriptor> ListJob::mailBoxes() const
{
    return QList<MailBoxDescriptor>();
}

QMap< MailBoxDescriptor, QList<QByteArray> > ListJob::flags() const
{
    return QMap< MailBoxDescriptor, QList<QByteArray> >();
}

void ListJob::doStart()
{
    Q_D(ListJob);

    switch (d->option) {
        break;
    case IncludeUnsubscribed:
        d->command = "LIST";
        break;
    case IncludeFolderRoleFlags:
        d->command = "XLIST";
        break;
    case NoOption:
    default:
        d->command = "LSUB";
    }

    d->emitPendingsTimer.start(100);

    if (d->namespaces.isEmpty()) {
        d->tags << d->sessionInternal()->sendCommand(d->command, "\"\" *");
    } else {
        foreach (const MailBoxDescriptor &descriptor, d->namespaces) {
            QString parameters = QStringLiteral("\"\" \"%1\"");

            if (descriptor.name.endsWith(descriptor.separator)) {
                QString name = encodeImapFolderName(descriptor.name);
                name.chop(1);
                d->tags << d->sessionInternal()->sendCommand(d->command,
                        parameters.arg(name).toUtf8());
            }

            d->tags << d->sessionInternal()->sendCommand(d->command,
                    parameters.arg(descriptor.name + QLatin1Char('*')).toUtf8());
        }
    }
}

void ListJob::handleResponse(const Message &response)
{
    Q_D(ListJob);

    // We can predict it'll be handled by handleErrorReplies() so stop
    // the timer now so that result() will really be the last emitted signal.
    if (!response.content.isEmpty() &&
            d->tags.size() == 1 &&
            d->tags.contains(response.content.first().toString())) {
        d->emitPendingsTimer.stop();
        d->emitPendings();
    }

    if (handleErrorReplies(response) == NotHandled) {
        if (response.content.size() >= 5 && response.content[1].toString() == d->command) {
            QList<QByteArray> flags = response.content[2].toList();
            std::transform(flags.begin(), flags.end(), flags.begin(), boost::bind(&QByteArray::toLower, _1));
            QByteArray separator = response.content[3].toString();
            if (separator.isEmpty()) {
                // Defaults to / for servers reporting an empty list
                // it's supposedly not a problem as servers doing that
                // only do it for mailboxes with no child.
                separator = "/"; //krazy:exclude=doublequote_chars since a QByteArray
            }
            Q_ASSERT(separator.size() == 1);
            QByteArray fullName;
            for (int i = 4; i < response.content.size(); i++) {
                fullName += response.content[i].toString() + ' ';
            }
            fullName.chop(1);

            fullName = decodeImapFolderName(fullName);

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
    //Inbox must be case sensitive, according to the RFC, so make it always uppercase
    QStringList pathParts = descriptor.name.split(descriptor.separator);
    if (!pathParts.isEmpty() &&
            pathParts[0].compare(QLatin1String("INBOX"), Qt::CaseInsensitive) == 0) {
        pathParts.removeAt(0);
        descriptor.name = QLatin1String("INBOX");
        if (!pathParts.isEmpty()) {
            descriptor.name += descriptor.separator + pathParts.join(descriptor.separator);
        }
    }
}
#include "moc_listjob.cpp"
