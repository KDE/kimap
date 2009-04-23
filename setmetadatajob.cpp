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

#include "setmetadatajob.h"

#include <KDE/KLocale>
#include <KDE/KDebug>

#include "metadatajobbase_p.h"
#include "message_p.h"
#include "session_p.h"

namespace KIMAP
{
  class SetMetaDataJobPrivate : public MetaDataJobBasePrivate
  {
    public:
      SetMetaDataJobPrivate( Session *session, const QString& name ) : MetaDataJobBasePrivate(session, name), metaDataError(0), maxAcceptedSize(-1) { }
      ~SetMetaDataJobPrivate() { }

      QMap<QByteArray, QByteArray> entries;
      QMap<QByteArray, QByteArray>::ConstIterator entriesIt;
      QByteArray entryName;
      int metaDataError;
      qint64 maxAcceptedSize;
  };
}

using namespace KIMAP;

SetMetaDataJob::SetMetaDataJob( Session *session )
  : MetaDataJobBase( *new SetMetaDataJobPrivate(session, i18n("SetMetaData")) )
{
}

SetMetaDataJob::~SetMetaDataJob()
{
}

void SetMetaDataJob::doStart()
{
  Q_D(SetMetaDataJob);
  QByteArray parameters;
  parameters = '\"' + d->mailBox + "\" ";
  d->entriesIt = d->entries.constBegin();

  QByteArray command = "SETMETADATA";
  if (d->serverCapability == Annotatemore) {
    command = "SETANNOTATION";
    parameters += '\"' + d->entryName + "\" (";
    d->m_name = i18n("SetAnnotation");
    if (!d->entries.isEmpty()) {
      for (; d->entriesIt != d->entries.constEnd(); ++d->entriesIt) {
        parameters  += '\"' + d->entriesIt.key() + "\" \"" + d->entriesIt.value() + "\" ";
      }
      parameters[parameters.length() - 1] = ')';
    }
  } else {
    parameters += "(";
    if (!d->entries.isEmpty()) {
      parameters += '\"' + d->entriesIt.key() + '\"';
      parameters += ' ';
      parameters +=" {" + QByteArray::number(d->entriesIt.value().size()) + '}';
    }
  }

  if (d->entries.isEmpty()) {
    parameters += ')';
  }

  d->tag = d->sessionInternal()->sendCommand( command, parameters );
//   kDebug() << "SENT: " << command << " " << parameters;
}

void SetMetaDataJob::doHandleResponse( const Message &response )
{
  Q_D(SetMetaDataJob);

  //TODO: Test if a server can really return more then one untagged NO response. If not, no need to OR the error codes
  if ( !response.content.isEmpty()
        && response.content.first().toString() == d->tag ) {
    if ( response.content[1].toString() == "NO" ) {
      setError( UserDefinedError );
      setErrorText( i18n("%1 failed, server replied: %2", d->m_name, response.toString().constData()) );
      if (response.content[2].toString() == "[ANNOTATEMORE TOOMANY]" || response.content[2].toString() == "[METADATA TOOMANY]") {
        d->metaDataError |= TooMany;
      } else if (response.content[2].toString() == "[ANNOTATEMORE TOOBIG]" || response.content[2].toString().startsWith("[METADATA MAXSIZE")) {
        d->metaDataError |= TooBig;
        d->maxAcceptedSize = -1;
        if (response.content[2].toString().startsWith("[METADATA MAXSIZE")) {
          QByteArray max = response.content[2].toString();
          max.replace("[METADATA MAXSIZE", "");
          max.replace("]","");
          d->maxAcceptedSize = max.toLongLong();
        }
      } else if (response.content[2].toString() == "[METADATA NOPRIVATE]") {
        d->metaDataError |= NoPrivate;
      }
    } else if ( response.content.size() < 2 ) {
      setErrorText( i18n("%1 failed, malformed reply from the server.", d->m_name) );
    } else if ( response.content[1].toString() != "OK" ) {
      setError( UserDefinedError );
      setErrorText( i18n("%1 failed, server replied: %2", d->m_name, response.toString().constData()) );
    }
    emitResult();
   } else if ( d->serverCapability == Metadata && response.content[0].toString() == "+" ) {
      QByteArray content = d->entriesIt.value();
      ++d->entriesIt;
     if (d->entriesIt == d->entries.constEnd()) {
       content += ')';
     } else {
       content +=" {" + QByteArray::number(d->entriesIt.value().size()) + '}';
     }
//      kDebug() << "SENT: " << content;
     d->sessionInternal()->sendData( content );
  }
}

void SetMetaDataJob::addMetaData(const QByteArray &name, const QByteArray &value)
{
  Q_D(SetMetaDataJob);
  d->entries[name] = value;
}

void SetMetaDataJob::setEntry(const QByteArray &entry)
{
  Q_D(SetMetaDataJob);
  d->entryName = entry;
}

int SetMetaDataJob::metaDataError() const
{
  Q_D(const SetMetaDataJob);
  return d->metaDataError;
}

#include "setmetadatajob.moc"
