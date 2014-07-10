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

#include <KLocalizedString>
#include <QDebug>

#include "acljobbase_p.h"
#include "message_p.h"
#include "session_p.h"
#include "rfccodecs.h"

namespace KIMAP
{
  class GetAclJobPrivate : public AclJobBasePrivate
  {
    public:
      GetAclJobPrivate( Session *session, const QString& name ) : AclJobBasePrivate( session, name ) {}
      ~GetAclJobPrivate() { }

      QMap<QByteArray, Acl::Rights> userRights;
  };
}

using namespace KIMAP;

GetAclJob::GetAclJob( Session *session )
  : AclJobBase( *new GetAclJobPrivate( session, i18n( "GetAcl" ) ) )
{
}

GetAclJob::~GetAclJob()
{
}

void GetAclJob::doStart()
{
  Q_D( GetAclJob );

  d->tags << d->sessionInternal()->sendCommand( "GETACL", '\"' + KIMAP::encodeImapFolderName( d->mailBox.toUtf8() ) + '\"' );
}

void GetAclJob::handleResponse( const Message &response )
{
  Q_D( GetAclJob );
//   qDebug() << response.toString();

  if ( handleErrorReplies( response ) == NotHandled ) {
    if ( response.content.size() >= 4 &&
         response.content[1].toString() == "ACL" ) {
      int i = 3;
      while ( i < response.content.size() - 1 ) {
        QByteArray id = response.content[i].toString();
        QByteArray rights = response.content[i + 1].toString();
        d->userRights[id] = Acl::rightsFromString( rights );
        i += 2;
      }
    }
  }
}

QList<QByteArray> GetAclJob::identifiers() const
{
  Q_D( const GetAclJob );
  return d->userRights.keys();
}

bool GetAclJob::hasRightEnabled(const QByteArray &identifier, Acl::Right right) const
{
  Q_D( const GetAclJob );
  if ( d->userRights.contains( identifier ) ) {
    Acl::Rights rights = d->userRights[identifier];
    return rights & right;
  }

  return false;
}

Acl::Rights GetAclJob::rights(const QByteArray &identifier) const
{
  Q_D( const GetAclJob );
  Acl::Rights result;
  if ( d->userRights.contains( identifier ) ) {
    result = d->userRights[identifier];
  }
  return result;
}

QMap<QByteArray, Acl::Rights> GetAclJob::allRights() const
{
  Q_D( const GetAclJob );
  return d->userRights;
}
