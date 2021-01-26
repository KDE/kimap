/*
    SPDX-FileCopyrightText: 2009 Andras Mantia <amantia@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "getacljob.h"

#include "kimap_debug.h"
#include <KLocalizedString>

#include "acljobbase_p.h"
#include "response_p.h"
#include "rfccodecs.h"
#include "session_p.h"

namespace KIMAP
{
class GetAclJobPrivate : public AclJobBasePrivate
{
public:
    GetAclJobPrivate(Session *session, const QString &name)
        : AclJobBasePrivate(session, name)
    {
    }
    ~GetAclJobPrivate()
    {
    }

    QMap<QByteArray, Acl::Rights> userRights;
};
}

using namespace KIMAP;

GetAclJob::GetAclJob(Session *session)
    : AclJobBase(*new GetAclJobPrivate(session, i18n("GetAcl")))
{
}

GetAclJob::~GetAclJob()
{
}

void GetAclJob::doStart()
{
    Q_D(GetAclJob);

    d->tags << d->sessionInternal()->sendCommand("GETACL", '\"' + KIMAP::encodeImapFolderName(d->mailBox.toUtf8()) + '\"');
}

void GetAclJob::handleResponse(const Response &response)
{
    Q_D(GetAclJob);
    //   qCDebug(KIMAP_LOG) << response.toString();

    if (handleErrorReplies(response) == NotHandled) {
        if (response.content.size() >= 4 && response.content[1].toString() == "ACL") {
            int i = 3;
            while (i < response.content.size() - 1) {
                QByteArray id = response.content[i].toString();
                QByteArray rights = response.content[i + 1].toString();
                d->userRights[id] = Acl::rightsFromString(rights);
                i += 2;
            }
        }
    }
}

QList<QByteArray> GetAclJob::identifiers() const
{
    Q_D(const GetAclJob);
    return d->userRights.keys();
}

bool GetAclJob::hasRightEnabled(const QByteArray &identifier, Acl::Right right) const
{
    Q_D(const GetAclJob);
    if (d->userRights.contains(identifier)) {
        Acl::Rights rights = d->userRights[identifier];
        return rights & right;
    }

    return false;
}

Acl::Rights GetAclJob::rights(const QByteArray &identifier) const
{
    Q_D(const GetAclJob);
    Acl::Rights result;
    if (d->userRights.contains(identifier)) {
        result = d->userRights[identifier];
    }
    return result;
}

QMap<QByteArray, Acl::Rights> GetAclJob::allRights() const
{
    Q_D(const GetAclJob);
    return d->userRights;
}
