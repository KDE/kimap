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

#ifndef KIMAP_ACLJOBBASE_H
#define KIMAP_ACLJOBBASE_H

#include "kimap_export.h"

#include "job.h"

namespace KIMAP {

class Session;
class Message;
class AclJobBasePrivate;

/** @short Base class of Acl related jobs. It cannot be used directly, you must subclass it and reimplement at least the
doStart() method.
*/
class KIMAP_EXPORT AclJobBase : public Job
{
  Q_OBJECT
  Q_DECLARE_PRIVATE(AclJobBase)

  friend class SessionPrivate;

  public:
    AclJobBase( Session *session );
    virtual ~AclJobBase();

    enum AclRight {
      Lookup = 0,
      Read,
      KeepSeen,
      Write,
      Insert,
      Post,
      Create,
      CreateMailbox,
      DeleteMailbox,
      DeleteMessage,
      Delete,
      Admin,
      Expunge,
      WriteShared,
      Custom0,
      Custom1,
      Custom2,
      Custom3,
      Custom4,
      Custom5,
      Custom6,
      Custom7,
      Custom8,
      Custom9
    };

    Q_DECLARE_FLAGS(AclRights, AclRight)

    enum AclModifier {
      Add = 0,
      Remove,
      Change
    };

    Q_DECLARE_FLAGS(AclModifiers, AclModifier)


    void setMailBox( const QByteArray &mailBox );
    QByteArray mailBox() const;

    QByteArray rightsToString(const QList<AclJobBase::AclRight> &rights);

  protected:
    AclJobBase( JobPrivate &dd );

};

}

#endif
