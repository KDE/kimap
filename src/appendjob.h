/*
    Copyright (c) 2009 Kevin Ottens <ervin@kde.org>

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

#ifndef KIMAP_APPENDJOB_H
#define KIMAP_APPENDJOB_H

#include <kdatetime.h>
#include "kimap_export.h"

#include "job.h"

namespace KIMAP
{

class Session;
struct Message;
class AppendJobPrivate;

/**
 * Appends a message to a mailbox.
 *
 * This job can only be run when the session is in the
 * authenticated (or selected) state.
 *
 * If the server supports ACLs, the user will need the
 * Acl::Insert right on the mailbox.
 */
class KIMAP_EXPORT AppendJob : public Job
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(AppendJob)

    friend class SessionPrivate;

public:
    AppendJob(Session *session);
    virtual ~AppendJob();

    /**
     * Set the mailbox to append the message to.
     *
     * If the mailbox does not exist, it will not automatically
     * be created and the command will fail.
     *
     * @param mailBox  the (unquoted) name of the mailbox
     */
    void setMailBox(const QString &mailBox);
    /**
     * The mailbox that the message will be appended to.
     */
    QString mailBox() const;

    /**
     * Set the flags that should be applied to the appended message.
     *
     * @param flags  a list of flags
     */
    void setFlags(const QList<QByteArray> &flags);
    /**
     * The flags that will be set on the appended message.
     */
    QList<QByteArray> flags() const;

    /**
     * Set the internal date that should be applied to the appended message.
     *
     * This is the date/time the IMAP server should set internally for the appended message.
     * See http://tools.ietf.org/html/rfc3501#section-6.3.11
     *
     * If this is not set, the server will use the current date/time.
     *
     * @param internalDate  the internal date
     *
     * @since 4.13
     */
    void setInternalDate(const KDateTime &internalDate);
    /**
     * The internal date that will be set on the appended message.
     *
     * @since 4.13
     */
    KDateTime internalDate() const;

    /**
     * The content of the message.
     *
     * This should be in RFC-2822 format, although some required header
     * lines may be omitted in certain cases, for example when appending
     * to a Drafts folder.
     *
     * @param content  usually an RFC-2822 message
     */
    void setContent(const QByteArray &content);
    /**
     * The content that the message will have.
     */
    QByteArray content() const;

    /**
     * The UID of the new message.
     *
     * This will be zero if it is unknown.
     *
     * The UID will not be known until the job has been successfully
     * executed, and it will only be known at all if the server
     * supports the UIDPLUS extension (RFC 4315).
     */
    qint64 uid() const;

protected:
    virtual void doStart() Q_DECL_OVERRIDE;
    virtual void handleResponse(const Message &response) Q_DECL_OVERRIDE;
};

}

#endif
