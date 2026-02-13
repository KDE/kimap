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
/*!
 * \class KIMAP::NamespaceJob
 * \inmodule KIMAP
 * \inheaderfile KIMAP/NamespaceJob
 *
 * \brief The NamespaceJob class
 */
class KIMAP_EXPORT NamespaceJob : public Job
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(NamespaceJob)

    friend class SessionPrivate;

public:
    /*!
     */
    explicit NamespaceJob(Session *session);
    /*!
     */
    ~NamespaceJob() override;

    /*!
     * \brief personalNamespaces
     * \return
     */
    [[nodiscard]] QList<MailBoxDescriptor> personalNamespaces() const;
    /*!
     * \brief userNamespaces
     * \return
     */
    [[nodiscard]] QList<MailBoxDescriptor> userNamespaces() const;
    /*!
     * \brief sharedNamespaces
     * \return
     */
    [[nodiscard]] QList<MailBoxDescriptor> sharedNamespaces() const;

    /*!
     * \brief containsEmptyNamespace
     * \return
     */
    [[nodiscard]] bool containsEmptyNamespace() const;

protected:
    /*!
     */
    void doStart() override;
    /*!
     */
    void handleResponse(const Response &response) override;
};

}
