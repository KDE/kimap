/*
    Copyright (c) 2016 Daniel Vrátil <dvratil@kde.org>

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

#ifndef KIMAP_STATUSJOB_H
#define KIMAP_STATUSJOB_H

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

#endif // KIMAP_STATUSJOB_H
