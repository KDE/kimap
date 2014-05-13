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

#include "getmetadatajob.h"

#include <KDE/KLocalizedString>
#include <QDebug>

#include "metadatajobbase_p.h"
#include "message_p.h"
#include "session_p.h"
#include "rfccodecs.h"

namespace KIMAP
{
  class GetMetaDataJobPrivate : public MetaDataJobBasePrivate
  {
    public:
      GetMetaDataJobPrivate( Session *session, const QString& name ) : MetaDataJobBasePrivate( session, name ), maxSize( -1 ), depth( "0" ) { }
      ~GetMetaDataJobPrivate() { }

       qint64 maxSize;
       QByteArray depth;
       QList<QByteArray> entries;
       QList<QByteArray> attributes;
       QMap<QString, QMap<QByteArray, QMap<QByteArray, QByteArray> > > metadata;
       //    ^ mailbox        ^ entry          ^attribute  ^ value
  };
}

using namespace KIMAP;

GetMetaDataJob::GetMetaDataJob( Session *session )
  : MetaDataJobBase( *new GetMetaDataJobPrivate( session, i18n( "GetMetaData" ) ) )
{
}

GetMetaDataJob::~GetMetaDataJob()
{
}

void GetMetaDataJob::doStart()
{
  Q_D( GetMetaDataJob );
  QByteArray parameters;
  parameters = '\"' + KIMAP::encodeImapFolderName( d->mailBox.toUtf8() ) + "\" ";

  QByteArray command = "GETMETADATA";
  if ( d->serverCapability == Annotatemore ) {
    d->m_name = i18n( "GetAnnotation" );
    command = "GETANNOTATION";
    if ( d->entries.size() > 1 ) {
      parameters += '(';
    }
    Q_FOREACH ( const QByteArray &entry, d->entries ) {
      parameters += '\"' + entry + "\" ";
    }
    if ( d->entries.size() > 1 ) {
      parameters[parameters.length() - 1 ] = ')';
      parameters += ' ';
    }

    if ( d->attributes.size() > 1 ) {
      parameters += '(';
    }
    Q_FOREACH ( const QByteArray &attribute, d->attributes ) {
      parameters += '\"' + attribute + "\" ";
    }
    if ( d->attributes.size() > 1 ) {
      parameters[parameters.length() - 1 ] = ')';
    } else {
      parameters.truncate( parameters.length() - 1 );
    }

  } else {

    QByteArray options;
    if ( d->depth != "0" ) {
      options = "DEPTH " + d->depth;
    }
    if ( d->maxSize != -1 ) {
      if ( !options.isEmpty() ) {
        options += ' ';
      }
      options += "MAXSIZE " + QByteArray::number( d->maxSize );
    }

    if ( !options.isEmpty() ) {
      parameters = "(" + options + ") " + parameters;
    }

    if ( d->entries.size() >= 1 ) {
      parameters += '(';
      Q_FOREACH ( const QByteArray &entry, d->entries ) {
        parameters += entry + " ";
      }
      parameters[parameters.length() - 1 ] = ')';
    } else {
      parameters.truncate( parameters.length() - 1 );
    }
  }

  d->tags << d->sessionInternal()->sendCommand( command, parameters );
//  qDebug() << "SENT: " << command << " " << parameters;
}

void GetMetaDataJob::handleResponse( const Message &response )
{
  Q_D( GetMetaDataJob );
//  qDebug() << "GOT: " << response.toString();

  //TODO: handle NO error messages having [METADATA MAXSIZE NNN], [METADATA TOOMANY], [METADATA NOPRIVATE] (see rfc5464)
  // or [ANNOTATEMORE TOOBIG], [ANNOTATEMORE TOOMANY] respectively
  if ( handleErrorReplies( response ) == NotHandled  ) {
    if ( response.content.size() >= 4 ) {
      if ( d->serverCapability == Annotatemore && response.content[1].toString() == "ANNOTATION" ) {
        QString mailBox = QString::fromUtf8( KIMAP::decodeImapFolderName( response.content[2].toString() ) );

        int i = 3;
        while ( i < response.content.size() - 1 ) {
          QByteArray entry = response.content[i].toString();
          QList<QByteArray> attributes = response.content[i + 1].toList();
          int j = 0;
          while ( j < attributes.size() - 1 ) {
            d->metadata[mailBox][entry][attributes[j]] = attributes[j + 1];
            j += 2;
          }
          i += 2;
        }
      } else if ( d->serverCapability == Metadata && response.content[1].toString() == "METADATA" ) {
        QString mailBox = QString::fromUtf8( KIMAP::decodeImapFolderName( response.content[2].toString() ) );

        const QList<QByteArray> &entries = response.content[3].toList();
        int i = 0;
        while ( i < entries.size() - 1 ) {
          const QByteArray &value = entries[i + 1];
          QByteArray &targetValue = d->metadata[mailBox][entries[i]][""];
          if ( value != "NIL" ) { //This just indicates no value
            targetValue = value;
          }
          i += 2;
        }
      }
    }
  }
}

void GetMetaDataJob::addEntry(const QByteArray &entry, const QByteArray &attribute)
{
  Q_D( GetMetaDataJob );
  if ( d->serverCapability == Annotatemore && attribute.isNull() ) {
    qWarning() << "In ANNOTATEMORE mode an attribute must be specified with addEntry!";
  }
  d->entries.append( entry );
  d->attributes.append( attribute );
}

void GetMetaDataJob::addRequestedEntry(const QByteArray &entry)
{
  Q_D( GetMetaDataJob );
  d->entries.append( d->removePrefix(entry) );
  d->attributes.append( d->getAttribute( entry ) );
}

void GetMetaDataJob::setMaximumSize(qint64 size)
{
  Q_D( GetMetaDataJob );
  d->maxSize = size;
}

void GetMetaDataJob::setDepth(Depth depth)
{
  Q_D( GetMetaDataJob );

  switch ( depth ) {
    case OneLevel:
      d->depth = "1"; //krazy:exclude=doublequote_chars
      break;
    case AllLevels:
      d->depth = "infinity";
      break;
    default:
      d->depth = "0"; //krazy:exclude=doublequote_chars
  }
}

QByteArray GetMetaDataJob::metaData(const QString &mailBox, const QByteArray &entry, const QByteArray &attribute) const
{
  Q_D( const GetMetaDataJob );
  QByteArray attr = attribute;

  if ( d->serverCapability == Metadata ) {
     attr = "";
  }

  QByteArray result;
  if ( d->metadata.contains( mailBox ) ) {
    if ( d->metadata[mailBox].contains( entry ) ) {
      result = d->metadata[mailBox][entry].value( attr );
    }
  }
  return result;
}

QByteArray GetMetaDataJob::metaData(const QByteArray& entry) const
{
  qDebug() << entry;
  Q_D( const GetMetaDataJob );
  return d->metadata.value( d->mailBox ).value( d->removePrefix(entry) ).value( d->getAttribute( entry ) );
}

QMap<QByteArray, QMap<QByteArray, QByteArray> > GetMetaDataJob::allMetaData(const QString &mailBox) const
{
  Q_D( const GetMetaDataJob );
  return d->metadata[mailBox];
}

QMap<QByteArray, QByteArray> GetMetaDataJob::allMetaData() const
{
  Q_D( const GetMetaDataJob );
  const QMap<QByteArray, QMap<QByteArray, QByteArray> > &entries = d->metadata[d->mailBox];
  QMap<QByteArray, QByteArray> map;
  foreach(const QByteArray &entry, entries.keys()) {
    const QMap<QByteArray, QByteArray> &values = entries[entry];
    foreach(const QByteArray &attribute, values.keys()) {
      map.insert(d->addPrefix(entry, attribute), values[attribute]);
    }
  }
  return map;
}

