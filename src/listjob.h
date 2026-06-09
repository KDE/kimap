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
 * \brief Describes a mailbox name and separator.
 */
struct KIMAP_EXPORT MailBoxDescriptor {
    /*!
     * \variable KIMAP::MailBoxDescriptor::name
     */
    QString name;

    /*!
     * \variable KIMAP::MailBoxDescriptor::separator
     */
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
 * List return options, require LIST-EXTENDED support
 * \value Subscribed adds \Subscribed flags in the response
 * \value Children adds \HasChildren and \HasNoChildren flags in the response
 * \value Status will also fetch mailbox status
 * \since 6.8
 */
namespace ListReturnOptions
{
struct Subscribed {
};
struct Children {
};
struct Status {
    bool messages = false;
    bool uidNext = false;
    bool uidValidity = false;
    bool unseen = false;
    bool deleted = false;
    bool size = false;
};
}

/*!
 * \class KIMAP::ListJob
 * \inmodule KIMAP
 * \inheaderfile KIMAP/ListJob
 *
 * \brief The ListJob class.
 */
class KIMAP_EXPORT ListJob : public Job
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(ListJob)

    friend class SessionPrivate;

public:
    /*!
     * \typealias KIMAP::ListJob::MailboxStatus
     * \since 6.8
     */
    using MailboxStatus = QList<QPair<QByteArray, qint64>>;

    /*!
     * \value NoOption only subscribed mailboxes. (Uses the LSUB IMAP command.)
     * \value IncludeUnsubscribed subscribed and unsubscribed mailboxes. (Uses the LIST IMAP command.)
     * \value IncludeFolderRoleFlags List subscribed and unsubscribed mailboxes with flags to identify standard mailboxes whose name may be localized. The
     * server must support the XLIST extension.
     */
    enum Option {
        NoOption = 0x0,
        IncludeUnsubscribed,
        IncludeFolderRoleFlags
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
    void setListExtendedEnabled(bool enabled);

    /*!
     */
    [[nodiscard]] bool listExtendedEnabled() const;

    /*!
     * Sets a new return options list
     * \since 6.8
     */
    template<typename... Ts>
    void setReturnOptions(Ts &&...vals)
    {
        clearReturnOptions();
        (setReturnOption(std::forward<Ts>(vals)), ...);
    }

    /*!
     * Removes all return options
     * \since 6.8
     */
    void clearReturnOptions();

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

    /*!
     */
    void mailBoxesStatusReceived(const QList<KIMAP::MailBoxDescriptor> &descriptors,
                                 const QList<QList<QByteArray>> &flags,
                                 const QList<std::optional<MailboxStatus>> &status);

protected:
    void doStart() override;
    void handleResponse(const Response &response) override;

private:
    /*!
     * \brief Converts a mailbox descriptor's name to uppercase if it is the Inbox or an Inbox subfolder.
     *  This is according to the RFC3501, 5.1. Mailbox Naming section.
     *
     * \a descriptor the descriptor to convert, conversion happens in place
     *
     * \internal
     **/
    void convertInboxName(KIMAP::MailBoxDescriptor &descriptor);

    /*!
     * \internal
     * \since 6.8
     */
    void setReturnOption(const ListReturnOptions::Subscribed &opt);

    /*!
     * \internal
     * \since 6.8
     */
    void setReturnOption(const ListReturnOptions::Children &opt);

    /*!
     * \internal
     * \since 6.8
     */
    void setReturnOption(const ListReturnOptions::Status &opt);
};

}
