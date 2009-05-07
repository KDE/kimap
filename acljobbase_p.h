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
        rightsMap['w'] = AclJobBase::Write;
        rightsMap['i'] = AclJobBase::Insert;
        rightsMap['p'] = AclJobBase::Post;
        rightsMap['c'] = AclJobBase::Create; //TODO: obsolete, keep it?
        rightsMap['d'] = AclJobBase::Delete; //TODO: obsolete, keep it?
        rightsMap['k'] = AclJobBase::CreateMailbox;
        rightsMap['x'] = AclJobBase::DeleteMailbox;
        rightsMap['t'] = AclJobBase::DeleteMessage;
        rightsMap['e'] = AclJobBase::Expunge;
        rightsMap['a'] = AclJobBase::Admin;
        rightsMap['n'] = AclJobBase::WriteShared;
        rightsMap['0'] = AclJobBase::Custom0;
        rightsMap['1'] = AclJobBase::Custom1;
        rightsMap['2'] = AclJobBase::Custom2;
        rightsMap['3'] = AclJobBase::Custom3;
        rightsMap['4'] = AclJobBase::Custom4;
        rightsMap['5'] = AclJobBase::Custom5;
        rightsMap['6'] = AclJobBase::Custom6;
        rightsMap['7'] = AclJobBase::Custom7;
        rightsMap['8'] = AclJobBase::Custom8;
        rightsMap['9'] = AclJobBase::Custom9;
      }
      ~AclJobBasePrivate() { }

      void setIdentifier( const QByteArray &identifier );
      QByteArray identifier() const;

      QByteArray rights();

      bool hasRightEnabled(AclJobBase::AclRight right);

      void setRights(const QByteArray& rights);
      void setRights(AclJobBase::AclModifier modifier, const QList<AclJobBase::AclRight> &rights);

      QList<AclJobBase::AclRight> rightsFromString(const QByteArray& rights);

      QString mailBox;
      QByteArray id;
      QList<AclJobBase::AclRight> rightList;
      AclJobBase::AclModifier modifier;
      QMap<QChar, AclJobBase::AclRight> rightsMap;
  };
}

#endif
