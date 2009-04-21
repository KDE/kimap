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

#include <KDE/KLocale>
#include <KDE/KDebug>

#include "quotajobbase_p.h"
#include "message_p.h"
#include "session_p.h"

namespace KIMAP
{
  class GetQuotaRootJobPrivate : public QuotaJobBasePrivate
  {
    public:
      GetQuotaRootJobPrivate( Session *session, const QString& name ) : QuotaJobBasePrivate(session, name) { }
      ~GetQuotaRootJobPrivate() { }

      QByteArray mailBox;
      QList<QByteArray> rootList;
      uint rootIndex;
      QMap< QByteArray, QMap<QByteArray, QPair<qint32, qint32> > > quotas;
  };
}

using namespace KIMAP;

GetQuotaRootJob::GetQuotaRootJob( Session *session )
  : QuotaJobBase( *new QuotaJobBasePrivate(session, i18n("GetQuotaRoot")) )
{
}

GetQuotaRootJob::~GetQuotaRootJob()
{
}

void GetQuotaRootJob::doStart()
{
  Q_D(GetQuotaRootJob);
  d->tag = d->sessionInternal()->sendCommand( "GETQUOTAROOT", '\"' + d->mailBox + '\"');
}

void GetQuotaRootJob::doHandleResponse(const Message &response)
{
  Q_D(GetQuotaRootJob);
  if (handleErrorReplies(response) == NotHandled) {
    if ( response.content.size() >= 4 ) {
      if (response.content[1].toString() == "QUOTAROOT" ) {
        d->rootList.clear();
        int i = 3;
        while ( i < response.content.size())
        {
          d->rootList.append(response.content[i].toString());
          i++;
        }
        d->rootIndex = 0;
      } else
      if (response.content[1].toString() == "QUOTA" ) {
        //TODO: check if we should use the roots in order it came in QUOTAROOT response or the root name from the QUOTA response itself
        d->quotas[ d->rootList[d->rootIndex] ] = d->readQuota(response.content[3]);
        d->rootIndex++;
      }
    }
  }
}


void GetQuotaRootJob::setMailBox(const QByteArray& mailBox)
{
  Q_D(GetQuotaRootJob);

  d->mailBox = mailBox;
}

QByteArray GetQuotaRootJob::mailBox() const
{
  Q_D(const GetQuotaRootJob);

  return d->mailBox;
}

QList<QByteArray> GetQuotaRootJob::roots() const
{
  Q_D(const GetQuotaRootJob);

  return d->rootList;
}

qint32 GetQuotaRootJob::usage(const QByteArray& root, const QByteArray& resource)
{
  Q_D(GetQuotaRootJob);

  QByteArray r = resource.toUpper();

  if (d->quotas.contains(root) && d->quotas[root].contains(r)) {
    return d->quotas[root][r].first;
  }

  return -1;
}

qint32 GetQuotaRootJob::limit(const QByteArray& root, const QByteArray& resource)
{
  Q_D(GetQuotaRootJob);

  QByteArray r = resource.toUpper();

  if (d->quotas.contains(root) && d->quotas[root].contains(r)) {
    return d->quotas[root][r].second;
  }

  return -1;
}


#include "getquotarootjob.moc"
