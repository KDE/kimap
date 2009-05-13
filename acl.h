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

#ifndef KIMAP_ACL_H
#define KIMAP_ACL_H

#include "kimap_export.h"

namespace KIMAP {

namespace Acl {

enum Right {
  None          = 0x000000,
  Lookup        = 0x000001,
  Read          = 0x000002,
  KeepSeen      = 0x000004,
  Write         = 0x000008,
  Insert        = 0x000010,
  Post          = 0x000020,
  Create        = 0x000040,
  CreateMailbox = 0x000080,
  DeleteMailbox = 0x000100,
  DeleteMessage = 0x000200,
  Delete        = 0x000400,
  Admin         = 0x000800,
  Expunge       = 0x001000,
  WriteShared   = 0x002000,
  Custom0       = 0x004000,
  Custom1       = 0x008000,
  Custom2       = 0x010000,
  Custom3       = 0x020000,
  Custom4       = 0x040000,
  Custom5       = 0x080000,
  Custom6       = 0x100000,
  Custom7       = 0x200000,
  Custom8       = 0x400000,
  Custom9       = 0x800000
};

Q_DECLARE_FLAGS(Rights, Right)

KIMAP_EXPORT QByteArray rightsToString( Rights rights );
KIMAP_EXPORT Rights rightsFromString( const QByteArray &string );

}
}

Q_DECLARE_OPERATORS_FOR_FLAGS( KIMAP::Acl::Rights )

#endif
