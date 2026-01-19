/*
    SPDX-FileCopyrightText: 2009 Andras Mantia <amantia@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "acljobbase.h"
#include "job_p.h"
#include "session.h"

namespace KIMAP
{
class AclJobBasePrivate : public JobPrivate
{
public:
    AclJobBasePrivate(Session *session, const QString &name)
        : JobPrivate(session, name)
    {
    }

    void setIdentifier(const QByteArray &identifier);
    [[nodiscard]] QByteArray identifier() const;

    [[nodiscard]] bool hasRightEnabled(Acl::Right right) const;

    void setRights(const QByteArray &rights);
    void setRights(AclJobBase::AclModifier modifier, Acl::Rights rights);

    QString mailBox;
    QByteArray id;
    Acl::Rights rightList = Acl::None;
    AclJobBase::AclModifier modifier = AclJobBase::Change;
};
}
