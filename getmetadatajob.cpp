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

#include <KDE/KLocale>
#include <KDE/KDebug>

#include "metadatajobbase_p.h"
#include "message_p.h"
#include "session_p.h"

namespace KIMAP
{
  class GetMetaDataJobPrivate : public MetaDataJobBasePrivate
  {
    public:
      GetMetaDataJobPrivate( Session *session, const QString& name ) : MetaDataJobBasePrivate(session, name), maxSize(-1), depth("0") { }
      ~GetMetaDataJobPrivate() { }

       qint32 maxSize;
       QByteArray depth;
       QList<QByteArray> entries;
       QList<QByteArray> attributes;
       QMap<QByteArray, QMap<QByteArray, QMap<QByteArray, QByteArray> > > metadata;
       //    ^ mailbox        ^ entry          ^attribute  ^ value
  };
}

using namespace KIMAP;

GetMetaDataJob::GetMetaDataJob( Session *session )
  : MetaDataJobBase( *new GetMetaDataJobPrivate(session, i18n("GetMetaData")) )
{
}

GetMetaDataJob::~GetMetaDataJob()
{
}

void GetMetaDataJob::doStart()
{
  Q_D(GetMetaDataJob);
  QByteArray parameters;
  parameters = '\"' + d->mailBox + "\" ";

  QByteArray command = "GETMETADATA";
  if (d->serverCapability == Annotatemore) {
    d->m_name = i18n("GetAnnotation");
    command = "GETANNOTATION";
    if (d->entries.size() > 1)
      parameters += "(";
    Q_FOREACH(QByteArray entry, d->entries) {
      parameters += '\"' + entry + "\" ";
    }
    if (d->entries.size() > 1)
      parameters[parameters.length() -1 ] = ')';
    else
      parameters.truncate(parameters.length() -1);

    parameters += ' ';

    if (d->attributes.size() > 1)
      parameters += "(";
    Q_FOREACH(QByteArray attribute, d->attributes) {
      parameters += '\"' + attribute + "\" ";
    }
    if (d->attributes.size() > 1)
      parameters[parameters.length() -1 ] = ')';
    else
      parameters.truncate(parameters.length() -1);

  } else {
    if (d->depth != "0") {
      parameters += "(DEPTH " + d->depth;
    }
    if (d->maxSize != -1) {
      parameters += "(MAXSIZE " + QByteArray::number(d->maxSize) + ')';
    }
    if (d->depth != "0") {
      parameters += " )";
    }

    if (d->entries.size() > 1)
      parameters += "(";
    Q_FOREACH(QByteArray entry, d->entries) {
      parameters += '\"' + entry + "\" ";
    }
    if (d->entries.size() > 1)
      parameters[parameters.length() -1 ] = ')';
  }

  if (d->entries.isEmpty()) {
    parameters += ')';
  }

  d->tag = d->sessionInternal()->sendCommand( command, parameters );
  kDebug() << "SENT: " << command << " " << parameters;
}

void GetMetaDataJob::doHandleResponse( const Message &response )
{
  Q_D(GetMetaDataJob);
  kDebug() << "GOT: " << response.toString();

  //TODO: handle NO error messages having [METADATA MAXSIZE NNN], [METADATA TOOMANY], [METADATA NOPRIVATE] (see rfc5464)
  // or [ANNOTATEMORE TOOBIG], [ANNOTATEMORE TOOMANY] respectively
  if (handleErrorReplies(response) == NotHandled ) {
    if ( response.content.size() >= 4 ) {
      if (d->serverCapability == Annotatemore && response.content[1].toString() == "ANNOTATION" ) {
        QByteArray mailBox = response.content[2].toString();

        int i = 3;
        while (i < response.content.size() - 1) {
          QByteArray entry = response.content[i].toString();
          QList<QByteArray> attributes = response.content[i + 1].toList();
          int j = 0;
          while ( j < attributes.size() - 1) {
            d->metadata[mailBox][entry][attributes[j]] = attributes[j + 1];
            j += 2;
          }
          i += 2;
        }
      } else
      if (d->serverCapability == Metadata && response.content[1].toString() == "METADATA" ) {
        QByteArray mailBox = response.content[2].toString();

        QList<QByteArray> entries = response.content[3].toList();
        int i = 0;
        while ( i < entries.size() - 1) {
          d->metadata[mailBox][entries[i]][""] = entries[i + 1];
          i += 2;
        }
      }
    }
  }
}

void GetMetaDataJob::addEntry(const QByteArray &entry)
{
  Q_D(GetMetaDataJob);
  d->entries.append(entry);
}

void GetMetaDataJob::addAttribute(const QByteArray &attribute)
{
  Q_D(GetMetaDataJob);
  d->attributes.append(attribute);
}

void GetMetaDataJob::setMaxSize(qint32 size)
{
  Q_D(GetMetaDataJob);
  d->maxSize = size;
}

void GetMetaDataJob::setDepth(Depth depth)
{
  Q_D(GetMetaDataJob);

  switch (depth)
  {
    case OneLevel:
      d->depth = "1";
      break;
    case AllLevels:
      d->depth = "infinity";
      break;
    default:
      d->depth = "0";
  }
}

QByteArray GetMetaDataJob::metaData(const QByteArray &mailBox, const QByteArray &entry)
{
  return metaData(mailBox, entry, "");
}


QByteArray GetMetaDataJob::metaData(const QByteArray &mailBox, const QByteArray &entry, const QByteArray &attribute)
{
  Q_D(GetMetaDataJob);
  QByteArray attr = attribute;

  if (d->serverCapability == Metadata)
     attr = "";

  QByteArray result;
  if (d->metadata.contains(mailBox)) {
    if (d->metadata[mailBox].contains(entry)) {
      result = d->metadata[mailBox][entry].value(attr);
    }
  }

  return result;
}


#include "getmetadatajob.moc"
