/*
    SPDX-FileCopyrightText: 2009 Andras Mantia <amantia@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "kimap_export.h"

#include "job.h"

namespace KIMAP
{
class Session;
struct Response;
class MetaDataJobBasePrivate;

/**
 * Base class for jobs that operate on mailbox metadata
 *
 * Provides support for the IMAP METADATA extension; both the
 * final RFC version
 * (<a href="https://tools.ietf.org/html/rfc5464">RFC 5464</a>)
 * and the older, incompatible draft version (known as ANNOTATEMORE)
 * (<a
 * href="https://tools.ietf.org/html/draft-daboo-imap-annotatemore-07"
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
    ~MetaDataJobBase() override;

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
    Q_REQUIRED_RESULT QString mailBox() const;

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
    void setServerCapability(ServerCapability capability);
    /**
     * The version of the metadata extension that will be used.
     */
    Q_REQUIRED_RESULT ServerCapability serverCapability() const;

protected:
    MetaDataJobBase(JobPrivate &dd);
};

}

