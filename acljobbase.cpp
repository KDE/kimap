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

#include <KDE/KLocale>
#include <KDE/KDebug>

#include "acljobbase_p.h"
#include "message_p.h"
#include "session_p.h"

using namespace KIMAP;

void AclJobBasePrivate::setIdentifier( const QByteArray &identifier )
{
  id = identifier;
}

QByteArray AclJobBasePrivate::identifier() const
{
  return id;
}



QByteArray AclJobBasePrivate::rights()
{
  QByteArray r;
  for (int i = 0; i < rightList.size(); i++) {
    r += rightsMap.key(rightList[i]).toAscii();
  }

  return r;
}

bool AclJobBasePrivate::hasRightEnabled(AclJobBase::AclRight right)
{
  return rightList.contains(right);
}

QList<AclJobBase::AclRight> AclJobBasePrivate::rightsFromString(const QByteArray& rights)
{
  QList<AclJobBase::AclRight> result;

  if (rights.isEmpty())
    return result;

  int pos = 0;
  if (rights[0] == '+') {
    modifier = AclJobBase::Add;
    pos++;
  } else if (rights[0]== '-') {
    modifier = AclJobBase::Remove;
    pos++;
  } else {
    modifier = AclJobBase::Change;
  }

  for (int i = pos; i < rights.size(); i++) {
    if (rightsMap.contains(rights[i]) && !result.contains(rightsMap[rights[i]])) {
      result.append(rightsMap[rights[i]]);
    }
  }

  return result;
}

void AclJobBasePrivate::setRights(const QByteArray& rights)
{
  rightList = rightsFromString(rights);
}

void AclJobBasePrivate::setRights(AclJobBase::AclModifier _modifier, const QList<AclJobBase::AclRight> &rights)
{

  modifier = _modifier;
  for (int i = 0; i < rights.size(); i++) {
    if (!rightList.contains(rights[i])) {
      rightList.append(rights[i]);
    }
  }
}



AclJobBase::AclJobBase( Session *session )
  : Job( *new AclJobBasePrivate(session, i18n("AclJobBase")) )
{
}


AclJobBase::AclJobBase( JobPrivate &dd )
  : Job(dd)
{

}

AclJobBase::~AclJobBase()
{
}


void AclJobBase::setMailBox( const QByteArray &mailBox )
{
  Q_D(AclJobBase);
  d->mailBox = mailBox;
}

QByteArray AclJobBase::mailBox() const
{
  Q_D(const AclJobBase);
  return d->mailBox;
}

QByteArray AclJobBase::rightsToString(const QList<AclJobBase::AclRight> &rights)
{
  Q_D(const AclJobBase);

  QByteArray result;
  Q_FOREACH(AclJobBase::AclRight right, rights) {
    result += d->rightsMap.key(right).toLatin1();
  }
  return result;
}


#include "acljobbase.moc"
