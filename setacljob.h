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

#ifndef KIMAP_SETACLJOB_H
#define KIMAP_SETACLJOB_H

#include "kimap_export.h"

#include "acljobbase.h"

namespace KIMAP {

class Session;
struct Message;
class SetAclJobPrivate;


class KIMAP_EXPORT SetAclJob : public AclJobBase
{
  Q_OBJECT
  Q_DECLARE_PRIVATE(SetAclJob)

  friend class SessionPrivate;

  public:
    explicit SetAclJob( Session *session );
    virtual ~SetAclJob();

    void setRights(AclModifier modifier, Acl::Rights rights);
    void setIdentifier( const QByteArray &identifier );
    QByteArray identifier();

  protected:
    virtual void doStart();

};

}

#endif
