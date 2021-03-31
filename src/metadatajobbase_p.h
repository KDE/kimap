/*
    SPDX-FileCopyrightText: 2009 Andras Mantia <amantia@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "job_p.h"
#include "metadatajobbase.h"
#include "response_p.h"
#include "session.h"

namespace KIMAP
{
class MetaDataJobBasePrivate : public JobPrivate
{
public:
    MetaDataJobBasePrivate(Session *session, const QString &name)
        : JobPrivate(session, name)
        , serverCapability(MetaDataJobBase::Metadata)
    {
    }

    ~MetaDataJobBasePrivate()
    {
    }

    QByteArray addPrefix(const QByteArray &entry, const QByteArray &attribute) const;
    QByteArray removePrefix(const QByteArray &) const;

    QByteArray getAttribute(const QByteArray &entry) const;

    MetaDataJobBase::ServerCapability serverCapability;
    QString mailBox;
};
}

