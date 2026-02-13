/*
    SPDX-FileCopyrightText: 2009 Kevin Ottens <ervin@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "kimap_export.h"

#include "job.h"
#include <QDateTime>

namespace KIMAP
{
class Session;
struct Response;
class AppendJobPrivate;

/*!
 * \class KIMAP::AppendJob
 * \inmodule KIMAP
 * \inheaderfile KIMAP/AppendJob
 *
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
    /*!
     */
    explicit AppendJob(Session *session);
    /*!
     */
    ~AppendJob() override;

    /*!
     * Set the mailbox to append the message to.
     *
     * If the mailbox does not exist, it will not automatically
     * be created and the command will fail.
     *
     * \a mailBox  the (unquoted) name of the mailbox
     */
    void setMailBox(const QString &mailBox);
    /*!
     * The mailbox that the message will be appended to.
     */
    [[nodiscard]] QString mailBox() const;

    /*!
     * Set the flags that should be applied to the appended message.
     *
     * \a flags  a list of flags
     */
    void setFlags(const QList<QByteArray> &flags);
    /*!
     * The flags that will be set on the appended message.
     */
    [[nodiscard]] QList<QByteArray> flags() const;

    /*!
     * Set the internal date that should be applied to the appended message.
     *
     * This is the date/time the IMAP server should set internally for the appended message.
     * See https://tools.ietf.org/html/rfc3501#section-6.3.11
     *
     * If this is not set, the server will use the current date/time.
     *
     * \a internalDate  the internal date
     *
     * \since 4.13
     */
    void setInternalDate(const QDateTime &internalDate);

    /*!
     * The internal date that will be set on the appended message.
     *
     * \since 4.13
     */
    [[nodiscard]] QDateTime internalDate() const;

    /*!
     * The content of the message.
     *
     * This should be in RFC-2822 format, although some required header
     * lines may be omitted in certain cases, for example when appending
     * to a Drafts folder.
     *
     * \a content  usually an RFC-2822 message
     */
    void setContent(const QByteArray &content);
    /*!
     * The content that the message will have.
     */
    [[nodiscard]] QByteArray content() const;

    /*!
     * The UID of the new message.
     *
     * This will be zero if it is unknown.
     *
     * The UID will not be known until the job has been successfully
     * executed, and it will only be known at all if the server
     * supports the UIDPLUS extension (RFC 4315).
     */
    [[nodiscard]] qint64 uid() const;

protected:
    void doStart() override;
    void handleResponse(const Response &response) override;
};

}
