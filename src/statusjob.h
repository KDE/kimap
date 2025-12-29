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
/*!
 * \class KIMAP::StatusJob
 * \inmodule KIMAP
 * \inheaderfile KIMAP/StatusJob
 */
class KIMAP_EXPORT StatusJob : public Job
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(StatusJob)

    friend class StatusJobPrivate;

public:
    explicit StatusJob(Session *session);
    ~StatusJob() override;

    /*!
     * \brief setMailBox
     * \param mailBox
     */
    void setMailBox(const QString &mailBox);
    /*!
     * \brief mailBox
     * \return
     */
    [[nodiscard]] QString mailBox() const;

    /*!
     * \brief setDataItems
     * \param dataItems
     */
    void setDataItems(const QList<QByteArray> &dataItems);

    /*!
     * \brief dataItems
     * \return
     */
    [[nodiscard]] QList<QByteArray> dataItems() const;

    /*!
     * \brief status
     * \return
     */
    [[nodiscard]] QList<QPair<QByteArray, qint64>> status() const;

protected:
    void doStart() override;
    void handleResponse(const Response &response) override;
};

}
