/*
    Copyright (c) 2016 Daniel Vrátil <dvratil@kde.org>

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

#ifndef KIMAP_MOVEJOB_H_
#define KIMAP_MOVEJOB_H_

#include "kimap_export.h"

#include "job.h"
#include "imapset.h"

namespace KIMAP {

class MoveJobPrivate;

/**
 * Moves messages from current mailbox to another
 *
 * Note that move functionality is not specified in the base IMAP
 * protocol and is defined as an extension in RFC6851. That means
 * that the MoveJob can only be used when the server lists "MOVE"
 * in response to CAPABILITY command.
 *
 * Unlike the traditional emulation of moving messages, i.e. COPY + STORE + EXPUNGE,
 * MOVE guarantees the transaction to be atomic on the server.
 *
 * @since 5.4
 */
class KIMAP_EXPORT MoveJob : public Job
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(MoveJob)

    friend class SessionPrivate;

public:
    explicit MoveJob(Session *session);
    ~MoveJob() override;

    /**
     * Set the destination mailbox
     *
     * If the mailbox does not exist, the server should not create
     * it automatically and the job should fail.  Note, however,
     * that a conforming server may create the mailbox automatically.
     *
     * @param mailBox  the (unquoted) name of the mailbox where the
     *                 messages should be moved to
     */
    void setMailBox(const QString &mailbox);
    /**
     * The destination mailbox
     */
    Q_REQUIRED_RESULT QString mailBox() const;

    /**
     * Sets the messages to be moved,
     *
     * If sequence numbers are given, isUidBased() should be false.  If UIDs
     * are given, isUidBased() should be true.
     *
     * @param set  the sequence numbers or UIDs of the messages to be moved
     */
    void setSequenceSet(const ImapSet &set);
    /**
     * The messages that will be moved.
     *
     * isUidBased() can be used to check whether the ImapSet contains
     * sequence numbers or UIDs.
     *
     * @return  the sequence numbers or UIDs of the messages to be moved
     */
    Q_REQUIRED_RESULT ImapSet sequenceSet() const;

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
     * The UIDs of the moved messages in the destination mailbox.
     *
     * This will be an empty set if no messages have been moved yet
     * or if the server does not support the UIDPLUS extension.
     */
    Q_REQUIRED_RESULT ImapSet resultingUids() const;

protected:
    void doStart() override;
    void handleResponse(const Response &response) override;
};

}
#endif
