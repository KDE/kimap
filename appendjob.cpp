/*
    Copyright (c) 2009 Kevin Ottens <ervin@kde.org>

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

#include "appendjob.h"

#include <KDE/KLocale>

#include "job_p.h"
#include "message_p.h"
#include "session_p.h"

namespace KIMAP
{
  class AppendJobPrivate : public JobPrivate
  {
    public:
      AppendJobPrivate( Session *session, const QString& name ) : JobPrivate( session, name ) { }
      ~AppendJobPrivate() { }

      QByteArray mailBox;
      QList<QByteArray> flags;
      QByteArray content;
  };
}

using namespace KIMAP;

AppendJob::AppendJob( Session *session )
  : Job( *new AppendJobPrivate(session, i18n("Append")) )
{
}

AppendJob::~AppendJob()
{
}

void AppendJob::setMailBox( const QByteArray &mailBox )
{
  Q_D(AppendJob);
  d->mailBox = mailBox;
}

QByteArray AppendJob::mailBox() const
{
  Q_D(const AppendJob);
  return d->mailBox;
}

void AppendJob::setFlags( const QList<QByteArray> &flags)
{
  Q_D(AppendJob);
  d->flags = flags;
}

QList<QByteArray> AppendJob::flags() const
{
  Q_D(const AppendJob);
  return d->flags;
}

void AppendJob::setContent( const QByteArray &content )
{
  Q_D(AppendJob);
  d->content = content;
}

QByteArray AppendJob::content() const
{
  Q_D(const AppendJob);
  return d->content;
}

void AppendJob::doStart()
{
  Q_D(AppendJob);

  QByteArray parameters = d->mailBox;

  if ( !d->flags.isEmpty() ) {
    parameters+=" (";
    foreach ( const QByteArray &flag, d->flags ) {
      parameters+= flag+' ';
    }
    parameters.chop(1);
    parameters+=')';
  }

  parameters+=" {"+QByteArray::number(d->content.size())+'}';
  qDebug("%s", parameters.constData());

  d->tag = d->sessionInternal()->sendCommand( "APPEND", parameters );
}

void AppendJob::doHandleResponse( const Message &response )
{
  Q_D(AppendJob);

  if (handleErrorReplies(response) == NotHandled ) {
    if ( response.content[0].toString() == "+" ) {
      d->sessionInternal()->sendData( d->content );
    }
  }
}

#include "appendjob.moc"
