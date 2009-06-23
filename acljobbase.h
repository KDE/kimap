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

#include "acl.h"
#include "job.h"

namespace KIMAP {

class Session;
struct Message;
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

    enum AclModifier {
      Add = 0,
      Remove,
      Change
    };

    void setMailBox( const QString &mailBox );
    QString mailBox() const;

  protected:
    explicit AclJobBase( JobPrivate &dd );

};

}

#endif
