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

#ifndef KIMAP_DELETEJOB_H
#define KIMAP_DELETEJOB_H

#include "kimap_export.h"

#include "job.h"

namespace KIMAP
{

class Session;
class DeleteJobPrivate;

/**
 * Delete a mailbox
 *
 * Note that some servers will refuse to delete a
 * mailbox unless it is empty (ie: all mails have
 * had their \Deleted flag set, and then the
 * mailbox has been expunged).
 *
 * This job can only be run when the session is in the
 * authenticated (or selected) state.
 *
 * If the server supports ACLs, you will need the
 * Acl::DeleteMailbox right on the mailbox.
 */
class KIMAP_EXPORT DeleteJob : public Job
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(DeleteJob)

    friend class SessionPrivate;

public:
    explicit DeleteJob(Session *session);
    virtual ~DeleteJob();

    /**
     * Set the mailbox to delete.
     */
    void setMailBox(const QString &mailBox);
    /**
     * The mailbox that will be deleted.
     */
    QString mailBox() const;

protected:
    virtual void doStart() Q_DECL_OVERRIDE;
};

}

#endif
