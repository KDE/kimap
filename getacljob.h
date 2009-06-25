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

#ifndef KIMAP_GETACLJOB_H
#define KIMAP_GETACLJOB_H

#include "kimap_export.h"

#include "acljobbase.h"

namespace KIMAP {

class Session;
struct Message;
class GetAclJobPrivate;


class KIMAP_EXPORT GetAclJob : public AclJobBase
{
  Q_OBJECT
  Q_DECLARE_PRIVATE(GetAclJob)

  friend class SessionPrivate;

  public:
    explicit GetAclJob( Session *session );
    virtual ~GetAclJob();

    QList<QByteArray> identifiers() const;
    bool hasRightEnabled(const QByteArray &identifier, Acl::Right right) const;
    Acl::Rights rights(const QByteArray &identifier) const;

    QMap<QByteArray, Acl::Rights> allRights() const;

  protected:
    virtual void doStart();
    virtual void handleResponse(const Message &response);

};

}

#endif
