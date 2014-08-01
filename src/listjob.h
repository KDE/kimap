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

#ifndef KIMAP_LISTJOB_H
#define KIMAP_LISTJOB_H

#include "kimap_export.h"

#include "job.h"

namespace KIMAP
{

class Session;
struct Message;
class ListJobPrivate;

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

class KIMAP_EXPORT ListJob : public Job
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(ListJob)

    friend class SessionPrivate;

public:
    enum Option {
        NoOption = 0x0,         /**< List only subscribed mailboxes. (Uses the LSUB IMAP command.) */
        IncludeUnsubscribed,    /**< List subscribed and unsubscribed mailboxes. (Uses the LIST IMAP command.) */
        IncludeFolderRoleFlags  /**< List subscribed and unsubscribed mailboxes with flags to identify standard mailboxes whose name may be localized.
                                   The server must support the XLIST extension. */
    };

    explicit ListJob(Session *session);
    virtual ~ListJob();

    KIMAP_DEPRECATED void setIncludeUnsubscribed(bool include);
    KIMAP_DEPRECATED bool isIncludeUnsubscribed() const;

    void setOption(Option option);
    Option option() const;

    void setQueriedNamespaces(const QList<MailBoxDescriptor> &namespaces);
    QList<MailBoxDescriptor> queriedNamespaces() const;

    KIMAP_DEPRECATED QList<MailBoxDescriptor> mailBoxes() const;
    KIMAP_DEPRECATED QMap< MailBoxDescriptor, QList<QByteArray> > flags() const;

Q_SIGNALS:
    void mailBoxesReceived(const QList<KIMAP::MailBoxDescriptor> &descriptors,
                           const QList< QList<QByteArray> > &flags);

protected:
    virtual void doStart();
    virtual void handleResponse(const Message &response);

private:
    Q_PRIVATE_SLOT(d_func(), void emitPendings())

    /**
    * @brief Converts a mailbox descriptor's name to uppercase if it is the Inbox or an Inbox subfolder.
    *  This is according to the RFC3501, 5.1. Mailbox Naming section.
    *
    * @param descriptor the descriptor to convert, conversion happens in place
    **/
    void convertInboxName(KIMAP::MailBoxDescriptor &descriptor);
};

}

#endif
