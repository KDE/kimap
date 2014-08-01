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

#include <qglobal.h>

namespace KIMAP
{

/**
 * Operations for dealing with mailbox permissions.
 */
namespace Acl
{

/**
 * Possible rights that can be held on a mailbox
 */
enum Right {
    None          = 0x000000,
    /** Mailbox is visible to LIST/LSUB commands, SUBSCRIBE mailbox */
    Lookup        = 0x000001,
    /** SELECT the mailbox, perform STATUS */
    Read          = 0x000002,
    /** Set or clear the \Seen flag on messages in the mailbox, and keep it across sessions */
    KeepSeen      = 0x000004,
    /** Set or clear flags other than \Seen and \Deleted on messages in the mailbox */
    Write         = 0x000008,
    /** Perform APPEND and COPY with the mailbox as the target */
    Insert        = 0x000010,
    /** Send mail to the submission address for the mailbox
     *
     * Note: this is not enforced by IMAP4, but is purely advisory.
     */
    Post          = 0x000020,
    /** Obsolete as of RFC 4314, replaced by CreateMailbox and DeleteMailbox */
    Create        = 0x000040,
    /** Create new child mailboxes, or move a mailbox with this mailbox as the new parent
     *
     * Note that what constitutes a "child" mailbox is implementation-defined, but
     * . or / are usually used as separaters.
     */
    CreateMailbox = 0x000080,
    /** Delete or move the mailbox */
    DeleteMailbox = 0x000100,
    /** Set or clear the \Deleted flag on messages in the mailbox */
    DeleteMessage = 0x000200,
    /** Obsolete as of RFC 4314, replaced by DeleteMessage and Expunge*/
    Delete        = 0x000400,
    /** View and modify the access control list for the mailbox */
    Admin         = 0x000800,
    /** Expunge the messages in this mailbox
     *
     * Note that if this right is not held on a mailbox, closing the mailbox
     * (see CloseJob) will succeed, but will not expunge the messages.
     */
    Expunge       = 0x001000,
    /** Write shared annotations
     *
     * See <a href="http://www.apps.ietf.org/rfc/rfc5257.html" title="IMAP ANNOTATE extension">RFC
     * 5257</a>.  Only supported by servers that implement the ANNOTATE extension.
     */
    WriteShared   = 0x002000,
    Custom0       = 0x004000, /**< Server-specific right 0 */
    Custom1       = 0x008000, /**< Server-specific right 1 */
    Custom2       = 0x010000, /**< Server-specific right 2 */
    Custom3       = 0x020000, /**< Server-specific right 3 */
    Custom4       = 0x040000, /**< Server-specific right 4 */
    Custom5       = 0x080000, /**< Server-specific right 5 */
    Custom6       = 0x100000, /**< Server-specific right 6 */
    Custom7       = 0x200000, /**< Server-specific right 7 */
    Custom8       = 0x400000, /**< Server-specific right 8 */
    Custom9       = 0x800000  /**< Server-specific right 9 */
};

Q_DECLARE_FLAGS(Rights, Right)

/**
 * Returns a rights mask that has no obsolete members anymore, i.e. obsolete flags are removed and
 * replaced by their successors.
 * @param rights set of #Rights flags to normalize
 * @since 4.6
 */
KIMAP_EXPORT Rights normalizedRights(Rights rights);

/**
 * Returns a rights mask that contains both obsolete and new flags if one of them is set.
 * @param rights set of #Rights flags to augment
 * @since 4.6
 */
KIMAP_EXPORT Rights denormalizedRights(Rights rights);

/**
 * Convert a set of rights into text format
 *
 * No modifier flag ('+' or '-') will be included.
 */
KIMAP_EXPORT QByteArray rightsToString(Rights rights);
/**
 * Convert the text form of a set of rights into a Rights bitflag
 *
 * Modifier flags ('+' and '-') are ignored, as are any unknown
 * characters.  This method will not complain if you give it
 * something that is not a list of rights.
 */
KIMAP_EXPORT Rights rightsFromString(const QByteArray &string);

}
}

Q_DECLARE_OPERATORS_FOR_FLAGS(KIMAP::Acl::Rights)

#endif
