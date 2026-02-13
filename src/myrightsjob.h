/*
    SPDX-FileCopyrightText: 2009 Andras Mantia <amantia@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "kimap_export.h"

#include "acljobbase.h"

namespace KIMAP
{
class Session;
struct Response;
class MyRightsJobPrivate;

/*!
 * \class KIMAP::MyRightsJob
 * \inmodule KIMAP
 * \inheaderfile KIMAP/MyRightsJob
 *
 * \brief Determine the rights the currently-logged-in user
 * has on the current mailbox.
 *
 * This should take into account the full access control
 * list.
 *
 * This job can only be run when the session is in the
 * authenticated (or selected) state.
 *
 * The current user must have one of the following rights
 * on the mailbox for this job to succeed:
 * \list
 * \li Acl::Lookup
 * \li Acl::Read
 * \li Acl::Insert
 * \li Acl::CreateMailbox
 * \li Acl::DeleteMailbox
 * \li Acl::Admin
 * \endlist
 *
 * This job requires that the server supports the ACL
 * capability, defined in
 * \l{https://tools.ietf.org/html/rfc4314}{RFC 4314}.
 */
class KIMAP_EXPORT MyRightsJob : public AclJobBase
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(MyRightsJob)

    friend class SessionPrivate;

public:
    /*!
     *
     */
    explicit MyRightsJob(Session *session);
    ~MyRightsJob() override;

    /*!
     * Check whether the current user has the a particular right
     * on the mailbox.
     *
     * The result of this method is undefined if the job has
     * not yet completed.
     *
     * \a right the right to check for
     */
    [[nodiscard]] bool hasRightEnabled(Acl::Right right);
    /*!
     * Get the rights for the current user on the mailbox.
     *
     * The result of this method is undefined if the job has
     * not yet completed.
     */
    [[nodiscard]] Acl::Rights rights();

protected:
    void doStart() override;
    void handleResponse(const Response &response) override;
};

}
