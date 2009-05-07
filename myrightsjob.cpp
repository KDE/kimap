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

#include "myrightsjob.h"

#include <KDE/KLocale>
#include <KDE/KDebug>

#include "acljobbase_p.h"
#include "message_p.h"
#include "session_p.h"
#include "rfccodecs.h"

namespace KIMAP
{
  class MyRightsJobPrivate : public AclJobBasePrivate
  {
    public:
      MyRightsJobPrivate( Session *session, const QString& name ) : AclJobBasePrivate(session, name) {}
      ~MyRightsJobPrivate() { }

      QList<AclJobBase::AclRight> myRights;
  };
}

using namespace KIMAP;

MyRightsJob::MyRightsJob( Session *session )
  : AclJobBase( *new MyRightsJobPrivate(session, i18n("MyRights") ))
{
}

MyRightsJob::~MyRightsJob()
{
}

void MyRightsJob::doStart()
{
  Q_D(MyRightsJob);

  d->tag = d->sessionInternal()->sendCommand( "MYRIGHTS", '\"' + KIMAP::encodeImapFolderName( d->mailBox ) + '\"');
}

void MyRightsJob::doHandleResponse( const Message &response )
{
  Q_D(MyRightsJob);

  if (handleErrorReplies(response) == NotHandled) {
    if ( response.content.size() == 4
         && response.content[1].toString() == "MYRIGHTS" ) {
      d->myRights = d->rightsFromString( response.content[3].toString() );
    }
  }
}

bool MyRightsJob::hasRightEnabled(AclJobBase::AclRight right)
{
  Q_D(MyRightsJob);
  return d->myRights.contains(right);
}

QList<AclJobBase::AclRight> MyRightsJob::rights()
{
  Q_D(MyRightsJob);
  return d->myRights;
}

#include "myrightsjob.moc"
