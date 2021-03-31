/*
    SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
    SPDX-FileContributor: Kevin Ottens <kevin@kdab.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <QFile>

namespace KIMAP
{
class SessionLoggerPrivate;

class SessionLogger
{
public:
    SessionLogger();
    ~SessionLogger();

    void dataSent(const QByteArray &data);
    void dataReceived(const QByteArray &data);
    void disconnectionOccured();

private:
    Q_DISABLE_COPY(SessionLogger)
    qint64 m_id = 0;
    QFile m_file;
};

}

