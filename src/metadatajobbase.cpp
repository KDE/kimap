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

#include "metadatajobbase.h"
#include "metadatajobbase_p.h"
#include "message_p.h"
#include "session_p.h"

#include <KDE/KLocalizedString>

using namespace KIMAP;

QByteArray MetaDataJobBasePrivate::addPrefix( const QByteArray &entry, const QByteArray &attribute ) const
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


QByteArray MetaDataJobBasePrivate::removePrefix( const QByteArray &entry ) const
{
  if (serverCapability == MetaDataJobBase::Annotatemore) {
    if ( entry.startsWith("/shared")) {
      return entry.mid(QByteArray("/shared").size());
    }
    if ( entry.startsWith("/private")) {
      return entry.mid(QByteArray("/private").size());
    }
  }
  return entry;
}

QByteArray MetaDataJobBasePrivate::getAttribute( const QByteArray &entry ) const
{
  if (serverCapability == MetaDataJobBase::Annotatemore) {
    if ( entry.startsWith("/shared")) {
      return QByteArray("value.shared");
    } else if (entry.startsWith("/private")) {
      return QByteArray("value.priv");
    }
  }
  return QByteArray();
}

MetaDataJobBase::MetaDataJobBase( Session *session )
  : Job( *new MetaDataJobBasePrivate( session, i18n( "MetaDataJobBase" ) ) )
{
}

MetaDataJobBase::MetaDataJobBase( JobPrivate &dd )
  : Job( dd )
{
}

MetaDataJobBase::~MetaDataJobBase()
{
}

void MetaDataJobBase::setMailBox( const QString &mailBox )
{
  Q_D( MetaDataJobBase );
  d->mailBox = mailBox;
}

QString MetaDataJobBase::mailBox() const
{
  Q_D( const MetaDataJobBase );
  return d->mailBox;
}

void MetaDataJobBase::setServerCapability(const ServerCapability& capability)
{
  Q_D( MetaDataJobBase );
  d->serverCapability = capability;
}

MetaDataJobBase::ServerCapability MetaDataJobBase::serverCapability() const
{
  Q_D( const MetaDataJobBase );
  return d->serverCapability;
}
