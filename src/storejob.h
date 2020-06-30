/*
    SPDX-FileCopyrightText: 2009 Kevin Ottens <ervin@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KIMAP_STOREJOB_H
#define KIMAP_STOREJOB_H

#include "kimap_export.h"

#include "job.h"
#include "imapset.h"

namespace KIMAP
{

class Session;
struct Response;
class StoreJobPrivate;

typedef QList<QByteArray> MessageFlags;

class KIMAP_EXPORT StoreJob : public Job
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(StoreJob)

    friend class SessionPrivate;

public:
    enum StoreMode {
        SetFlags,
        AppendFlags,
        RemoveFlags
    };

    explicit StoreJob(Session *session);
    ~StoreJob() override;

    void setSequenceSet(const ImapSet &set);
    Q_REQUIRED_RESULT ImapSet sequenceSet() const;

    void setUidBased(bool uidBased);
    Q_REQUIRED_RESULT bool isUidBased() const;

    void setFlags(const MessageFlags &flags);
    Q_REQUIRED_RESULT MessageFlags flags() const;

    void setGMLabels(const MessageFlags &gmLabels);
    Q_REQUIRED_RESULT MessageFlags gmLabels() const;

    void setMode(StoreMode mode);
    Q_REQUIRED_RESULT StoreMode mode() const;

    Q_REQUIRED_RESULT QMap<int, MessageFlags> resultingFlags() const;

protected:
    void doStart() override;
    void handleResponse(const Response &response) override;
};

}

#endif
