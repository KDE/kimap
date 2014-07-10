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

#include "renamejob.h"

#include <KLocalizedString>

#include "job_p.h"
#include "message_p.h"
#include "session_p.h"
#include "rfccodecs.h"

namespace KIMAP
{
  class RenameJobPrivate : public JobPrivate
  {
    public:
      RenameJobPrivate( Session *session, const QString& name ) : JobPrivate( session, name ) { }
      ~RenameJobPrivate() { }

      QString sourceMailBox;
      QString destinationMailBox;
  };
}

using namespace KIMAP;

RenameJob::RenameJob( Session *session )
  : Job( *new RenameJobPrivate( session, i18n( "Rename" ) ) )
{
}

RenameJob::~RenameJob()
{
}

void RenameJob::doStart()
{
  Q_D( RenameJob );
  d->tags << d->sessionInternal()->sendCommand( "RENAME", '\"' + KIMAP::encodeImapFolderName( d->sourceMailBox.toUtf8() ) +
                                                "\" \"" + KIMAP::encodeImapFolderName( d->destinationMailBox.toUtf8() ) + '\"' );
}

void RenameJob::setSourceMailBox( const QString &mailBox )
{
  Q_D( RenameJob );
  d->sourceMailBox = mailBox;
}

QString RenameJob::sourceMailBox() const
{
  Q_D( const RenameJob );
  return d->sourceMailBox;
}

void RenameJob::setDestinationMailBox( const QString &mailBox )
{
  Q_D( RenameJob );
  d->destinationMailBox = mailBox;
}

QString RenameJob::destinationMailBox() const
{
  Q_D( const RenameJob );
  return d->destinationMailBox;
}
