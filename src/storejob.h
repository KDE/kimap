/*
    SPDX-FileCopyrightText: 2009 Kevin Ottens <ervin@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "kimap_export.h"

#include "imapset.h"
#include "job.h"

namespace KIMAP
{
class Session;
struct Response;
class StoreJobPrivate;

/*!
 * \typealias KIMAP::MessageFlags
 * \relates KIMAP::StoreJob
 */
using MessageFlags = QList<QByteArray>;
/*!
 * \class KIMAP::StoreJob
 * \inmodule KIMAP
 * \inheaderfile KIMAP/StoreJob
 *
 * \brief Store Job.
 *
 * If flags contains \Recent, it won't be pushed to the server. This is to protect clients from mistake making them violate the RFC which mandates that clients
 * shouldn't fiddle with this flag.
 */
class KIMAP_EXPORT StoreJob : public Job
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(StoreJob)

    friend class SessionPrivate;

public:
    /*!
     * \value SetFlags
     * \value AppendFlags
     * \value RemoveFlags
     */
    enum StoreMode {
        SetFlags,
        AppendFlags,
        RemoveFlags
    };

    /*!
     */
    explicit StoreJob(Session *session);

    /*!
     */
    ~StoreJob() override;

    /*!
     */
    void setSequenceSet(const ImapSet &set);

    /*!
     */
    [[nodiscard]] ImapSet sequenceSet() const;

    /*!
     */
    void setUidBased(bool uidBased);

    /*!
     */
    [[nodiscard]] bool isUidBased() const;

    /*!
     */
    void setFlags(const MessageFlags &flags);

    /*!
     */
    [[nodiscard]] MessageFlags flags() const;

    /*!
     */
    void setGMLabels(const MessageFlags &gmLabels);

    /*!
     */
    [[nodiscard]] MessageFlags gmLabels() const;

    /*!
     */
    void setMode(StoreMode mode);

    /*!
     */
    [[nodiscard]] StoreMode mode() const;

    /*!
     */
    [[nodiscard]] QMap<qint64, MessageFlags> resultingFlags() const;

    /*!
     * Set CONDSTORE mod-seq parameter for UNCHANGEDSINCE modifier
     *
     * This can be set only when server
     * has CONDSTORE capability (RFC4551), otherwise the STORE command will
     * fail.
     *
     * \a lastModseq Last modification sequence number known to the client
     */
    void setLastModSeq(quint64 lastModSeq);

    /*!
    The list of updated MODSEQ for the modified messages

    Only use when CONDSTORE is enabled (when lastModSeq is set)
     */
    [[nodiscard]] QMap<qint64, qint64> resultingModSeqs() const;

    /*!
    The list of messages that have not been modified by the StoreJob
    Because their MODSEQ on the server is superior to the lastModSeq value set for this job

    Only use when CONDSTORE is enabled (when lastModSeq is set)
     */
    ImapSet unchangedMessages() const;

protected:
    /*!
     */
    void doStart() override;
    /*!
     */
    void handleResponse(const Response &response) override;
};

}
