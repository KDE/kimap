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

#include "quotajobbase.h"
#include "quotajobbase_p.h"
#include "message_p.h"
#include "session_p.h"

#include <KDE/KLocalizedString>

using namespace KIMAP;

QMap<QByteArray, QPair<qint64, qint64> > QuotaJobBasePrivate::readQuota( const Message::Part &content )
{
  QMap<QByteArray, QPair<qint64, qint64> > quotaMap;
  QList<QByteArray> quotas = content.toList();

  int i = 0;
  while ( i < quotas.size() - 2 ) {
    QByteArray resource = quotas[i].toUpper();
    qint64 usage = quotas[i+1].toInt();
    qint64 limit = quotas[i+2].toInt();
    quotaMap[resource] = qMakePair( usage, limit );
    i += 3;
  }

  return quotaMap;
}

QuotaJobBase::QuotaJobBase( Session *session )
  : Job( *new QuotaJobBasePrivate( session, i18n( "QuotaJobBase" ) ) )
{
}

QuotaJobBase::QuotaJobBase( JobPrivate &dd )
  : Job( dd )
{
}

QuotaJobBase::~QuotaJobBase()
{
}

qint64 QuotaJobBase::usage(const QByteArray& resource)
{
  Q_D( QuotaJobBase );

  QByteArray r = resource.toUpper();

  if ( d->quota.contains( r ) ) {
     return d->quota[r].first;
  }
  return -1;
}

qint64 QuotaJobBase::limit(const QByteArray& resource)
{
  Q_D( QuotaJobBase );

  QByteArray r = resource.toUpper();

  if ( d->quota.contains( r ) ) {
    return d->quota[r].second;
  }
  return -1;
}
