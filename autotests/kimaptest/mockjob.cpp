/*
    This file is part of the KDE project
    Copyright (C) 2008 Kevin Ottens <ervin@kde.org>

    Copyright (c) 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
    Author: Kevin Ottens <kevin@kdab.com>

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

#include "mockjob.h"

#include "job_p.h"
#include "session.h"
#include "session_p.h"
#include <KLocalizedString>
#include <QtCore/QTimer>

class MockJobPrivate : public KIMAP::JobPrivate
{
public:
    MockJobPrivate(KIMAP::Session *session, const QString &name)
        : KIMAP::JobPrivate(session, name),
          timeout(10)
    { }

    ~MockJobPrivate() { }

    QByteArray command;
    int timeout;
};

MockJob::MockJob(KIMAP::Session *session)
    : KIMAP::Job(*new MockJobPrivate(session, i18n("Mock")))
{
}

void MockJob::doStart()
{
    Q_D(MockJob);
    if (isNull()) {
        QTimer::singleShot(d->timeout, this, SLOT(done()));
    } else {
        d->sessionInternal()->setSocketTimeout(d->timeout);
        d->tags << d->sessionInternal()->sendCommand(d->command);
    }
}

void MockJob::done()
{
    emitResult();
}

void MockJob::setCommand(const QByteArray &command)
{
    Q_D(MockJob);
    d->command = command;
}

QByteArray MockJob::command() const
{
    Q_D(const MockJob);
    return d->command;
}

void MockJob::setTimeout(int timeout)
{
    Q_D(MockJob);
    d->timeout = timeout;
}

int MockJob::timeout() const
{
    Q_D(const MockJob);
    return d->timeout;
}

bool MockJob::isNull() const
{
    Q_D(const MockJob);
    return d->command.isEmpty();
}
