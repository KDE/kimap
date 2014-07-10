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

#include "getquotarootjob.h"

#include <KLocalizedString>

#include "quotajobbase_p.h"
#include "message_p.h"
#include "session_p.h"
#include "rfccodecs.h"

namespace KIMAP
{
  class GetQuotaRootJobPrivate : public QuotaJobBasePrivate
  {
    public:
      GetQuotaRootJobPrivate( Session *session, const QString& name ) : QuotaJobBasePrivate( session, name ) { }
      ~GetQuotaRootJobPrivate() { }

      QString mailBox;
      QList<QByteArray> rootList;
      QMap< QByteArray, QMap<QByteArray, QPair<qint64, qint64> > > quotas;
  };
}

using namespace KIMAP;

GetQuotaRootJob::GetQuotaRootJob( Session *session )
  : QuotaJobBase( *new GetQuotaRootJobPrivate( session, i18n( "GetQuotaRoot" ) ) )
{
}

GetQuotaRootJob::~GetQuotaRootJob()
{
}

void GetQuotaRootJob::doStart()
{
  Q_D( GetQuotaRootJob );
  d->tags << d->sessionInternal()->sendCommand( "GETQUOTAROOT", '\"' + KIMAP::encodeImapFolderName( d->mailBox.toUtf8() ) + '\"' );
}

void GetQuotaRootJob::handleResponse(const Message &response)
{
  Q_D( GetQuotaRootJob );
  if ( handleErrorReplies( response ) == NotHandled ) {
    if ( response.content.size() >= 3 ) {
      if ( response.content[1].toString() == "QUOTAROOT"  ) {
        d->rootList.clear();
        //some impls don't give the root a name which for us seems as if
        //there were no message part
        if ( response.content.size() == 3 ) {
          d->rootList.append( "" );
        } else {
          int i = 3;
          while ( i < response.content.size() ) {
            d->rootList.append( response.content[i].toString() );
            i++;
          }
        }
      } else if ( response.content[1].toString() == "QUOTA"  ) {
        QByteArray rootName;
        int  quotaContentIndex = 3;
        //some impls don't give the root a name in the response
        if ( response.content.size() == 3  ) {
          quotaContentIndex = 2;
        } else {
          rootName = response.content[2].toString();
        }

        const QMap<QByteArray, QPair<qint64, qint64> >& quota = d->readQuota(response.content[quotaContentIndex]);
        if ( d->quotas.contains( rootName ) ) {
          d->quotas[ rootName ].unite( quota );
        } else {
          d->quotas[ rootName ] = quota;
        }
      }
    }
  }
}

void GetQuotaRootJob::setMailBox(const QString& mailBox)
{
  Q_D( GetQuotaRootJob );
  d->mailBox = mailBox;
}

QString GetQuotaRootJob::mailBox() const
{
  Q_D( const GetQuotaRootJob );
  return d->mailBox;
}

QList<QByteArray> GetQuotaRootJob::roots() const
{
  Q_D( const GetQuotaRootJob );
  return d->rootList;
}

qint64 GetQuotaRootJob::usage(const QByteArray &root, const QByteArray &resource) const
{
  Q_D( const GetQuotaRootJob );
  QByteArray r = resource.toUpper();

  if ( d->quotas.contains( root ) && d->quotas[root].contains( r ) ) {
    return d->quotas[root][r].first;
  }
  return -1;
}

qint64 GetQuotaRootJob::limit(const QByteArray &root, const QByteArray &resource) const
{
  Q_D( const GetQuotaRootJob );

  QByteArray r = resource.toUpper();

  if ( d->quotas.contains( root ) && d->quotas[root].contains( r ) ) {
    return d->quotas[root][r].second;
  }
  return -1;
}

QMap<QByteArray, qint64> GetQuotaRootJob::allUsages(const QByteArray &root) const
{
  Q_D( const GetQuotaRootJob );

  QMap<QByteArray, qint64> result;

  if ( d->quotas.contains( root ) ) {
    const QMap< QByteArray, QPair<qint64, qint64> > quota = d->quotas[root];
    QMapIterator<QByteArray, QPair<qint64, qint64> > it( quota );
    while ( it.hasNext() ) {
      it.next();
      result[it.key()] = it.value().first;
    }
  }
  return result;
}

QMap<QByteArray, qint64> GetQuotaRootJob::allLimits(const QByteArray &root) const
{
  Q_D( const GetQuotaRootJob );

  QMap<QByteArray, qint64> result;

  if ( d->quotas.contains( root ) ) {
    const QMap< QByteArray, QPair<qint64, qint64> > quota = d->quotas[root];
    QMapIterator<QByteArray, QPair<qint64, qint64> > it( quota );
    while ( it.hasNext() ) {
      it.next();
      result[it.key()] = it.value().second;
    }
  }
  return result;
}
