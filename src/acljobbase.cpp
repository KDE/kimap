/*
    SPDX-FileCopyrightText: 2009 Andras Mantia <amantia@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "acljobbase.h"
#include "acljobbase_p.h"
#include "response_p.h"
#include "session_p.h"

#include <KLocalizedString>

using namespace KIMAP;

void AclJobBasePrivate::setIdentifier(const QByteArray &identifier)
{
    id = identifier;
}

QByteArray AclJobBasePrivate::identifier() const
{
    return id;
}

bool AclJobBasePrivate::hasRightEnabled(Acl::Right right) const
{
    return rightList & right;
}

void AclJobBasePrivate::setRights(const QByteArray &rights)
{
    switch (rights[0]) {
    case '+':
        modifier = AclJobBase::Add;
        break;
    case '-':
        modifier = AclJobBase::Remove;
        break;
    default:
        modifier = AclJobBase::Change;
        break;
    }

    rightList = Acl::rightsFromString(rights);
}

void AclJobBasePrivate::setRights(AclJobBase::AclModifier _modifier, Acl::Rights rights)
{
    modifier = _modifier;
    // XXX: [alexmerry, 2010-07-24]: this is REALLY unintuitive behaviour
    rightList |= rights;
}

AclJobBase::AclJobBase(Session *session)
    : Job(*new AclJobBasePrivate(session, i18n("AclJobBase")))
{
}

AclJobBase::AclJobBase(JobPrivate &dd)
    : Job(dd)
{
}

AclJobBase::~AclJobBase()
{
}

void AclJobBase::setMailBox(const QString &mailBox)
{
    Q_D(AclJobBase);
    d->mailBox = mailBox;
}

QString AclJobBase::mailBox() const
{
    Q_D(const AclJobBase);
    return d->mailBox;
}
