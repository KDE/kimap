/*
    This file is part of the kimap library.

    SPDX-FileCopyrightText: 2007 Allen Winter <winter@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <QObject>

class RFCCodecsTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testIMAPEncoding();
    void testQuotes();
};

