/*
    SPDX-FileCopyrightText: 2009 Kevin Ottens <ervin@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "namespacejob.h"

#include "kimap_debug.h"
#include <KLocalizedString>

#include "imapstreamparser.h"
#include "job_p.h"
#include "listjob.h"
#include "response_p.h"
#include "rfccodecs.h"
#include "session_p.h"

namespace KIMAP
{
class NamespaceJobPrivate : public JobPrivate
{
public:
    NamespaceJobPrivate(Session *session, const QString &name)
        : JobPrivate(session, name)
    {
    }
    ~NamespaceJobPrivate()
    {
    }

    QList<MailBoxDescriptor> processNamespaceList(const QList<QByteArray> &namespaceList)
    {
        QList<MailBoxDescriptor> result;

        for (const QByteArray &namespaceItem : namespaceList) {
            ImapStreamParser parser(nullptr);
            parser.setData(namespaceItem);

            try {
                QList<QByteArray> parts = parser.readParenthesizedList();
                if (parts.size() < 2) {
                    continue;
                }
                MailBoxDescriptor descriptor;
                descriptor.name = QString::fromUtf8(decodeImapFolderName(parts[0]));
                descriptor.separator = QLatin1Char(parts[1][0]);

                result << descriptor;
            } catch (const KIMAP::ImapParserException &e) {
                qCWarning(KIMAP_LOG) << "The stream parser raised an exception during namespace list parsing:" << e.what();
                qCWarning(KIMAP_LOG) << "namespacelist:" << namespaceList;
            }
        }

        return result;
    }

    QList<MailBoxDescriptor> personalNamespaces;
    QList<MailBoxDescriptor> userNamespaces;
    QList<MailBoxDescriptor> sharedNamespaces;
};
}

using namespace KIMAP;

NamespaceJob::NamespaceJob(Session *session)
    : Job(*new NamespaceJobPrivate(session, i18n("Namespace")))
{
}

NamespaceJob::~NamespaceJob()
{
}

QList<MailBoxDescriptor> NamespaceJob::personalNamespaces() const
{
    Q_D(const NamespaceJob);
    return d->personalNamespaces;
}

QList<MailBoxDescriptor> NamespaceJob::userNamespaces() const
{
    Q_D(const NamespaceJob);
    return d->userNamespaces;
}

QList<MailBoxDescriptor> NamespaceJob::sharedNamespaces() const
{
    Q_D(const NamespaceJob);
    return d->sharedNamespaces;
}

bool NamespaceJob::containsEmptyNamespace() const
{
    Q_D(const NamespaceJob);
    const QList<MailBoxDescriptor> completeList = d->personalNamespaces + d->userNamespaces + d->sharedNamespaces;

    for (const MailBoxDescriptor &descriptor : completeList) {
        if (descriptor.name.isEmpty()) {
            return true;
        }
    }

    return false;
}

void NamespaceJob::doStart()
{
    Q_D(NamespaceJob);
    d->tags << d->sessionInternal()->sendCommand("NAMESPACE");
}

void NamespaceJob::handleResponse(const Response &response)
{
    Q_D(NamespaceJob);
    if (handleErrorReplies(response) == NotHandled) {
        if (response.content.size() >= 5 && response.content[1].toString() == "NAMESPACE") {
            // Personal namespaces
            d->personalNamespaces = d->processNamespaceList(response.content[2].toList());

            // User namespaces
            d->userNamespaces = d->processNamespaceList(response.content[3].toList());

            // Shared namespaces
            d->sharedNamespaces = d->processNamespaceList(response.content[4].toList());
        }
    }
}
