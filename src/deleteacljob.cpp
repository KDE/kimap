/*
    SPDX-FileCopyrightText: 2009 Andras Mantia <amantia@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "deleteacljob.h"

#include <KLocalizedString>

#include "acljobbase_p.h"
#include "response_p.h"
#include "rfccodecs.h"
#include "session_p.h"

namespace KIMAP
{
class DeleteAclJobPrivate : public AclJobBasePrivate
{
public:
    DeleteAclJobPrivate(Session *session, const QString &name)
        : AclJobBasePrivate(session, name)
    {
    }
    ~DeleteAclJobPrivate()
    {
    }
};
}

using namespace KIMAP;

DeleteAclJob::DeleteAclJob(Session *session)
    : AclJobBase(session)
{
    Q_D(DeleteAclJob);
    d->m_name = i18n("DeleteAclJob");
}

DeleteAclJob::~DeleteAclJob()
{
}

void DeleteAclJob::doStart()
{
    Q_D(DeleteAclJob);

    d->tags << d->sessionInternal()->sendCommand("DELETEACL", '\"' + KIMAP::encodeImapFolderName(d->mailBox.toUtf8()) + "\" \"" + d->id);
}

void DeleteAclJob::setIdentifier(const QByteArray &identifier)
{
    Q_D(DeleteAclJob);
    d->setIdentifier(identifier);
}

QByteArray DeleteAclJob::identifier()
{
    Q_D(DeleteAclJob);
    return d->identifier();
}
