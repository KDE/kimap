/*
    Copyright (c) 2009 Andras Mantia <amantia@kde.org>

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

#include "deleteacljob.h"

#include <KLocalizedString>

#include "acljobbase_p.h"
#include "message_p.h"
#include "session_p.h"
#include "rfccodecs.h"

namespace KIMAP
{
class DeleteAclJobPrivate : public AclJobBasePrivate
{
public:
    DeleteAclJobPrivate(Session *session, const QString &name) : AclJobBasePrivate(session, name) {}
    ~DeleteAclJobPrivate() { }
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
