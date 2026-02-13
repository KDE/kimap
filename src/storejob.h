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
     *
     */
    explicit StoreJob(Session *session);
    ~StoreJob() override;

    /*!
     *
     */
    void setSequenceSet(const ImapSet &set);

    /*!
     *
     */
    [[nodiscard]] ImapSet sequenceSet() const;

    /*!
     *
     */
    void setUidBased(bool uidBased);

    /*!
     *
     */
    [[nodiscard]] bool isUidBased() const;

    /*!
     *
     */
    void setFlags(const MessageFlags &flags);

    /*!
     *
     */
    [[nodiscard]] MessageFlags flags() const;

    /*!
     *
     */
    void setGMLabels(const MessageFlags &gmLabels);

    /*!
     *
     */
    [[nodiscard]] MessageFlags gmLabels() const;

    /*!
     *
     */
    void setMode(StoreMode mode);

    /*!
     *
     */
    [[nodiscard]] StoreMode mode() const;

    /*!
     *
     */
    [[nodiscard]] QMap<qint64, MessageFlags> resultingFlags() const;

protected:
    void doStart() override;
    void handleResponse(const Response &response) override;
};

}
