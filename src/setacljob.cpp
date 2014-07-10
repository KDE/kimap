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

#include "setacljob.h"

#include <KLocalizedString>

#include "acljobbase_p.h"
#include "message_p.h"
#include "session_p.h"
#include "rfccodecs.h"

namespace KIMAP
{
  class SetAclJobPrivate : public AclJobBasePrivate
  {
    public:
      SetAclJobPrivate( Session *session, const QString& name ) : AclJobBasePrivate( session, name ) {}
      ~SetAclJobPrivate() { }
  };
}

using namespace KIMAP;

SetAclJob::SetAclJob( Session *session )
  : AclJobBase( *new SetAclJobPrivate( session, i18n( "SetAcl" ) ) )
{
}

SetAclJob::~SetAclJob()
{
}

void SetAclJob::doStart()
{
  Q_D( SetAclJob );
  QByteArray r = Acl::rightsToString( d->rightList );
  if ( d->modifier == Add ) {
    r.prepend( '+' );
  } else if ( d->modifier == Remove ) {
    r.prepend( '-' );
  }
  d->tags << d->sessionInternal()->sendCommand( "SETACL", '\"' + KIMAP::encodeImapFolderName( d->mailBox.toUtf8() ) + "\" \"" + d->id + "\" \"" + r + '\"' );
}

void SetAclJob::setRights(AclModifier modifier, Acl::Rights rights)
{
  Q_D( SetAclJob );
  d->setRights( modifier, rights );
}

void SetAclJob::setIdentifier( const QByteArray &identifier )
{
  Q_D( SetAclJob );
  d->setIdentifier( identifier );
}

QByteArray SetAclJob::identifier()
{
  Q_D( SetAclJob );
  return d->identifier();
}
