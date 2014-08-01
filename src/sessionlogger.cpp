/*
    Copyright (c) 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
    Author: Kevin Ottens <kevin@kdab.com>

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

#include "sessionlogger_p.h"

#include <QDebug>

#include <unistd.h>

using namespace KIMAP;

SessionLogger::SessionLogger()
    : m_id(0)
{
    static qint64 nextId = 0;
    m_id = ++nextId;

    m_file.setFileName(QLatin1String(qgetenv("KIMAP_LOGFILE"))
                       + QLatin1Char('.') + QString::number(getpid())
                       + QLatin1Char('.') + QString::number(m_id));
    if (!m_file.open(QFile::WriteOnly)) {
        qDebug() << " m_file can be open in write only";
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
