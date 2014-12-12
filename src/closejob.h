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

#ifndef KIMAP_CLOSEJOB_H
#define KIMAP_CLOSEJOB_H

#include "kimap_export.h"

#include "job.h"

namespace KIMAP
{

class Session;
struct Message;
class CloseJobPrivate;

/**
 * Closes the current mailbox.
 *
 * This job can only be run when the session is in the selected state.
 *
 * Permanently removes all messages that have the \\Deleted
 * flag set from the currently selected mailbox, and returns
 * to the authenticated state from the selected state.
 *
 * The server will not provide any notifications of which
 * messages were expunged, so this is quicker than doing
 * an expunge and then implicitly closing the mailbox
 * (by selecting or examining another mailbox or logging
 * out).
 *
 * No messages are removed if the mailbox is open in a read-only
 * state, or if the server supports ACLs and the user does not
 * have the Acl::Expunge right on the mailbox.
 */
class KIMAP_EXPORT CloseJob : public Job
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(CloseJob)

    friend class SessionPrivate;

public:
    explicit CloseJob(Session *session);
    virtual ~CloseJob();

protected:
    void doStart() Q_DECL_OVERRIDE;
};

}

#endif
