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

#ifndef KIMAP_METADATAJOBBASE_H
#define KIMAP_METADATAJOBBASE_H

#include "kimap_export.h"

#include "job.h"

namespace KIMAP
{

class Session;
struct Message;
class MetaDataJobBasePrivate;

/**
 * Base class for jobs that operate on mailbox metadata
 *
 * Provides support for the IMAP METADATA extension; both the
 * final RFC version
 * (<a href="http://tools.ietf.org/html/rfc5464">RFC 5464</a>)
 * and the older, incompatible draft version (known as ANNOTATEMORE)
 * (<a
 * href="http://tools.ietf.org/html/draft-daboo-imap-annotatemore-07"
 * >draft-daboo-imap-annotatemore-07</a>).
 *
 * This class cannot be used directly, you must subclass it and reimplement
 * at least the doStart() method.
*/
class KIMAP_EXPORT MetaDataJobBase : public Job
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(MetaDataJobBase)

    friend class SessionPrivate;

public:
    explicit MetaDataJobBase(Session *session);
    virtual ~MetaDataJobBase();

    /**
     * Represents the capability level of the server.
     */
    enum ServerCapability {
        /**
         * Used to indicate that the server supports the RFC 5464 version
         * of the extension.
         *
         * This corresponds to the METADATA server capability.
         */
        Metadata = 0,
        /**
         * Used to indicate that the server supports the
         * draft-daboo-imap-annotatemore-07 version of the extension.
         *
         * This corresponds to the ANNOTATEMORE server capability.
         */
        Annotatemore
    };

    /**
     * Set the mailbox to act on
     *
     * This may be an empty string, in which case metadata for the
     * server (rather than a specific mailbox) will be retrieved.
     *
     * @param mailBox  the name of an existing mailbox, or an empty string
     */
    void setMailBox(const QString &mailBox);
    /**
     * The mailbox that will be acted upon.
     *
     * If this is an empty string, server metadata will be retrieved.
     *
     * @return  a mailbox name, or an empty string
     */
    QString mailBox() const;

    /**
     * Set what version of the metadata extension to be compatible with.
     *
     * This will determine the commands that will be sent to the server.
     *
     * The draft for the metadata extension changed in an incompatible
     * way between versions 7 and 8, and some servers support version 7.
     * It should be possible to check which version the server supports
     * using CapabilityJob: servers implementing
     * draft-daboo-imap-annotatemore-07 should advertise the
     * ANNOTATEMORE capability, whereas servers implementing the final
     * RFC 5464 should advertise the METADATA capability.
     *
     * The default mode is Metadata.
     *
     * @param capability  the version of the extension implemented by the server
     */
    void setServerCapability(const ServerCapability &capability);
    /**
     * The version of the metadata extension that will be used.
     */
    ServerCapability serverCapability() const;

protected:
    MetaDataJobBase(JobPrivate &dd);

};

}

#endif
