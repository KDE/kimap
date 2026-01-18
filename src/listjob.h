/*
    SPDX-FileCopyrightText: 2009 Kevin Ottens <ervin@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "kimap_export.h"

#include "job.h"

namespace KIMAP
{
class Session;
struct Response;
class ListJobPrivate;

/*!
 * \struct KIMAP::MailBoxDescriptor
 * \inmodule KIMAP
 * \inheaderfile KIMAP/ListJob
 *
 * Describes a mailbox name and separator.
 */
struct KIMAP_EXPORT MailBoxDescriptor {
    QString name;
    QChar separator;

    inline bool operator==(const MailBoxDescriptor &other) const
    {
        return other.name == name && other.separator == separator;
    }

    inline bool operator<(const MailBoxDescriptor &other) const
    {
        return other.name < name || (other.name == name && other.separator < separator);
    }
};
/*!
 * \class KIMAP::ListJob
 * \inmodule KIMAP
 * \inheaderfile KIMAP/ListJob
 *
 * \brief The ListJob class
 */
class KIMAP_EXPORT ListJob : public Job
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(ListJob)

    friend class SessionPrivate;

public:
    enum Option {
        NoOption = 0x0, /*!< List only subscribed mailboxes. (Uses the LSUB IMAP command.) */
        IncludeUnsubscribed, /*!< List subscribed and unsubscribed mailboxes. (Uses the LIST IMAP command.) */
        IncludeFolderRoleFlags /*!< List subscribed and unsubscribed mailboxes with flags to identify standard mailboxes whose name may be localized.
                                  The server must support the XLIST extension. */
    };

    /*!
     */
    explicit ListJob(Session *session);
    /*!
     */
    ~ListJob() override;

    /*!
     */
    void setOption(Option option);
    /*!
     */
    [[nodiscard]] Option option() const;

    /*!
     */
    void setQueriedNamespaces(const QList<MailBoxDescriptor> &namespaces);
    /*!
     */
    [[nodiscard]] QList<MailBoxDescriptor> queriedNamespaces() const;

Q_SIGNALS:
    /*!
     */
    void mailBoxesReceived(const QList<KIMAP::MailBoxDescriptor> &descriptors, const QList<QList<QByteArray>> &flags);

protected:
    void doStart() override;
    void handleResponse(const Response &response) override;

private:
    /*!
     * \brief Converts a mailbox descriptor's name to uppercase if it is the Inbox or an Inbox subfolder.
     *  This is according to the RFC3501, 5.1. Mailbox Naming section.
     *
     * \a descriptor the descriptor to convert, conversion happens in place
     **/
    void convertInboxName(KIMAP::MailBoxDescriptor &descriptor);
};

}
