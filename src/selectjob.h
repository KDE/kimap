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
struct Response;
class SelectJobPrivate;

class KIMAP_EXPORT SelectJob : public Job
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(SelectJob)

    friend class SessionPrivate;

public:
    explicit SelectJob(Session *session);
    ~SelectJob() override;

    void setMailBox(const QString &mailBox);
    Q_REQUIRED_RESULT QString mailBox() const;

    void setOpenReadOnly(bool readOnly);
    /**
     * @return Returns whether the mailbox is opened in read-only mode. Note
     * that this can return true even if setOpenReadOnly() was set to false,
     * as the mailbox may be read-only on the server.
     */
    Q_REQUIRED_RESULT bool isOpenReadOnly() const;

    Q_REQUIRED_RESULT QList<QByteArray> flags() const;
    Q_REQUIRED_RESULT QList<QByteArray> permanentFlags() const;

    Q_REQUIRED_RESULT int messageCount() const;
    Q_REQUIRED_RESULT int recentCount() const;
    Q_REQUIRED_RESULT int firstUnseenIndex() const;

    Q_REQUIRED_RESULT qint64 uidValidity() const;
    Q_REQUIRED_RESULT qint64 nextUid() const;

    /**
     * @return Highest mod-sequence value of all messages in the mailbox or 0
     * if the server does not have CONDSTORE capability (RFC4551) or does not
     * support persistent storage of mod-sequences.
     *
     * @since 4.12
     */
    Q_REQUIRED_RESULT quint64 highestModSequence() const;

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
    Q_REQUIRED_RESULT bool condstoreEnabled() const;

protected:
    void doStart() override;
    void handleResponse(const Response &response) override;
};

}

#endif
