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

#include "acljobbase.h"
#include "acljobbase_p.h"
#include "message_p.h"
#include "session_p.h"

#include <KDE/KLocalizedString>

using namespace KIMAP;

void AclJobBasePrivate::setIdentifier( const QByteArray &identifier )
{
  id = identifier;
}

QByteArray AclJobBasePrivate::identifier() const
{
  return id;
}

bool AclJobBasePrivate::hasRightEnabled(Acl::Right right)
{
  return rightList & right;
}

void AclJobBasePrivate::setRights(const QByteArray& rights)
{
  switch ( rights[0] ) {
  case '+':
    modifier = AclJobBase::Add;
    break;
  case '-':
    modifier = AclJobBase::Remove;
    break;
  default:
    modifier = AclJobBase::Change;
    break;
  }

  rightList = Acl::rightsFromString( rights );
}

void AclJobBasePrivate::setRights(AclJobBase::AclModifier _modifier, Acl::Rights rights)
{
  modifier = _modifier;
  // XXX: [alexmerry, 2010-07-24]: this is REALLY unintuitive behaviour
  rightList|= rights;
}

AclJobBase::AclJobBase( Session *session )
  : Job( *new AclJobBasePrivate( session, i18n( "AclJobBase" ) ) )
{
}

AclJobBase::AclJobBase( JobPrivate &dd )
  : Job( dd )
{

}

AclJobBase::~AclJobBase()
{
}

void AclJobBase::setMailBox( const QString &mailBox )
{
  Q_D( AclJobBase );
  d->mailBox = mailBox;
}

QString AclJobBase::mailBox() const
{
  Q_D( const AclJobBase );
  return d->mailBox;
}
