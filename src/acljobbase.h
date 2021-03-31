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

/**
 * Base class for jobs that operate on mailbox ACLs
 *
 * Provides support for the IMAP ACL extension, as defined by
 * <a href="https://tools.ietf.org/html/rfc4314" title="IMAP ACL extension">RFC 4314</a>.
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
    AclJobBase(Session *session);
    ~AclJobBase() override;

    /**
     * Used when subclassing to specify how the ACL will be modified.
     */
    enum AclModifier { Add = 0, Remove, Change };

    /**
     * Set the mailbox to act on
     *
     * @param mailBox  the name of an existing mailbox
     */
    void setMailBox(const QString &mailBox);
    /**
     * The mailbox that will be acted upon.
     */
    QString mailBox() const;

protected:
    explicit AclJobBase(JobPrivate &dd);
};

}

