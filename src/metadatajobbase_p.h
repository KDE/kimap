/*
    Copyright (c) 2009 Andras Mantia <amantia@kde.org>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

#ifndef KIMAP_METADATAJOBBASE_P_H
#define KIMAP_METADATAJOBBASE_P_H

#include "metadatajobbase.h"
#include "job_p.h"
#include "message_p.h"
#include "session.h"

namespace KIMAP
{
class MetaDataJobBasePrivate : public JobPrivate
{
public:
    MetaDataJobBasePrivate(Session *session, const QString &name) : JobPrivate(session, name), serverCapability(MetaDataJobBase::Metadata)
    {}

    ~MetaDataJobBasePrivate() { }

    QByteArray addPrefix(const QByteArray &entry, const QByteArray &attribute) const;
    QByteArray removePrefix(const QByteArray &) const;

    QByteArray getAttribute(const QByteArray &entry) const;

    MetaDataJobBase::ServerCapability serverCapability;
    QString mailBox;
};
}

#endif
