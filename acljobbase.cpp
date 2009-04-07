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

#include "job_p.h"
#include "message_p.h"
#include "session_p.h"

namespace KIMAP
{
  class AclJobBasePrivate : public JobPrivate
  {
    public:
      AclJobBasePrivate( Session *session, const QString& name ) : JobPrivate(session, name), modifier(AclJobBase::Change)
      {
         rightsMap['l'] = AclJobBase::Lookup;
         rightsMap['r'] = AclJobBase::Read;
         rightsMap['s'] = AclJobBase::KeepSeen;
         rightsMap['i'] = AclJobBase::Insert;
         rightsMap['p'] = AclJobBase::Post;
         rightsMap['c'] = AclJobBase::Create;
         rightsMap['d'] = AclJobBase::Delete;
         rightsMap['a'] = AclJobBase::Admin;
      }
      ~AclJobBasePrivate() { }

      QByteArray mailBox;
      QList<AclJobBase::AclRight> rights;
      AclJobBase::AclModifier modifier;
      QMap<QChar, AclJobBase::AclRight> rightsMap;
  };
}

using namespace KIMAP;

AclJobBase::AclJobBase( Session *session )
  : Job( *new AclJobBasePrivate(session, i18n("AclJobBase")) )
{
}

AclJobBase::~AclJobBase()
{
}

void AclJobBase::doStart()
{
  //does nothing
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

void AclJobBase::setRights(const QByteArray& rights)
{
  Q_D(AclJobBase);
  
  if (rights.isEmpty())
    return;

  int pos = 0;
  if (rights[0] == '+') {
    d->modifier = Add;
    pos++;
  } else if (rights[0]== '-') {
    d->modifier = Remove;
    pos++;
  } else {
    d->modifier = Change;
  }

  for (int i = pos; i < rights.size(); i++) {
    if (d->rightsMap.contains(rights[i]) && !d->rights.contains(d->rightsMap[rights[i]])) {
      d->rights.append(d->rightsMap[rights[i]]);
    }
  }
}

void AclJobBase::setRights(AclModifier modifier, const QList<AclRight> &rights)
{
  Q_D(AclJobBase);
  
  d->modifier = modifier;
  for (int i = 0; i < rights.size(); i++) {
    if (!d->rights.contains(rights[i])) {
      d->rights.append(d->rightsMap[rights[i]]);
    }
  }
}

QByteArray AclJobBase::rights()
{
  Q_D(AclJobBase);
  
  QByteArray rights; 
  for (int i = 0; i < d->rights.size(); i++) {
    rights += d->rightsMap.key(d->rights[i]).toAscii();
  }

  return rights;
}

bool AclJobBase::hasRightEnabled(AclRight right)
{
  Q_D(AclJobBase);
  
  return d->rights.contains(right);
}


#include "acljobbase.moc"
