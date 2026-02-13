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
class SetMetaDataJobPrivate;

/*!
 * \class KIMAP::SetMetaDataJob
 * \inmodule KIMAP
 * \inheaderfile KIMAP/SetMetaDataJob
 *
 * \brief Sets mailbox metadata.
 *
 * Provides support for the IMAP METADATA extension; both the
 * final RFC version
 * (\l{https://tools.ietf.org/html/rfc5464}{RFC 5464})
 * and the older, incompatible draft version (known as ANNOTATEMORE)
 * (\l{https://tools.ietf.org/html/draft-daboo-imap-annotatemore-07}{draft-daboo-imap-annotatemore-07}).  See setServerCompatibility().
 *
 * Note that in Annotatemore mode, this job can only operate on
 * one metadata entry at once.
 *
 * This job can only be run when the session is in the
 * authenticated (or selected) state.
 *
 * If the server supports ACLs, the user will need the
 * Acl::Lookup right on the mailbox, as well as one of
 * \list
 * \li Acl::Read
 * \li Acl::KeepSeen
 * \li Acl::Write
 * \li Acl::Insert
 * \li Acl::Post
 * \endlist
 *
 * Otherwise, the user must be able to list the mailbox
 * and either read or write the message content.
 *
 * Note that even if the user has these permissions, the
 * server may refuse to allow the user to write metadata
 * based on some other criteria.
 *
 * Note also that on servers that implement the Annotatemore
 * version of the extension, only Acl::Lookup rights are
 * required (ie: the user must be able to list the mailbox).
 */
class KIMAP_EXPORT SetMetaDataJob : public MetaDataJobBase
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(SetMetaDataJob)

    friend class SessionPrivate;

public:
    /*!
     *
     */
    explicit SetMetaDataJob(Session *session);
    ~SetMetaDataJob() override;

    // KDE5: drop ANNOTATEMORE support
    /*!
     * Adds a metadata entry or attribute to the list of modifications to make
     *
     * When in Metadata mode, this method adds a metadata
     * entry to the list of metadata additions and updates that
     * will be performed when the job is run.
     *
     * \a name must be a valid ASCII string and may not contain two
     * consecutive forward slashes ('/'), must not end with '/' and
     * must not contain '*', '%', non-ASCII characters or characters
     * in the ASCII range 0x00 to 0x19 (in practice, all control
     * characters should be avoided).  The name is case-insensitive.
     *
     * The first part of the entry name should be "/private" or
     * "/shared", indicating the scope of the entry.  Note that
     * private metadata may not be supported by all servers.
     *
     * Server metadata entry names include:
     * \list
     * \li /shared/comment
     * \li /shared/admin - a URI for contacting the server administrator
     *                   (eg: a mailto: or tel: URI)
     * \li /shared/vendor/<vendor-token>/something
     * \li /private/vendor/<vendor-token>/something
     * \endlist
     *
     * Mailbox metadata entry names include:
     * \list
     * \li /shared/comment
     * \li /private/comment
     * \li /shared/vendor/<vendor-token>/something
     * \li /private/vendor/<vendor-token>/something
     * \endlist
     *
     * \a value can be any data, although if it is a multi-line string
     * value, CRLF line-endings must be used.
     *
     * In Annotatemore mode it is possible to prefix the entry name with a /shared or /private prefix, that is automatically translated
     * to an appropriate value.shared|priv attribute.
     *
     * Annotatemore legacy mode:
     * When in Annotatemore mode, this method adds an attribute
     * entry to the list of additions and updates that will be
     * performed on the metadata entry when the job is run.
     *
     * \a name must be a valid UTF-8 string, and may not contain the
     * '%' or '*' characters, or NUL.  Use of non-visible UTF-8 characters
     * is strongly discouraged.
     *
     * Possible attribute name prefixes are:
     * \list
     * \li value - the data value of the attribute
     * \li content-type - a MIME content type and subtype
     * \li content-language - a RFC 3282 language code
     * \li vendor.<vendor-token> - a vendor-specific attribute
     * \endlist
     *
     * Attribute names an attribute name prefix followed by ".priv" for
     * private attributes or ".shared" for shared attributes.  Note that
     * the attributes "size.priv" and "size.shared" are read-only
     * attributes set by the server, and so cannot be used with
     * SetMetaDataJob.
     *
     * \a name the metadata entry name (Metadata or Annotatemore mode) in ASCII or
     *               attribute name (Annotatemore mode, if used without /shared or /private prefix) in UTF-8
     * \a value the value of the entry or attribute
     */
    void addMetaData(const QByteArray &name, const QByteArray &value);

    /*!
     * Possible error codes that may be returned by the server.
     *
     * \value NoError Used to indicate that no errors have been received
     * \value TooMany Cannot add a new metadata item, because the limit has already been reached
     * \value TooBig A metadata value was too big (see maxAcceptedSize())
     * \value NoPrivate The server does not support private metadata entries
     */
    enum MetaDataError {
        NoError = 0,
        TooMany = 1,
        TooBig = 2,
        NoPrivate = 4
    };

    // Q_DECLARE_WHATEVER_THAT_WAS missing
    Q_DECLARE_FLAGS(MetaDataErrors, MetaDataError)

    /*!
     * The metadata errors received from the server.
     *
     * Returns a set of error codes
     */
    [[nodiscard]] MetaDataErrors metaDataErrors() const;
    /*!
     * The maximum accepted metadata size.
     *
     * If the server replied that one of the metadata values was too
     * large (see metaDataErrors), this should indicate what the
     * maximum size accepted by the server is.
     *
     * Returns the maximum value size in octets, or -1 if the limit is unknown
     */
    [[nodiscard]] qint64 maxAcceptedSize();

protected:
    void doStart() override;
    void handleResponse(const Response &response) override;
};

}

Q_DECLARE_OPERATORS_FOR_FLAGS(KIMAP::SetMetaDataJob::MetaDataErrors)
