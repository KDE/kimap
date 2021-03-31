/*
    SPDX-FileCopyrightText: 2009 Kevin Ottens <ervin@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "kimap_export.h"

#include "imapset.h"
#include "job.h"

#include <kmime/kmime_content.h>
#include <kmime/kmime_message.h>

#include <tuple>

namespace KIMAP
{
class Session;
struct Response;
class FetchJobPrivate;

using ContentPtr = QSharedPointer<KMime::Content>;
using MessageParts = QMap<QByteArray, ContentPtr>;

using MessagePtr = QSharedPointer<KMime::Message>;
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
    MessagePtr message;
};

/**
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
    /**
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

        /**
         * Used to indicate what part of the message should be fetched.
         */
        enum Mode {
            /**
             * Fetch RFC-2822 or MIME message headers.
             *
             * To fetch MIME headers for a MIME part, populate the @p parts field.
             *
             * If the RFC-2822 headers are requested (so @p parts is empty), the
             * returned information is:
             * - To, From, Message-id, References In-Reply-To, Subject and Date headers
             * - The message size (in octets)
             * - The internal date of the message
             * - The message flags
             * - The message UID
             */
            Headers,
            /**
             * Fetch the message flags (the UID is also fetched)
             */
            Flags,
            /**
             * Fetch the MIME message body structure (the UID is also fetched)
             */
            Structure,
            /**
             * Fetch the message content (the UID is also fetched)
             *
             * To fetch only certain MIME parts (see Structure), populate the
             * @p parts field.
             */
            Content,
            /**
             * Fetch the complete message.
             */
            Full,
            /**
             * Fetch the message MIME headers and the content of parts specified in the @p parts
             * field.
             *
             * If @p parts is empty, this mode will return the full message, just like
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
             * @since 4.7
             */
            HeaderAndContent,

            /**
             * Fetch message size (in octets), internal date of the message, flags, UID
             * and all RFC822 headers.
             *
             * The @p parts field is ignored when using this scope
             *
             * @since 4.12
             */
            FullHeaders
        };

        /**
         * Specify which message parts to operate on.
         *
         * This refers to multipart-MIME message parts or MIME-IMB encapsulated
         * message parts.
         *
         * Note that this is ignored unless @p mode is Headers or Content.
         *
         * If @p mode is Headers, this sets the parts to get the MIME headers
         * for.  If this list is empty, the headers for the whole message
         * (the RFC-2822 headers) are fetched.
         *
         * If @p mode is Content, this sets the parts to fetch.  Parts are
         * fetched wholesale.  If this list is empty, the whole message body
         * is fetched (all MIME parts together).
         */
        QList<QByteArray> parts;
        /**
         * Specify what message data should be fetched.
         */
        Mode mode = Content;

        /**
         * Specify to fetch only items with mod-sequence higher then @p changedSince.
         *
         * The server must have CONDSTORE capability (RFC4551).
         *
         * Default value is 0 (ignored).
         *
         * @since 4.12
         */
        quint64 changedSince = 0;

        /**
         * Specify whether QRESYNC is supported and should be used.
         *
         * When enabled, the @p changedSince parameter must be specified as
         * well. The server will then also return list of messages that have
         * been deleted from the mailbox since the specified modification sequence.
         *
         * The server must have QRESYNC capability (RFC5162) and it must have
         * explicitly been enabled via ENABLE command (see @EnableJob).
         *
         * QRESYNC can only be used in UID FETCH (@see setUidBased())
         *
         * @since 5.16
         */
        bool qresync = false;
    };

    explicit FetchJob(Session *session);
    ~FetchJob() override = default;

    /**
     * Set which messages to fetch data for.
     *
     * If sequence numbers are given, isUidBased() should be false.  If UIDs
     * are given, isUidBased() should be true.
     *
     * @param set  the sequence numbers or UIDs of the messages to fetch data for
     */
    void setSequenceSet(const ImapSet &set);
    /**
     * The messages that will be fetched.
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
     * Sets what data should be fetched.
     *
     * The default scope is FetchScope::Content (all content parts).
     *
     * @param scope  a FetchScope object describing what data
     *               should be fetched
     */
    void setScope(const FetchScope &scope);
    /**
     * Specifies what data will be fetched.
     */
    FetchScope scope() const;

    // TODO: KF5: Move this to FetchScope
    /**
     * Enables retrieving of Gmail-specific extensions
     *
     * The FETCH response will contain X-GM-MSGID, X-GM-THRID and X-GM-LABELS
     *
     * Do NOT enable this, unless talking to Gmail servers, otherwise the
     * request may fail.
     *
     * @param enabled Whether the Gmail support should be enabled
     * @since 4.14
     */
    void setGmailExtensionsEnabled(bool enabled);

    /**
     * Returns whether Gmail support is enabled
     *
     * @since 4.14
     * @see setGmailExtensionsEnabled()
     */
    bool setGmailExtensionsEnabled() const;

    /**
     * Returns the name of the mailbox the fetch job is executed on.
     *
     * Can only be accessed after the job is actually started, before that
     * returns an empty string.
     *
     * @since 5.6
     */
    QString mailBox() const;

Q_SIGNALS:
    /**
     * Provides header and message results.
     *
     * This signal will be emitted if the requested scope mode
     * was FetchScope::Full, FetchScope::Flags or
     * FetchScope::Headers with no parts specified
     *
     * This signal may be emitted any number of times before
     * the result() signal is emitted.  The result() signal will
     * only be emitted once all results have been reported via
     * one of the signals.
     *
     * Note that, depending on the scope, some of the parameters
     * of this signal may be empty maps.
     *
     * @param mailBox  the name of the mailbox the fetch job was
     *                 executed on
     * @param uids     a map from message sequence numbers to message UIDs;
     *                 this will always be populated
     * @param sizes    a map from message sequence numbers to message sizes
     *                 (sizes are in octets and refer to the transfer encoding of
     *                 the message); populated if the scope is FetchScope::Full or
     *                 FetchScope::Headers
     * @param flags    a map from message sequence numbers to message flags;
     *                 populated if the scope is FetchScope::Flags, FetchScope::Full
     *                 of FetchScope::Headers
     * @param messages a map from message sequence numbers to message contents (including
     *                 headers); populated if the scope is FetchScope::Full,
     *                 FetchScope::Headers or FetchScope::Structure
     *
     * @deprecated Use messagesAvailable() instead.
     */
    KIMAP_DEPRECATED
    void headersReceived(const QString &mailBox,
                         const QMap<qint64, qint64> &uids,
                         const QMap<qint64, qint64> &sizes,
                         const QMap<qint64, KIMAP::MessageFlags> &flags,
                         const QMap<qint64, KIMAP::MessagePtr> &messages);

    /**
     * An overloaded version of headersReceived(), which includes additional attribute
     * specified in the FETCH response, but that don't belong to actual content of the
     * message.
     *
     * @param mailBox  the name of the mailbox the fetch job was
     *                 executed on
     * @param uids     a map from message sequence numbers to message UIDs;
     *                 this will always be populated
     * @param attrs    a map from message sequence numbers to a pair of attribute
     *                 name and value
     * @param sizes    a map from message sequence numbers to message sizes
     *                 (sizes are in octets and refer to the transfer encoding of
     *                 the message); populated if the scope is FetchScope::Full or
     *                 FetchScope::Headers
     * @param flags    a map from message sequence numbers to message flags;
     *                 populated if the scope is FetchScope::Flags, FetchScope::Full
     *                 of FetchScope::Headers
     * @param messages a map from message sequence numbers to message contents (including
     *                 headers); populated if the scope is FetchScope::Full,
     *                 FetchScope::Headers or FetchScope::Structure
     *
     * @overload
     * @since 4.14
     * @deprecated Use messagesAvailable() instead.
     */
    KIMAP_DEPRECATED
    void headersReceived(const QString &mailBox,
                         const QMap<qint64, qint64> &uids,
                         const QMap<qint64, qint64> &sizes,
                         const QMap<qint64, KIMAP::MessageAttribute> &attrs,
                         const QMap<qint64, KIMAP::MessageFlags> &flags,
                         const QMap<qint64, KIMAP::MessagePtr> &messages);

    /**
     * Provides header and message results.
     *
     * This signal will be emitted if the requested scope mode
     * was FetchScope::Content or FetchScope::Headers with no
     * parts specified or FetchScope::Structure.
     *
     * This signal may be emitted any number of times before
     * the result() signal is emitted.  The result() signal will
     * only be emitted once all results have been reported via
     * one of the signals.
     *
     *
     * @param mailBox  the name of the mailbox the fetch job was
     *                 executed on
     * @param uids     a map from message sequence numbers to message UIDs
     * @param messages a map from message sequence numbers to message contents
     *
     * @deprecated Use messagesAvailable() instead.
     */
    KIMAP_DEPRECATED
    void messagesReceived(const QString &mailBox, const QMap<qint64, qint64> &uids, const QMap<qint64, KIMAP::MessagePtr> &messages);

    /**
     * An overloaded version of messagesReceived(), which includes additional attribute
     * specified in the FETCH response, but that don't belong to actual content of the
     * message.
     *
     * @param mailBox  the name of the mailbox the fetch job was
     *                 executed on
     * @param uids     a map from message sequence numbers to message UIDs
     * @param attrs    a map from message sequence numbers to pair of attribute
     *                 name and it's value
     * @param messages a map from message sequence numbers to message contents
     *
     * @overload
     * @since 4.14
     *
     * @deprecated Use messagesAvailable() instead.
     */
    KIMAP_DEPRECATED
    void messagesReceived(const QString &mailBox,
                          const QMap<qint64, qint64> &uids,
                          const QMap<qint64, KIMAP::MessageAttribute> &attrs,
                          const QMap<qint64, KIMAP::MessagePtr> &messages);
    /**
     * Provides header and message results.
     *
     * This signal will be emitted if the requested scope mode
     * was FetchScope::Content or FetchScope::Headers with
     * specified parts.
     *
     * This signal may be emitted any number of times before
     * the result() signal is emitted.  The result() signal will
     * only be emitted once all results have been reported via
     * one of the signals.
     *
     * @param mailBox  the name of the mailbox the fetch job was
     *                 executed on
     * @param uids     a map from message sequence numbers to message UIDs
     * @param parts    a map from message sequence numbers to message part collections
     *
     * @deprecated Use messagesAvailable() instead.
     */
    KIMAP_DEPRECATED
    void partsReceived(const QString &mailBox, const QMap<qint64, qint64> &uids, const QMap<qint64, KIMAP::MessageParts> &parts);

    /**
     * An overloaded version of partsReceived(), which includes additional attribute
     * specified in the FETCH response, but that don't belong to actual content of the
     * message.
     *
     * @param mailBox  the name of the mailbox the fetch job was
     *                 executed on
     * @param uids     a map from message sequence numbers to message UIDs
     * @param attrs    a map from message sequence numbers to pair of attribute
     * @param parts    a map from message sequence numbers to message part collections
     *
     * @overload
     * @since 4.14
     *
     * @deprecated Use messagesAvailable() instead.
     */
    KIMAP_DEPRECATED
    void partsReceived(const QString &mailBox,
                       const QMap<qint64, qint64> &uids,
                       const QMap<qint64, KIMAP::MessageAttribute> &attrs,
                       const QMap<qint64, KIMAP::MessageParts> &parts);

    /**
     * Provides received messages.
     *
     * This signal is emitted when some data are received. The signal can be
     * emitted multiple times as the messages are being received.
     *
     * @param messages A map from message sequence number to message. Not all
     *                 fields may be populated, depending on the fetch scope.
     *
     * @since 5.6
     */
    void messagesAvailable(const QMap<qint64, KIMAP::Message> &messages);

    /**
     * Provides vanished messages.
     *
     * This signal is emitted when QRESYNC capability (RFC5162) is available and has
     * bee enabled on the server, and @p FetchScope::qresync has been set to @p true.
     * It contains a list of messages that have vanished from the mailbox since the
     * last modification sequence specified in @p FetchScope::changedSince.
     *
     * @param uids UIDs of messages that have been removed from the mailbox since
     *             the specified modification sequence.
     *
     * @since 5.16
     */
    void messagesVanished(const KIMAP::ImapSet &uids);

protected:
    void doStart() override;
    void handleResponse(const Response &response) override;

private:
    Q_PRIVATE_SLOT(d_func(), void emitPendings())
};

}

