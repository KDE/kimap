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

#ifndef KIMAP_FETCHJOB_H
#define KIMAP_FETCHJOB_H

#include "kimap_export.h"

#include "imapset.h"
#include "job.h"

#include "kmime/kmime_content.h"
#include "kmime/kmime_message.h"

#include <boost/shared_ptr.hpp>

namespace KIMAP {

class Session;
struct Message;
class FetchJobPrivate;

typedef boost::shared_ptr<KMime::Content> ContentPtr;
typedef QMap<QByteArray, ContentPtr> MessageParts;

typedef boost::shared_ptr<KMime::Message> MessagePtr;
typedef QList<QByteArray> MessageFlags;

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
    struct FetchScope
    {
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
        Full
      } mode;
    };

    explicit FetchJob( Session *session );
    virtual ~FetchJob();

    /**
     * Set which messages to fetch data for.
     *
     * If sequence numbers are given, isUidBased() should be false.  If UIDs
     * are given, isUidBased() should be true.
     *
     * @param set  the sequence numbers or UIDs of the messages to fetch data for
     */
    void setSequenceSet( const ImapSet &set );
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
    void setScope( const FetchScope &scope );
    /**
     * Specifies what data will be fetched.
     */
    FetchScope scope() const;

    /**
     * All the messages that have been received.
     *
     * This may be empty, depending on the scope of the fetch.
     *
     * @return a map from message sequence numbers to message contents (including headers)
     */
    QMap<qint64, MessagePtr> messages() const;
    /**
     * All the parts that have been received.
     *
     * Empty unless parts were specified in the scope.
     *
     * @return a map from message sequence numbers to message part collections
     */
    QMap<qint64, MessageParts> parts() const;
    /**
     * All the flags that have been received.
     *
     * This may be empty, depending on the scope of the fetch.
     *
     * @return a map from message sequence numbers to message flags
     */
    QMap<qint64, MessageFlags> flags() const;
    /**
     * All the message sizes that have been received.
     *
     * This may be empty, depending on the scope of the fetch.
     *
     * @return a map from message sequence numbers to message sizes
     *         (sizes are in octets and refer to the transfer encoding of
     *         the message)
     */
    QMap<qint64, qint64> sizes() const;
    /**
     * All the message UIDs that have been received.
     *
     * This will be populated with all the messages that data was
     * fetched for.
     *
     * @return a map from message sequence numbers to message UIDs
     */
    QMap<qint64, qint64> uids() const;

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
     */
    void headersReceived( const QString &mailBox,
                          const QMap<qint64, qint64> &uids,
                          const QMap<qint64, qint64> &sizes,
                          const QMap<qint64, KIMAP::MessageFlags> &flags,
                          const QMap<qint64, KIMAP::MessagePtr> &messages );

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
     */
    void messagesReceived( const QString &mailBox,
                           const QMap<qint64, qint64> &uids,
                           const QMap<qint64, KIMAP::MessagePtr> &messages );

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
     */
    void partsReceived( const QString &mailBox,
                        const QMap<qint64, qint64> &uids,
                        const QMap<qint64, KIMAP::MessageParts> &parts );

  protected:
    virtual void doStart();
    virtual void handleResponse(const Message &response);

  private:
    Q_PRIVATE_SLOT( d_func(), void emitPendings() )
};

}

#endif
