/*
    SPDX-FileCopyrightText: 2009 Andras Mantia <amantia@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "kimap_export.h"

#include <qglobal.h>

namespace KIMAP
{
/*!
 * \namespace KIMAP::Acl
 * \inmodule KIMAP
 * \inheaderfile KIMAP/Acl
 *
 * \brief Operations for dealing with mailbox permissions.
 */
namespace Acl
{
/*!
 * Possible rights that can be held on a mailbox
 *
 * \value None
 * \value Lookup Mailbox is visible to LIST/LSUB commands, SUBSCRIBE mailbox
 * \value Read SELECT the mailbox, perform STATUS
 * \value KeepSeen Set or clear the Seen flag on messages in the mailbox, and keep it across sessions
 * \value Write Set or clear flags other than Seen and Deleted on messages in the mailbox
 * \value Insert Perform APPEND and COPY with the mailbox as the target
 * \value Post Send mail to the submission address for the mailbox. Note this is not enforced by IMAP4, but is purely advisory.
 * \value Create Obsolete as of RFC 4314, replaced by CreateMailbox and DeleteMailbox
 * \value CreateMailbox Create new child mailboxes, or move a mailbox with this mailbox as the new parent. Note that what constitutes a "child" mailbox is
 * implementation-defined, but . or / are usually used as separaters.
 * \value DeleteMailbox Delete or move the mailbox
 * \value DeleteMessage Set or clear the Deleted flag on messages in the mailbox
 * \value Delete Obsolete as of RFC 4314, replaced by DeleteMessage and Expunge
 * \value Admin View and modify the access control list for the mailbox
 * \value Expunge Expunge the messages in this mailbox. Note that if this right is not held on a mailbox, closing the mailbox (see CloseJob) will succeed, but
 * will not expunge the messages.
 * \value WriteShared Write shared annotations. See \l{https://tools.ietf.org/html/rfc5257}{RFC 5257}. Only supported by servers that implement the ANNOTATE
 * extension.
 * \value Custom0 Server-specific right 0
 * \value Custom1 Server-specific right 1
 * \value Custom2 Server-specific right 2
 * \value Custom3 Server-specific right 3
 * \value Custom4 Server-specific right 4
 * \value Custom5 Server-specific right 5
 * \value Custom6 Server-specific right 6
 * \value Custom7 Server-specific right 7
 * \value Custom8 Server-specific right 8
 * \value Custom9 Server-specific right 9
 */
enum Right {
    None = 0x000000,
    Lookup = 0x000001,
    Read = 0x000002,
    KeepSeen = 0x000004,
    Write = 0x000008,
    Insert = 0x000010,
    Post = 0x000020,
    Create = 0x000040,
    CreateMailbox = 0x000080,
    DeleteMailbox = 0x000100,
    DeleteMessage = 0x000200,
    Delete = 0x000400,
    Admin = 0x000800,
    Expunge = 0x001000,
    WriteShared = 0x002000,
    Custom0 = 0x004000,
    Custom1 = 0x008000,
    Custom2 = 0x010000,
    Custom3 = 0x020000,
    Custom4 = 0x040000,
    Custom5 = 0x080000,
    Custom6 = 0x100000,
    Custom7 = 0x200000,
    Custom8 = 0x400000,
    Custom9 = 0x800000
};

Q_DECLARE_FLAGS(Rights, Right)

/*!
 * \relates KIMAP::Acl
 *
 * Returns a rights mask that has no obsolete members anymore, i.e. obsolete flags are removed and
 * replaced by their successors.
 *
 * \a rights set of Rights flags to normalize
 *
 * \since 4.6
 */
[[nodiscard]] KIMAP_EXPORT Rights normalizedRights(Rights rights);

/*!
 * \relates KIMAP::Acl
 *
 * Returns a rights mask that contains both obsolete and new flags if one of them is set.
 *
 * \a rights set of Rights flags to augment
 *
 * \since 4.6
 */
[[nodiscard]] KIMAP_EXPORT Rights denormalizedRights(Rights rights);

/*!
 * \relates KIMAP::Acl
 *
 * Convert a set of rights into text format
 *
 * No modifier flag ('+' or '-') will be included.
 */
[[nodiscard]] KIMAP_EXPORT QByteArray rightsToString(Rights rights);

/*!
 * \relates KIMAP::Acl
 *
 * Convert the text form of a set of rights into a Rights bitflag
 *
 * Modifier flags ('+' and '-') are ignored, as are any unknown
 * characters. This method will not complain if you give it
 * something that is not a list of rights.
 */
[[nodiscard]] KIMAP_EXPORT Rights rightsFromString(QByteArrayView string);
}
}

Q_DECLARE_OPERATORS_FOR_FLAGS(KIMAP::Acl::Rights)
