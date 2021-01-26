/*
    SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
    SPDX-FileContributor: Kevin Ottens <kevin@kdab.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "sessionlogger_p.h"

#include "kimap_debug.h"
#include <QCoreApplication>

using namespace KIMAP;

SessionLogger::SessionLogger()
{
    static qint64 nextId = 0;
    m_id = ++nextId;

    m_file.setFileName(QLatin1String(qgetenv("KIMAP_LOGFILE")) + QLatin1Char('.') + QString::number(QCoreApplication::applicationPid()) + QLatin1Char('.')
                       + QString::number(m_id));
    if (!m_file.open(QFile::WriteOnly)) {
        qCWarning(KIMAP_LOG) << "Could not open log file for writing:" << m_file.fileName();
    }
}

SessionLogger::~SessionLogger()
{
    m_file.close();
}

void SessionLogger::dataSent(const QByteArray &data)
{
    m_file.write("C: " + data.trimmed() + '\n');
    m_file.flush();
}

void SessionLogger::dataReceived(const QByteArray &data)
{
    m_file.write("S: " + data.trimmed() + '\n');
    m_file.flush();
}

void SessionLogger::disconnectionOccured()
{
    m_file.write("X\n");
}
