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

#include <KDE/KLocale>
#include <KDE/KDebug>

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
      CopyJobPrivate( Session *session, const QString& name ) : JobPrivate(session, name) { }
      ~CopyJobPrivate() { }

      QString mailBox;
      ImapSet set;
  };
}

using namespace KIMAP;

CopyJob::CopyJob( Session *session )
  : Job( *new CopyJobPrivate(session, i18n("Copy")) )
{
}

CopyJob::~CopyJob()
{
}

void CopyJob::doStart()
{
  Q_D(CopyJob);
  d->tag = d->sessionInternal()->sendCommand( "COPY", d->set.toImapSequenceSet() + " \"" + KIMAP::encodeImapFolderName( d->mailBox.toUtf8() ) + '\"' );
}

void CopyJob::setMailBox( const QString &mailBox )
{
  Q_D(CopyJob);
  d->mailBox = mailBox;
}

QString CopyJob::mailBox() const
{
  Q_D(const CopyJob);
  return d->mailBox;
}

void CopyJob::setSequenceSet( const ImapSet &set )
{
  Q_D(CopyJob);
  d->set = set;
}

ImapSet CopyJob::sequenceSet() const
{
  Q_D(const CopyJob);
  return d->set;
}


#include "copyjob.moc"
