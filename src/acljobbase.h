/*
    SPDX-FileCopyrightText: 2009 Andras Mantia <amantia@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "kimap_export.h"

#include "acl.h"
#include "job.h"

namespace KIMAP
{
class Session;
struct Response;
class AclJobBasePrivate;

/*!
 * \class KIMAP::AclJobBase
 * \inmodule KIMAP
 * \inheaderfile KIMAP/AclJobBase
 *
 * \brief Base class for jobs that operate on mailbox ACLs.
 *
 * Provides support for the IMAP ACL extension, as defined by
 * \l{https://tools.ietf.org/html/rfc4314}{RFC 4314}.
 *
 * This class cannot be used directly, you must subclass it and reimplement
 * at least the doStart() method.
 */
class KIMAP_EXPORT AclJobBase : public Job
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(AclJobBase)

    friend class SessionPrivate;

public:
    /*!
     *
     */
    explicit AclJobBase(Session *session);
    ~AclJobBase() override;

    /*!
     * Used when subclassing to specify how the ACL will be modified.
     *
     * \value Add
     * \value Remove
     * \value Change
     */
    enum AclModifier {
        Add = 0,
        Remove,
        Change
    };

    /*!
     * Set the mailbox to act on
     *
     * \a mailBox  the name of an existing mailbox
     */
    void setMailBox(const QString &mailBox);
    /*!
     * The mailbox that will be acted upon.
     */
    [[nodiscard]] QString mailBox() const;

protected:
    KIMAP_NO_EXPORT explicit AclJobBase(JobPrivate &dd);
};

}
