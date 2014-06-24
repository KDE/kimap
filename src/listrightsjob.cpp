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

#include "listrightsjob.h"

#include <KDE/KLocalizedString>

#include "acljobbase_p.h"
#include "message_p.h"
#include "session_p.h"
#include "rfccodecs.h"

namespace KIMAP
{
  class ListRightsJobPrivate : public AclJobBasePrivate
  {
    public:
      ListRightsJobPrivate( Session *session, const QString& name ) : AclJobBasePrivate( session, name ), defaultRights( Acl::None ) {}
      ~ListRightsJobPrivate() { }

      Acl::Rights defaultRights;
      QList<Acl::Rights> possibleRights;

  };
}

using namespace KIMAP;

ListRightsJob::ListRightsJob( Session *session )
  : AclJobBase( *new ListRightsJobPrivate( session, i18n( "ListRights" ) ) )
{

}

ListRightsJob::~ListRightsJob()
{
}

void ListRightsJob::doStart()
{
  Q_D( ListRightsJob );

  d->tags << d->sessionInternal()->sendCommand( "LISTRIGHTS", '\"' + KIMAP::encodeImapFolderName( d->mailBox.toUtf8() ) + "\" \"" + d->id + "\"" );
}

void ListRightsJob::handleResponse( const Message &response )
{
  Q_D( ListRightsJob );

  if ( handleErrorReplies( response ) == NotHandled ) {
    if ( response.content.size() >= 4 &&
         response.content[1].toString() == "LISTRIGHTS" ) {
      QByteArray s = response.content[4].toString();
      d->defaultRights = Acl::rightsFromString( s );
      int i = 5;
      while ( i < response.content.size() ) {
        s = response.content[i].toString();
        d->possibleRights.append( Acl::rightsFromString( s ) );
        i++;
      }
   }
  }
}

void ListRightsJob::setIdentifier( const QByteArray &identifier )
{
  Q_D( ListRightsJob );
  d->setIdentifier( identifier );
}

QByteArray ListRightsJob::identifier()
{
  Q_D( ListRightsJob );
  return d->identifier();
}

Acl::Rights ListRightsJob::defaultRights()
{
  Q_D( ListRightsJob );
  return d->defaultRights;
}

QList<Acl::Rights> ListRightsJob::possibleRights()
{
  Q_D( ListRightsJob );
  return d->possibleRights;
}
