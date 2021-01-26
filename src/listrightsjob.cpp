/*
    SPDX-FileCopyrightText: 2009 Andras Mantia <amantia@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "listrightsjob.h"

#include <KLocalizedString>

#include "acljobbase_p.h"
#include "response_p.h"
#include "rfccodecs.h"
#include "session_p.h"

namespace KIMAP
{
class ListRightsJobPrivate : public AclJobBasePrivate
{
public:
    ListRightsJobPrivate(Session *session, const QString &name)
        : AclJobBasePrivate(session, name)
        , defaultRights(Acl::None)
    {
    }
    ~ListRightsJobPrivate()
    {
    }

    QList<Acl::Rights> possibleRights;
    Acl::Rights defaultRights;
};
}

using namespace KIMAP;

ListRightsJob::ListRightsJob(Session *session)
    : AclJobBase(*new ListRightsJobPrivate(session, i18n("ListRights")))
{
}

ListRightsJob::~ListRightsJob()
{
}

void ListRightsJob::doStart()
{
    Q_D(ListRightsJob);

    d->tags << d->sessionInternal()->sendCommand("LISTRIGHTS", '\"' + KIMAP::encodeImapFolderName(d->mailBox.toUtf8()) + "\" \"" + d->id + "\"");
}

void ListRightsJob::handleResponse(const Response &response)
{
    Q_D(ListRightsJob);

    if (handleErrorReplies(response) == NotHandled) {
        if (response.content.size() >= 4 && response.content[1].toString() == "LISTRIGHTS") {
            QByteArray s = response.content[4].toString();
            d->defaultRights = Acl::rightsFromString(s);
            int i = 5;
            while (i < response.content.size()) {
                s = response.content[i].toString();
                d->possibleRights.append(Acl::rightsFromString(s));
                i++;
            }
        }
    }
}

void ListRightsJob::setIdentifier(const QByteArray &identifier)
{
    Q_D(ListRightsJob);
    d->setIdentifier(identifier);
}

QByteArray ListRightsJob::identifier()
{
    Q_D(ListRightsJob);
    return d->identifier();
}

Acl::Rights ListRightsJob::defaultRights()
{
    Q_D(ListRightsJob);
    return d->defaultRights;
}

QList<Acl::Rights> ListRightsJob::possibleRights()
{
    Q_D(ListRightsJob);
    return d->possibleRights;
}
