/*
    SPDX-FileCopyrightText: 2020 Daniel Vrátil <dvratil@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "kimap_export.h"

#include "job.h"

#include <QStringList>

namespace KIMAP
{
class Session;
struct Response;
class EnableJobPrivate;

/**
 * Job to enable additional IMAP capabilities.
 *
 * Requires server to implement the IMAP ENABLE Extension (RFC5161). The
 * new capabilities to enable will be specified by the user. The user is
 * responsible for making sure the capabilities are supported by the server.
 *
 * The example usecase for this job is to enable support for the QRESYNC
 * extension (RFC5162) on the server.
 *
 * @since 5.16
 */
class KIMAP_EXPORT EnableJob : public Job
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(EnableJob)

    friend class SessionPrivate;

public:
    explicit EnableJob(Session *session);
    ~EnableJob() override;

    /**
     * List of server capabilities to enable.
     */
    void setCapabilities(const QStringList &capabilities);

    /**
     * List of capabilities that were successfully enabled on the server.
     */
    Q_REQUIRED_RESULT QStringList enabledCapabilities() const;

protected:
    void doStart() override;
    void handleResponse(const Response &response) override;
};

}

