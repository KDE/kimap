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

#ifndef KIMAP_COPYJOB_H
#define KIMAP_COPYJOB_H

#include "kimap_export.h"

#include "job.h"
#include "imapset.h"

namespace KIMAP
{

class Session;
struct Message;
class CopyJobPrivate;

/**
 * Copies one or more messages to another mailbox.
 *
 * This job can only be run when the session is in the selected state.
 *
 * If the server supports ACLs, the user will need the
 * Acl::Insert right on the target mailbox.
 * In order to preserve message flags, the user may also need
 * some combination of Acl::DeleteMessage,
 * Acl::KeepSeen and Acl::Write on the
 * target mailbox.
 */
class KIMAP_EXPORT CopyJob : public Job
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(CopyJob)

    friend class SessionPrivate;

public:
    explicit CopyJob(Session *session);
    virtual ~CopyJob();

    /**
     * Sets the destination mailbox.
     *
     * If the mailbox does not exist, the server should not create
     * it automatically and the job should fail.  Note, however,
     * that a conforming server may create the mailbox automatically.
     *
     * @param mailBox  the (unquoted) name of the mailbox where the
     *                 messages should be copied to
     */
    void setMailBox(const QString &mailBox);
    /**
     * The destination mailbox
     */
    QString mailBox() const;

    /**
     * Sets the messages to be copied
     *
     * If sequence numbers are given, isUidBased() should be false.  If UIDs
     * are given, isUidBased() should be true.
     *
     * RFC 3501 is unclear as to what should happen if invalid sequence numbers
     * are passed.  If non-existent UIDs are passed, they will be ignored.
     *
     * @param set  the sequence numbers or UIDs of the messages to be copied
     */
    void setSequenceSet(const ImapSet &set);
    /**
     * The messages that will be copied.
     *
     * isUidBased() can be used to check whether the ImapSet contains
     * sequence numbers or UIDs.
     *
     * @return  the sequence numbers or UIDs of the messages to be copied
     */
    ImapSet sequenceSet() const;

    /**
     * Set how the sequence set should be interpreted.
     *
     * @param uidBased  if @c true the argument to setSequenceSet will be
     *                  interpreted as UIDs, if @c false it will be interpreted
     *                  as sequence numbers
     */
    void setUidBased(bool uidBased);
    /**
     * How to interpret the sequence set.
     *
     * @return  if @c true the result of sequenceSet() should be
     *          interpreted as UIDs, if @c false it should be interpreted
     *          as sequence numbers
     */
    bool isUidBased() const;

    /**
     * The UIDs of the new copies of the messages
     *
     * This will be an empty set if no messages have been copied yet
     * or if the server does not support the UIDPLUS extension.
     */
    ImapSet resultingUids() const;

protected:
    virtual void doStart();
    virtual void handleResponse(const Message &response);
};

}

#endif
