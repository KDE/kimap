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

#include <KDE/KLocale>
#include <KDE/KDebug>

#include "job_p.h"
#include "message_p.h"
#include "session_p.h"
#include "rfccodecs.h"

namespace KIMAP
{
  class RenameJobPrivate : public JobPrivate
  {
    public:
      RenameJobPrivate( Session *session, const QString& name ) : JobPrivate(session, name) { }
      ~RenameJobPrivate() { }

      QString mailBox;
      QString newMailBox;
  };
}

using namespace KIMAP;

RenameJob::RenameJob( Session *session )
  : Job( *new RenameJobPrivate(session, i18n("Rename")) )
{
}

RenameJob::~RenameJob()
{
}

void RenameJob::doStart()
{
  Q_D(RenameJob);
  d->tag = d->sessionInternal()->sendCommand( "RENAME", '\"' + KIMAP::encodeImapFolderName( d->mailBox.toUtf8() ) + "\" \""
                                              + KIMAP::encodeImapFolderName( d->newMailBox.toUtf8() )+ '\"' );
}

void RenameJob::setMailBox( const QString &mailBox )
{
  Q_D(RenameJob);
  d->mailBox = mailBox;
}

QString RenameJob::mailBox() const
{
  Q_D(const RenameJob);
  return d->mailBox;
}

void RenameJob::setNewMailBox( const QString &mailBox )
{
  Q_D(RenameJob);
  d->newMailBox = mailBox;
}

QString RenameJob::newMailBox() const
{
  Q_D(const RenameJob);
  return d->newMailBox;
}


#include "renamejob.moc"
