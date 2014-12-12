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

#ifndef KIMAP_SELECTJOB_H
#define KIMAP_SELECTJOB_H

#include "kimap_export.h"

#include "job.h"

namespace KIMAP
{

class Session;
struct Message;
class SelectJobPrivate;

class KIMAP_EXPORT SelectJob : public Job
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(SelectJob)

    friend class SessionPrivate;

public:
    explicit SelectJob(Session *session);
    virtual ~SelectJob();

    void setMailBox(const QString &mailBox);
    QString mailBox() const;

    void setOpenReadOnly(bool readOnly);
    bool isOpenReadOnly() const;

    QList<QByteArray> flags() const;
    QList<QByteArray> permanentFlags() const;

    int messageCount() const;
    int recentCount() const;
    int firstUnseenIndex() const;

    qint64 uidValidity() const;
    qint64 nextUid() const;

    /**
     * @return Highest mod-sequence value of all messages in the mailbox or 0
     * if the server does not have CONDSTORE capability (RFC4551) or does not
     * support persistent storage of mod-sequences.
     *
     * @since 4.12
     */
    quint64 highestModSequence() const;

    /**
     * Whether to append CONDSTORE parameter to the SELECT command.
     *
     * This option is false by default and can be enabled only when server
     * has CONDSTORE capability (RFC4551), otherwise the SELECT command will
     * fail.
     *
     * @since 4.12
     */
    void setCondstoreEnabled(bool enable);

    /**
     * Returns whether the CONDSTORE parameter will be appended to SELECT command
     *
     * @since 4.12
     */
    bool condstoreEnabled() const;

protected:
    void doStart() Q_DECL_OVERRIDE;
    void handleResponse(const Message &response) Q_DECL_OVERRIDE;
};

}

#endif
