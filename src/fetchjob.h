/*
    SPDX-FileCopyrightText: 2009 Kevin Ottens <ervin@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "kimap_export.h"

#include "imapset.h"
#include "job.h"

#include <KMime/Content>
#include <KMime/Message>

#include <tuple>

namespace KIMAP
{
class Session;
struct Response;
class FetchJobPrivate;

using MessageParts = QMap<QByteArray, std::shared_ptr<KMime::Content>>;

using MessageFlags = QList<QByteArray>;

using MessageAttribute = QPair<QByteArray, QVariant>;

struct Message {
    inline bool operator==(const Message &other) const
    {
        return std::tie(uid, size, flags, attributes, parts, message)
            == std::tie(other.uid, other.size, other.flags, other.attributes, other.parts, other.message);
    }

    qint64 uid = -1;
    qint64 size = 0;
    MessageFlags flags;
    QMap<QByteArray, QVariant> attributes;
    MessageParts parts;
    std::shared_ptr<KMime::Message> message;
};

/*!
 * \class KIMAP::FetchJob
 * \inmodule KIMAP
 * \inheaderfile KIMAP/FetchJob
 *
 * Fetch message data from the server
 *
 * All data is returned using the signals, so you need to connect to
 * the relevant signal (or all of them) before starting the job.
 *
 * This job will always use BODY.PEEK rather than BODY to fetch message
 * content, so it will not set the \Seen flag.
 *
 * This job can only be run when the session is in the selected state.
 */
class KIMAP_EXPORT FetchJob : public Job
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(FetchJob)

    friend class SessionPrivate;

public:
    /*!
     * Used to indicate what message data should be fetched.
     *
     * This doesn't provide the same fine-grained control over
     * what is fetched that the IMAP FETCH command normally
     * does, but the common cases are catered for.
     */
    class KIMAP_EXPORT FetchScope
    {
    public:
        FetchScope();

        /*!
         * Used to indicate what part of the message should be fetched.
         */
        enum Mode {
            /*!
             * Fetch RFC-2822 or MIME message headers.
             *
             * To fetch MIME headers for a MIME part, populate the \a parts field.
             *
             * If the RFC-2822 headers are requested (so \a parts is empty), the
             * returned information is:
             * - To, From, Message-id, References In-Reply-To, Subject and Date headers
             * - The message size (in octets)
             * - The internal date of the message
             * - The message flags
             * - The message UID
             */
            Headers,
            /*!
             * Fetch the message flags (the UID is also fetched)
             */
            Flags,
            /*!
             * Fetch the MIME message body structure (the UID is also fetched)
             */
            Structure,
            /*!
             * Fetch the message content (the UID is also fetched)
             *
             * To fetch only certain MIME parts (see Structure), populate the
             * \a parts field.
             */
            Content,
            /*!
             * Fetch the complete message.
             */
            Full,
            /*!
             * Fetch the message MIME headers and the content of parts specified in the \a parts
             * field.
             *
             * If \a parts is empty, this mode will return the full message, just like
             * FetchScope::Content
             *
             * Use case:
             * -# Start a FetchJob with the FetchScope::Structure mode to retrieve the structure
             *    of the message.
             * -# Parse the structure to identify the parts that are interesting (ie: probably
             *    everything but attachments).
             * -# Start another FetchJob with FetchScope::HeaderAndContent to fetch those parts.
             * -# At the request of the user, you can repeat the step above to fetch the attachments.
             *
             * \since 4.7
             */
            HeaderAndContent,

            /*!
             * Fetch message size (in octets), internal date of the message, flags, UID
             * and all RFC822 headers.
             *
             * The \a parts field is ignored when using this scope
             *
             * \since 4.12
             */
            FullHeaders
        };

        /*!
         * Specify which message parts to operate on.
         *
         * This refers to multipart-MIME message parts or MIME-IMB encapsulated
         * message parts.
         *
         * Note that this is ignored unless \a mode is Headers or Content.
         *
         * If \a mode is Headers, this sets the parts to get the MIME headers
         * for.  If this list is empty, the headers for the whole message
         * (the RFC-2822 headers) are fetched.
         *
         * If \a mode is Content, this sets the parts to fetch.  Parts are
         * fetched wholesale.  If this list is empty, the whole message body
         * is fetched (all MIME parts together).
         */
        QList<QByteArray> parts;
        /*!
         * Specify what message data should be fetched.
         */
        Mode mode = Content;

        /*!
         * Specify to fetch only items with mod-sequence higher then \a changedSince.
         *
         * The server must have CONDSTORE capability (RFC4551).
         *
         * Default value is 0 (ignored).
         *
         * \since 4.12
         */
        quint64 changedSince = 0;

        /*!
         * Specify whether QRESYNC is supported and should be used.
         *
         * When enabled, the \a changedSince parameter must be specified as
         * well. The server will then also return list of messages that have
         * been deleted from the mailbox since the specified modification sequence.
         *
         * The server must have QRESYNC capability (RFC5162) and it must have
         * explicitly been enabled via ENABLE command (see @EnableJob).
         *
         * QRESYNC can only be used in UID FETCH (\sa setUidBased())
         *
         * \since 5.16
         */
        bool qresync = false;
    };

    explicit FetchJob(Session *session);
    ~FetchJob() override = default;

    /*!
     * Set which messages to fetch data for.
     *
     * If sequence numbers are given, isUidBased() should be false.  If UIDs
     * are given, isUidBased() should be true.
     *
     * \a set  the sequence numbers or UIDs of the messages to fetch data for
     */
    void setSequenceSet(const ImapSet &set);
    /*!
     * The messages that will be fetched.
     */
    [[nodiscard]] ImapSet sequenceSet() const;

    /*!
     * Set how the sequence set should be interpreted.
     *
     * \a uidBased  if \\ true the argument to setSequenceSet will be
     *                  interpreted as UIDs, if \\ false it will be interpreted
     *                  as sequence numbers
     */
    void setUidBased(bool uidBased);
    /*!
     * How to interpret the sequence set.
     *
     * Returns  if \\ true the result of sequenceSet() should be
     *          interpreted as UIDs, if \\ false it should be interpreted
     *          as sequence numbers
     */
    [[nodiscard]] bool isUidBased() const;

    /*!
     * Sets what data should be fetched.
     *
     * The default scope is FetchScope::Content (all content parts).
     *
     * \a scope  a FetchScope object describing what data
     *               should be fetched
     */
    void setScope(const FetchScope &scope);
    /*!
     * Specifies what data will be fetched.
     */
    [[nodiscard]] FetchScope scope() const;

    // TODO: KF6: Move this to FetchScope
    /*!
     * Enables retrieving of Gmail-specific extensions
     *
     * The FETCH response will contain X-GM-MSGID, X-GM-THRID and X-GM-LABELS
     *
     * Do NOT enable this, unless talking to Gmail servers, otherwise the
     * request may fail.
     *
     * \a enabled Whether the Gmail support should be enabled
     * \since 4.14
     */
    void setGmailExtensionsEnabled(bool enabled);

    /*!
     * Returns whether Gmail support is enabled
     *
     * \since 4.14
     * \sa setGmailExtensionsEnabled()
     */
    [[nodiscard]] bool setGmailExtensionsEnabled() const;

    /*!
     * Returns the name of the mailbox the fetch job is executed on.
     *
     * Can only be accessed after the job is actually started, before that
     * returns an empty string.
     *
     * \since 5.6
     */
    [[nodiscard]] QString mailBox() const;

Q_SIGNALS:
    /*!
     * Provides received messages.
     *
     * This signal is emitted when some data are received. The signal can be
     * emitted multiple times as the messages are being received.
     *
     * \a messages A map from message sequence number to message. Not all
     *                 fields may be populated, depending on the fetch scope.
     *
     * \since 5.6
     */
    void messagesAvailable(const QMap<qint64, KIMAP::Message> &messages);

    /*!
     * Provides vanished messages.
     *
     * This signal is emitted when QRESYNC capability (RFC5162) is available and has
     * bee enabled on the server, and \a FetchScope::qresync has been set to \a true.
     * It contains a list of messages that have vanished from the mailbox since the
     * last modification sequence specified in \a FetchScope::changedSince.
     *
     * \a uids UIDs of messages that have been removed from the mailbox since
     *             the specified modification sequence.
     *
     * \since 5.16
     */
    void messagesVanished(const KIMAP::ImapSet &uids);

protected:
    void doStart() override;
    void handleResponse(const Response &response) override;

private:
    Q_PRIVATE_SLOT(d_func(), void emitPendings())
};

}
