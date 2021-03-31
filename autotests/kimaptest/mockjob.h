/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2008 Kevin Ottens <ervin@kde.org>

    SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
    SPDX-FileContributor: Kevin Ottens <kevin@kdab.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
  */

#pragma once

#include <kimap/job.h>

class MockJobPrivate;

/**
 * Provides an easy way to send an arbitrary IMAP client command.
 */
class MockJob : public KIMAP::Job
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(MockJob)

public:
    MockJob(KIMAP::Session *session);

    /**
     * Sets the command to execute.
     *
     * This should not include the command tag.
     */
    void setCommand(const QByteArray &command);
    /**
     * The command that will be sent.
     */
    QByteArray command() const;
    /**
     * Sets the timeout before the job completes.
     */
    void setTimeout(int timeout);
    /**
     * The timeout used by the job.
     */
    int timeout() const;
    /**
     * Whether the command is empty.
     *
     * Equivalent to command().isEmpty().
     *
     * @return @c true if no command is set, @c false otherwise
     */
    bool isNull() const;

    /**
     * Starts the job.
     *
     * Do not call this directly.  Use start() instead.
     *
     * @reimp
     */
    void doStart() override;

private Q_SLOTS:
    void done();
};

