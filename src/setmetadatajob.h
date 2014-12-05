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

#ifndef KIMAP_SETMETADATAJOB_H
#define KIMAP_SETMETADATAJOB_H

#include "kimap_export.h"

#include "metadatajobbase.h"

namespace KIMAP
{

class Session;
struct Message;
class SetMetaDataJobPrivate;

/**
 * Sets mailbox metadata.
 *
 * Provides support for the IMAP METADATA extension; both the
 * final RFC version
 * (<a href="http://tools.ietf.org/html/rfc5464">RFC 5464</a>)
 * and the older, incompatible draft version (known as ANNOTATEMORE)
 * (<a
 * href="http://tools.ietf.org/html/draft-daboo-imap-annotatemore-07"
 * >draft-daboo-imap-annotatemore-07</a>).  See setServerCompatibility().
 *
 * Note that in Annotatemore mode, this job can only operate on
 * one metadata entry at once.
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
    explicit SetMetaDataJob(Session *session);
    virtual ~SetMetaDataJob();

    /**
     * Adds a metadata entry or attribute to the list of modifications to make
     *
     * When in Metadata mode, this method adds a metadata
     * entry to the list of metadata additions and updates that
     * will be performed when the job is run.
     *
     * @p name must be a valid ASCII string and may not contain two
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
     * - /shared/comment
     * - /shared/admin - a URI for contacting the server administrator
     *                   (eg: a mailto: or tel: URI)
     * - /shared/vendor/<vendor-token>/something
     * - /private/vendor/<vendor-token>/something
     *
     * Mailbox metadata entry names include:
     * - /shared/comment
     * - /private/comment
     * - /shared/vendor/<vendor-token>/something
     * - /private/vendor/<vendor-token>/something
     *
     * @p value can be any data, although if it is a multi-line string
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
     * @p name must be a valid UTF-8 string, and may not contain the
     * '%' or '*' characters, or NUL.  Use of non-visible UTF-8 characters
     * is strongly discouraged.
     *
     * Possible attribute name prefixes are:
     * - value - the data value of the attribute
     * - content-type - a MIME content type and subtype
     * - content-language - a RFC 3282 language code
     * - vendor.<vendor-token> - a vendor-specific attribute
     *
     * Attribute names an attribute name prefix followed by ".priv" for
     * private attributes or ".shared" for shared attributes.  Note that
     * the attributes "size.priv" and "size.shared" are read-only
     * attributes set by the server, and so cannot be used with
     * SetMetaDataJob.
     *
     * @param name   the metadata entry name (Metadata or Annotatemore mode) in ASCII or
     *               attribute name (Annotatemore mode, if used without /shared or /private prefix) in UTF-8
     * @param value  the value of the entry or attribute
     */
    // KDE5: drop ANNOTATEMORE support
    void addMetaData(const QByteArray &name, const QByteArray &value);

    /**
     * Sets the metadata entry name to operate on (in Annotatemore mode)
     *
     * In Annotatemore mode, this specifies the metadata entry name to
     * operate on.  For server metadata, this is one of:
     * - /comment
     * - /motd
     * - /admin
     * - /vendor/<vendor-token>/something
     *
     * For mailbox metadata, this is one of:
     * - /comment
     * - /sort
     * - /thread
     * - /check
     * - /checkperiod
     * - /vendor/<vendor-token>/something
     *
     * Entry names must be valid UTF-8 strings that do not contain the
     * '%' or '*' characters, or NUL.  Use of non-visible UTF-8
     * characters is strongly discouraged.
     *
     * In Metadata mode, this has no effect.  Metadata entry names
     * should instead be specified as the first argument to addMetaData().
     *
     * @see setServerCapability()
     *
     * @param entry  the metadata entry name in UTF-8
     *
     * @deprecated Use a /shared or /private prefix with addMetaData instead.
     */
    // KDE5: remove
    KIMAP_DEPRECATED void setEntry(const QByteArray &entry);

    /**
     * Possible error codes that may be returned by the server.
     */
    enum MetaDataError {
        NoError = 0,  /**< Used to indicate that no errors have been received */
        TooMany = 1,  /**< Cannot add a new metadata item, because the limit has already been reached */
        TooBig = 2,   /**< A metadata value was too big (see maxAcceptedSize()) */
        NoPrivate = 4 /**< The server does not support private metadata entries */
    };

    // Q_DECLARE_WHATEVER_THAT_WAS missing
    Q_DECLARE_FLAGS(MetaDataErrors, MetaDataError)

    /**
     * The metadata errors received from the server.
     *
     * @return  a set of error codes
     */
    MetaDataErrors metaDataErrors() const;
    /**
     * The maximum accepted metadata size.
     *
     * If the server replied that one of the metadata values was too
     * large (see metaDataErrors), this should indicate what the
     * maximum size accepted by the server is.
     *
     * @return  the maximum value size in octets, or -1 if the limit is unknown
     */
    qint64 maxAcceptedSize();

protected:
    virtual void doStart() Q_DECL_OVERRIDE;
    virtual void handleResponse(const Message &response) Q_DECL_OVERRIDE;

};

}

Q_DECLARE_OPERATORS_FOR_FLAGS(KIMAP::SetMetaDataJob::MetaDataErrors)

#endif
