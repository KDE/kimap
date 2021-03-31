/*
    SPDX-FileCopyrightText: 2016 Daniel Vrátil <dvratil@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "kimap_export.h"

#include "imapset.h"
#include "job.h"

namespace KIMAP
{
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
