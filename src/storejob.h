/*
    SPDX-FileCopyrightText: 2009 Kevin Ottens <ervin@kde.org>

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
