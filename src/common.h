/*  This file is part of the KDE project
    Copyright (C) 2008 Jarosław Staniek <staniek@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef _KIOSLAVE_COMMON_H
#define _KIOSLAVE_COMMON_H

#include <stdio.h>
#include <QFile>
#include <QDir>
#include <QCoreApplication>

extern "C" {
#include <sasl/sasl.h>
}

inline bool initSASL()
{
#ifdef Q_OS_WIN  //krazy:exclude=cpp
    for (const auto &path : QCoreApplication::libraryPaths()) {
        QDir dir(path);
        if (dir.exists(QStringLiteral("sasl2"))) {
            auto libInstallPath = QFile::encodeName(dir.absoluteFilePath(QStringLiteral("sasl2")));
            if (sasl_set_path(SASL_PATH_TYPE_PLUGIN, libInstallPath.data()) != SASL_OK) {
                fprintf(stderr, "SASL path initialization failed!\n");
                return false;
            }
            break;
        }
    }
#endif

    if (sasl_client_init(nullptr) != SASL_OK) {
        fprintf(stderr, "SASL library initialization failed!\n");
        return false;
    }
    return true;
}

#endif
