/*
    SPDX-FileCopyrightText: 2009 Andras Mantia <amantia@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "metadatajobbase.h"
#include "metadatajobbase_p.h"
#include "response_p.h"
#include "session_p.h"

#include <KLocalizedString>

using namespace KIMAP;

QByteArray MetaDataJobBasePrivate::addPrefix(const QByteArray &entry, const QByteArray &attribute) const
{
    if (serverCapability == MetaDataJobBase::Annotatemore) {
        if (attribute == "value.shared") {
            return QByteArray("/shared").append(entry);
        } else if (attribute == "value.priv") {
            return QByteArray("/private").append(entry);
        }
    }
    return entry;
}

QByteArray MetaDataJobBasePrivate::removePrefix(const QByteArray &entry) const
{
    if (serverCapability == MetaDataJobBase::Annotatemore) {
        if (entry.startsWith("/shared")) {
            return entry.mid(QByteArray("/shared").size());
        } else if (entry.startsWith("/private")) {
            return entry.mid(QByteArray("/private").size());
        }
    }
    return entry;
}

QByteArray MetaDataJobBasePrivate::getAttribute(const QByteArray &entry) const
{
    if (serverCapability == MetaDataJobBase::Annotatemore) {
        if (entry.startsWith("/shared")) {
            return QByteArray("value.shared");
        } else if (entry.startsWith("/private")) {
            return QByteArray("value.priv");
        }
    }
    return QByteArray();
}

MetaDataJobBase::MetaDataJobBase(Session *session)
    : Job(*new MetaDataJobBasePrivate(session, i18n("MetaDataJobBase")))
{
}

MetaDataJobBase::MetaDataJobBase(JobPrivate &dd)
    : Job(dd)
{
}

MetaDataJobBase::~MetaDataJobBase()
{
}

void MetaDataJobBase::setMailBox(const QString &mailBox)
{
    Q_D(MetaDataJobBase);
    d->mailBox = mailBox;
}

QString MetaDataJobBase::mailBox() const
{
    Q_D(const MetaDataJobBase);
    return d->mailBox;
}

void MetaDataJobBase::setServerCapability(ServerCapability capability)
{
    Q_D(MetaDataJobBase);
    d->serverCapability = capability;
}

MetaDataJobBase::ServerCapability MetaDataJobBase::serverCapability() const
{
    Q_D(const MetaDataJobBase);
    return d->serverCapability;
}
