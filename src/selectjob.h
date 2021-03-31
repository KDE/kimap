/*
    SPDX-FileCopyrightText: 2009 Kevin Ottens <ervin@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "kimap_export.h"

#include "fetchjob.h"
#include "job.h"

namespace KIMAP
{
class Session;
struct Response;
class SelectJobPrivate;
class ImapSet;

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

    /**
     * Set Quick Resynchronization parameters.
     *
     * Requires that the server supports the QRESYNC extension as defined in RFC5162
     * and the QRESYNC extension has been enabled via EnableJob.
     *
     * Using this option implies enabling CONDSTORE.
     *
     * @param lastUidvalidity Last UIDValidity value known to the client
     * @param lastModseq Last modification sequence number known to the client
     * @param knownUids List of all UIDs known to the client (optional).
     *
     * @see KIMAP::EnableJob
     */
    void setQResync(qint64 lastUidvalidity, quint64 lastModseq, const ImapSet &knownUids = ImapSet{});

Q_SIGNALS:
    /**
     * Emitted when the server provides a list of UIDs that have vanished since last sync.
     *
     * This feature requires that the QRESYNC parameters have been provided
     * to the SELECT command. This signal may not be emitted if no messages
     * have been expunged since the last check.
     *
     * @see setQResync()
     * @since 5.16
     */
    void vanished(const KIMAP::ImapSet &set);

    /**
     * Emitted when the server provides a list of messages that have changed or appeared
     * in the mailbox since the last sync.
     *
     * This feature requires that the QRESYNC parameters have been provided
     * to the SELECT command. The signal may not be emitted if no messages
     * have been modified or appended to the mailbox.
     *
     * @see setQResync()
     * @since 5.16
     */
    void modified(const QMap<qint64, KIMAP::Message> &messages);

protected:
    void doStart() override;
    void handleResponse(const Response &response) override;
};

}

