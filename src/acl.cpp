/*
    SPDX-FileCopyrightText: 2009 Andras Mantia <amantia@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "acl.h"

#include <QByteArray>
#include <QMap>

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
        map['c'] = Create; // TODO: obsolete, keep it?
        map['d'] = Delete; // TODO: obsolete, keep it?
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
    if (string[0] == '+' || string[0] == '-') { // Skip modifier if any
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
            result += globalRights->map.key(static_cast<Right>(right));
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
