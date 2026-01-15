/*
    SPDX-FileCopyrightText: 2009 Andras Mantia <amantia@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "kimap_export.h"

#include "metadatajobbase.h"

namespace KIMAP
{
class Session;
struct Response;
class GetMetaDataJobPrivate;

/*!
 * \class KIMAP::GetMetaDataJob
 * \inmodule KIMAP
 * \inheaderfile KIMAP/GetMetaDataJob
 *
 * Fetches mailbox metadata.
 *
 * Provides support for the IMAP METADATA extension; both the
 * final RFC version
 * (<a href="https://tools.ietf.org/html/rfc5464">RFC 5464</a>)
 * and the older, incompatible draft version (known as ANNOTATEMORE)
 * (<a
 * href="https://tools.ietf.org/html/draft-daboo-imap-annotatemore-07"
 * >draft-daboo-imap-annotatemore-07</a>).  See setServerCompatibility().
 *
 * This job can only be run when the session is in the
 * authenticated (or selected) state.
 *
 * If the server supports ACLs, the user will need the
 * Acl::Lookup right on the mailbox, as well as one of
 * - Acl::Read
 * - Acl::KeepSeen
 * - Acl::Write
 * - Acl::Insert
 * - Acl::Post
 * Otherwise, the user must be able to list the mailbox
 * and either read or write the message content.
 *
 * Note also that on servers that implement the Annotatemore
 * version of the extension, only Acl::Lookup rights are
 * required (ie: the user must be able to list the mailbox).
 */
class KIMAP_EXPORT GetMetaDataJob : public MetaDataJobBase
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(GetMetaDataJob)

    friend class SessionPrivate;

public:
    explicit GetMetaDataJob(Session *session);
    ~GetMetaDataJob() override;

    /*!
     * Used to specify the depth of the metadata hierarchy to walk.
     */
    enum Depth {
        NoDepth = 0, /*!< Only the requested entries */
        OneLevel, /*!< The requested entries and all their direct children */
        AllLevels /*!< The requested entries and all their descendants */
    };

    Q_DECLARE_FLAGS(Depths, Depth)

    /*!
     * Add an entry to the query list.
     *
     * See SetMetaDataJob for a description of metadata entry names.
     *
     * Note that this expects METADATA style entries (with a /shared or /private prefix typically).
     * In ANNOTATEMORE mode, this prefix is automatically replaced with an appropriate attribute.
     *
     * \a entry the metadata entry name
     */
    void addRequestedEntry(const QByteArray &entry);

    /*!
     * Limits the size of returned metadata entries.
     *
     * In order to save time or bandwidth, it is possible to prevent the
     * server from returning metadata entries that are larger than a
     * certain size.  These entries will simply not appear in the
     * list returned by allMetaData(), and will not be accessible using
     * metaData().
     *
     * Note that this is only used when the server capability mode is
     * Metadata.
     *
     * The default is no limit (-1).  A value of less than -1 will cause
     * the job to fail.
     *
     * \a size  the entry size limit, in octets, or -1 for no limit
     */
    void setMaximumSize(qint64 size);

    /*!
     * Sets whether to retrieve children or descendants of the requested entries.
     *
     * Metadata entry names are hierarchical, much like UNIX path names.
     * It therefore makes sense to ask for an entry and all its children
     * (OneLevel) or an entry and all its descendants (AllLevels).
     *
     * For example, /shared/foo/bar/baz is a child of /shared/foo/bar and a
     * descendent of /shared/foo.  So if you request the entry "/shared/foo"
     * with depth NoDepth, you will only get the "/shared/foo" entry.  If
     * you set the depth to OneLevel, you will also get "/shared/foo/bar".
     * If you set the depth to AllLevels, you will also get
     * "/shared/foo/bar/baz", and every other metadata entry that starts
     * with "/shared/foo/".
     *
     * Note that this is only used when the server capability mode is
     * Metadata.
     *
     * \a depth  the depth of the metadata tree to return
     */
    void setDepth(Depth depth);

    /*!
     * Get a single metadata entry.
     *
     * The metadata must have been requested using addEntry(), and
     * the job must have completed successfully, or this method
     * will not return anything.
     *
     * Note that if setMaximumSize() was used to limit the size of
     * returned metadata, this method may return an empty QByteArray
     * even if the metadata entry was requested and exists on the
     * server.  This will happen when the metadata entry is larger
     * than the size limit given to setMaximumSize().
     *
     * Note that this expects METADATA style entries (with a /shared or /private prefix typically).
     * In ANNOTATEMORE mode, this prefix is automatically replaced with an appropriate attribute.
     *
     * \a entry the entry to get
     * Returns  the metadata entry value
     */
    [[nodiscard]] QByteArray metaData(const QByteArray &entry) const;

    /*!
     * Get all the metadata for a given mailbox.
     *
     * The returned map is from metadata entry names to attributes or values.
     *
     * If operating in Metadata mode, the metadata value is stored against the
     * empty QByteArray:
     * \de
     * map = job.allMetaData( "INBOX" );
     * QByteArray value = map[ "/shared/comment" ].value( QByteArray() );
     * \endcode
     *
     * The equivalent in Annotatemore mode would be:
     * \de
     * map = job.allMetaData( "INBOX" );
     * QByteArray value = map[ "/comment" ].value( "value.shared" );
     * \endcode
     *
     * \a mailBox  a mailbox name or an empty string for server metadata
     * Returns  a map from metadata entry names to attributes or values
     */
    // XXX: what's with the mailBox argument in a class that has setMailBox()?
    //      KJobs are not intended to be run more than once
    [[nodiscard]] QMap<QByteArray, QMap<QByteArray, QByteArray>> allMetaData(const QString &mailBox) const;

    /*!
     * Get all the metadata for the mailbox set with setMailBox().
     *
     * Note that the returned map uses METADATA style entries (with a /shared or /private prefix typically),
     * also in ANNOTATEMORE mode.
     *
     * Returns a map from metadata entry names to values
     */
    [[nodiscard]] QMap<QByteArray, QByteArray> allMetaData() const;

    /*!
     * Get all the metadata for the mailbox.
     *
     * Note that the returned map uses METADATA style entries (with a /shared or /private prefix typically),
     * also in ANNOTATEMORE mode.
     *
     * Returns a map from metadata entry names to values
     */
    [[nodiscard]] QMap<QByteArray, QByteArray> allMetaDataForMailbox(const QString &mailbox) const;

    /*!
     * Get all the metadata for for all mailboxes.
     *
     * Note that the returned map uses METADATA style entries (with a /shared or /private prefix typically),
     * also in ANNOTATEMORE mode.
     *
     * Returns a map in the form (mailbox, (entry, value))
     */
    [[nodiscard]] QHash<QString, QMap<QByteArray, QByteArray>> allMetaDataForMailboxes() const;

protected:
    void doStart() override;
    void handleResponse(const Response &response) override;
};

}
