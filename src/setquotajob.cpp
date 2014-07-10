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

#include "setquotajob.h"

#include <KLocalizedString>
#include <QDebug>

#include "quotajobbase_p.h"
#include "message_p.h"
#include "session_p.h"

namespace KIMAP
{
  class SetQuotaJobPrivate : public QuotaJobBasePrivate
  {
    public:
      SetQuotaJobPrivate( Session *session, const QString& name ) : QuotaJobBasePrivate( session, name ) { }
      ~SetQuotaJobPrivate() { }

      QMap<QByteArray, qint64> setList;
      QByteArray root;
  };
}

using namespace KIMAP;

SetQuotaJob::SetQuotaJob( Session *session )
  : QuotaJobBase( *new SetQuotaJobPrivate( session, i18n( "SetQuota" ) ) )
{
}

SetQuotaJob::~SetQuotaJob()
{
}

void SetQuotaJob::doStart()
{
  Q_D( SetQuotaJob );
  QByteArray s;
  s += '(';
  for ( QMap<QByteArray, qint64>::ConstIterator it = d->setList.constBegin(); it != d->setList.constEnd(); ++it ) {
    s += it.key() + ' ' + QByteArray::number( it.value() ) + ' ';
  }
  if ( d->setList.isEmpty() ) {
    s += ')';
  } else {
    s[s.length() - 1] = ')';
  }

  qDebug() << "SETQUOTA " << '\"' + d->root + "\" " + s;
  //XXX: [alexmerry, 2010-07-24]: should d->root be quoted properly?
  d->tags << d->sessionInternal()->sendCommand( "SETQUOTA", '\"' + d->root + "\" " + s );
}

void SetQuotaJob::handleResponse(const Message &response)
{
  Q_D( SetQuotaJob );
  if ( handleErrorReplies( response ) == NotHandled ) {
    if ( response.content.size() >= 4 &&
         response.content[1].toString() == "QUOTA" ) {
      d->quota = d->readQuota( response.content[3] );
    }
  }
}

void SetQuotaJob::setQuota(const QByteArray &resource, qint64 limit)
{
  Q_D( SetQuotaJob );

  d->setList[resource.toUpper()] = limit;
}

void SetQuotaJob::setRoot(const QByteArray& root)
{
  Q_D( SetQuotaJob );

  d->root = root;
}

QByteArray SetQuotaJob::root() const
{
  Q_D( const SetQuotaJob );

  return d->root;
}
