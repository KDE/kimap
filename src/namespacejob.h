/*
    SPDX-FileCopyrightText: 2009 Kevin Ottens <ervin@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "kimap_export.h"

#include "job.h"

namespace KIMAP
{
class Session;
struct Response;
struct MailBoxDescriptor;
class NamespaceJobPrivate;

class KIMAP_EXPORT NamespaceJob : public Job
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(NamespaceJob)

    friend class SessionPrivate;

public:
    NamespaceJob(Session *session);
    ~NamespaceJob() override;

    Q_REQUIRED_RESULT QList<MailBoxDescriptor> personalNamespaces() const;
    Q_REQUIRED_RESULT QList<MailBoxDescriptor> userNamespaces() const;
    Q_REQUIRED_RESULT QList<MailBoxDescriptor> sharedNamespaces() const;

    bool containsEmptyNamespace() const;

protected:
    void doStart() override;
    void handleResponse(const Response &response) override;
};

}

