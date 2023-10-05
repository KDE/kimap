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
    [[nodiscard]] QString mailBox() const;

    void setDataItems(const QList<QByteArray> &dataItems);
    [[nodiscard]] QList<QByteArray> dataItems() const;

    [[nodiscard]] QList<QPair<QByteArray, qint64>> status() const;

protected:
    void doStart() override;
    void handleResponse(const Response &response) override;
};

}
