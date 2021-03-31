/*
    SPDX-FileCopyrightText: 2016 Daniel Vrátil <dvratil@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "kimap_export.h"

#include "job.h"
#include <QList>

namespace KIMAP
{
class Session;
class StatusJobPrivate;

class KIMAP_EXPORT StatusJob : public Job
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(StatusJob)

    friend class StatusJobPrivate;

public:
    explicit StatusJob(Session *session);
    ~StatusJob() override;

    void setMailBox(const QString &mailBox);
    Q_REQUIRED_RESULT QString mailBox() const;

    void setDataItems(const QList<QByteArray> &dataItems);
    Q_REQUIRED_RESULT QList<QByteArray> dataItems() const;

    Q_REQUIRED_RESULT QList<QPair<QByteArray, qint64>> status() const;

protected:
    void doStart() override;
    void handleResponse(const Response &response) override;
};

}

