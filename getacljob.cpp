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

#include "getacljob.h"

#include <KDE/KLocale>
#include <KDE/KDebug>

#include "acljobbase_p.h"
#include "message_p.h"
#include "session_p.h"
#include "rfccodecs.h"

namespace KIMAP
{
  class GetAclJobPrivate : public AclJobBasePrivate
  {
    public:
      GetAclJobPrivate( Session *session, const QString& name ) : AclJobBasePrivate(session, name) {}
      ~GetAclJobPrivate() { }

      QMap<QByteArray, QList<AclJobBase::AclRight> > userRights;
  };
}

using namespace KIMAP;

GetAclJob::GetAclJob( Session *session )
  : AclJobBase(  *new GetAclJobPrivate(session, i18n("MyRights")  ))
{
}

GetAclJob::~GetAclJob()
{
}

void GetAclJob::doStart()
{
  Q_D(GetAclJob);

  d->tag = d->sessionInternal()->sendCommand( "GETACL", '\"' + KIMAP::encodeImapFolderName( d->mailBox ) + '\"');
}

void GetAclJob::doHandleResponse( const Message &response )
{
  Q_D(GetAclJob);
//   kDebug() << response.toString();

  if (handleErrorReplies(response) == NotHandled) {
    if ( response.content.size() >= 4
         && response.content[1].toString() == "ACL" ) {
      int i = 3;
      while ( i < response.content.size() - 1 ) {
        QByteArray id = response.content[i].toString();
        QByteArray rights = response.content[i + 1].toString();
        d->userRights[id] = d->rightsFromString(rights);
        i += 2;
      }
    }
  }
}

bool GetAclJob::hasRightEnabled(const QByteArray &identifier, AclJobBase::AclRight right)
{
  Q_D(GetAclJob);
  if (d->userRights.contains(identifier))
  {
    QList<AclJobBase::AclRight> rights = d->userRights[identifier];
    return rights.contains(right);
  }

  return false;
}

QList<AclJobBase::AclRight> GetAclJob::rights(const QByteArray &identifier)
{
  Q_D(GetAclJob);
  QList<AclJobBase::AclRight> result;
  if (d->userRights.contains(identifier))
  {
    result = d->userRights[identifier];
  }
  return result;
}

#include "getacljob.moc"
