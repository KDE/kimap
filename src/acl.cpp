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

#include "acl.h"

#include <QtCore/QByteArray>
#include <QtCore/QMap>

namespace KIMAP
{
namespace Acl
{

class RightsMap
{
public:
    RightsMap()
    {
        map['l'] = Lookup;
        map['r'] = Read;
        map['s'] = KeepSeen;
        map['w'] = Write;
        map['i'] = Insert;
        map['p'] = Post;
        map['c'] = Create; //TODO: obsolete, keep it?
        map['d'] = Delete; //TODO: obsolete, keep it?
        map['k'] = CreateMailbox;
        map['x'] = DeleteMailbox;
        map['t'] = DeleteMessage;
        map['e'] = Expunge;
        map['a'] = Admin;
        map['n'] = WriteShared;
        map['0'] = Custom0;
        map['1'] = Custom1;
        map['2'] = Custom2;
        map['3'] = Custom3;
        map['4'] = Custom4;
        map['5'] = Custom5;
        map['6'] = Custom6;
        map['7'] = Custom7;
        map['8'] = Custom8;
        map['9'] = Custom9;
    }

    QMap<char, Right> map;
};

Q_GLOBAL_STATIC(RightsMap, globalRights)

}
}

KIMAP::Acl::Rights KIMAP::Acl::rightsFromString(const QByteArray &string)
{
    Rights result;

    if (string.isEmpty()) {
        return result;
    }

    int pos = 0;
    if (string[0] == '+' || string[0] == '-') {   // Skip modifier if any
        pos++;
    }

    for (int i = pos; i < string.size(); i++) {
        if (globalRights->map.contains(string[i])) {
            result |= globalRights->map[string[i]];
        }
    }

    return result;
}

QByteArray KIMAP::Acl::rightsToString(Rights rights)
{
    QByteArray result;

    for (int right = Lookup; right <= Custom9; right <<= 1) {
        if (rights & right) {
            result += globalRights->map.key((Right)right);
        }
    }

    return result;
}

KIMAP::Acl::Rights KIMAP::Acl::normalizedRights(KIMAP::Acl::Rights rights)
{
    Rights normalized = rights;
    if (normalized & Create) {
        normalized |= (CreateMailbox | DeleteMailbox);
        normalized &= ~Create;
    }
    if (normalized & Delete) {
        normalized |= (DeleteMessage | Expunge);
        normalized &= ~Delete;
    }
    return normalized;
}

KIMAP::Acl::Rights KIMAP::Acl::denormalizedRights(KIMAP::Acl::Rights rights)
{
    Rights denormalized = normalizedRights(rights);
    if (denormalized & (CreateMailbox | DeleteMailbox)) {
        denormalized |= Create;
    }
    if (denormalized & (DeleteMessage | Expunge)) {
        denormalized |= Delete;
    }
    return denormalized;
}
