/*
    Copyright (c) 2009 Kevin Ottens <ervin@kde.org>

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

#ifndef KIMAP_ACLJOBBASE_P_H
#define KIMAP_ACLJOBBASE_P_H

#include "acljobbase.h"
#include "job_p.h"
#include "session.h"
#include <KDE/KLocale>

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

      void setIdentifier( const QByteArray &identifier );
      QByteArray identifier() const;

      QByteArray rights();

      bool hasRightEnabled(AclJobBase::AclRight right);

      void setRights(const QByteArray& rights);
      void setRights(AclJobBase::AclModifier modifier, const QList<AclJobBase::AclRight> &rights);

      QList<AclJobBase::AclRight> rightsFromString(const QByteArray& rights);

      QByteArray mailBox;
      QByteArray id;
      QList<AclJobBase::AclRight> rightList;
      AclJobBase::AclModifier modifier;
      QMap<QChar, AclJobBase::AclRight> rightsMap;
  };
}

#endif
