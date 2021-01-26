/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2008 Kevin Ottens <ervin@kde.org>

    SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
    SPDX-FileContributor: Kevin Ottens <kevin@kdab.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
  */

#include "mockjob.h"

#include "job_p.h"
#include "session.h"
#include "session_p.h"
#include <KLocalizedString>
#include <QTimer>

class MockJobPrivate : public KIMAP::JobPrivate
{
public:
    MockJobPrivate(KIMAP::Session *session, const QString &name)
        : KIMAP::JobPrivate(session, name)
        , timeout(10)
    {
    }

    ~MockJobPrivate()
    {
    }

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
