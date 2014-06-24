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

#include "copyjob.h"

#include <KDE/KLocalizedString>

#include "job_p.h"
#include "message_p.h"
#include "session_p.h"
#include "rfccodecs.h"

//TODO: when custom error codes are introduced, handle the NO [TRYCREATE] response

namespace KIMAP
{
  class CopyJobPrivate : public JobPrivate
  {
    public:
      CopyJobPrivate( Session *session, const QString& name ) : JobPrivate( session, name ) { }
      ~CopyJobPrivate() { }

      QString mailBox;
      ImapSet set;
      bool uidBased;
      ImapSet resultingUids;
  };
}

using namespace KIMAP;

CopyJob::CopyJob( Session *session )
  : Job( *new CopyJobPrivate( session, i18n( "Copy" ) ) )
{
  Q_D( CopyJob );
  d->uidBased = false;
}

CopyJob::~CopyJob()
{
}

void CopyJob::setMailBox( const QString &mailBox )
{
  Q_D( CopyJob );
  d->mailBox = mailBox;
}

QString CopyJob::mailBox() const
{
  Q_D( const CopyJob );
  return d->mailBox;
}

void CopyJob::setSequenceSet( const ImapSet &set )
{
  Q_D( CopyJob );
  d->set = set;
}

ImapSet CopyJob::sequenceSet() const
{
  Q_D( const CopyJob );
  return d->set;
}

void CopyJob::setUidBased( bool uidBased )
{
  Q_D( CopyJob );
  d->uidBased = uidBased;
}

bool CopyJob::isUidBased() const
{
  Q_D( const CopyJob );
  return d->uidBased;
}

ImapSet CopyJob::resultingUids() const
{
  Q_D( const CopyJob );
  return d->resultingUids;
}

void CopyJob::doStart()
{
  Q_D( CopyJob );

  QByteArray parameters = d->set.toImapSequenceSet()+' ';
  parameters += '\"' + KIMAP::encodeImapFolderName( d->mailBox.toUtf8() ) + '\"';

  QByteArray command = "COPY";
  if ( d->uidBased ) {
    command = "UID "+command;
  }

  d->tags << d->sessionInternal()->sendCommand( command, parameters );
}

void CopyJob::handleResponse( const Message &response )
{
  Q_D( CopyJob );

  for ( QList<Message::Part>::ConstIterator it = response.responseCode.begin();
        it != response.responseCode.end(); ++it ) {
    if ( it->toString() == "COPYUID" ) {
      it = it + 3;
      if ( it < response.responseCode.end() ) {
        d->resultingUids = ImapSet::fromImapSequenceSet( it->toString() );
      }
      break;
    }
  }

  handleErrorReplies( response );
}
