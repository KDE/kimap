/*
    SPDX-FileCopyrightText: 2009 Andras Mantia <amantia@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "setacljob.h"

#include <KLocalizedString>

#include "acljobbase_p.h"
#include "response_p.h"
#include "rfccodecs.h"
#include "session_p.h"

namespace KIMAP
{
class SetAclJobPrivate : public AclJobBasePrivate
{
public:
    SetAclJobPrivate(Session *session, const QString &name)
        : AclJobBasePrivate(session, name)
    {
    }
    ~SetAclJobPrivate()
    {
    }
};
}

using namespace KIMAP;

SetAclJob::SetAclJob(Session *session)
    : AclJobBase(*new SetAclJobPrivate(session, i18n("SetAcl")))
{
}

SetAclJob::~SetAclJob()
{
}

void SetAclJob::doStart()
{
    Q_D(SetAclJob);
    QByteArray r = Acl::rightsToString(d->rightList);
    if (d->modifier == Add) {
        r.prepend('+');
    } else if (d->modifier == Remove) {
        r.prepend('-');
    }
    d->tags << d->sessionInternal()->sendCommand("SETACL", '\"' + KIMAP::encodeImapFolderName(d->mailBox.toUtf8()) + "\" \"" + d->id + "\" \"" + r + '\"');
}

void SetAclJob::setRights(AclModifier modifier, Acl::Rights rights)
{
    Q_D(SetAclJob);
    d->setRights(modifier, rights);
}

void SetAclJob::setIdentifier(const QByteArray &identifier)
{
    Q_D(SetAclJob);
    d->setIdentifier(identifier);
}

QByteArray SetAclJob::identifier()
{
    Q_D(SetAclJob);
    return d->identifier();
}
