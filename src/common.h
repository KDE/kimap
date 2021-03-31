/*  This file is part of the KDE project
    SPDX-FileCopyrightText: 2008 Jarosław Staniek <staniek@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <stdio.h>

extern "C" {
#include <sasl/sasl.h>
}

inline bool initSASL()
{
#ifdef Q_OS_WIN // krazy:exclude=cpp
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

