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

#include "fetchjob.h"

#include <KDE/KLocale>

#include "job_p.h"
#include "message_p.h"
#include "session_p.h"

namespace KIMAP
{
  class FetchJobPrivate : public JobPrivate
  {
    public:
      FetchJobPrivate( Session *session ) : JobPrivate( session ) { }
      ~FetchJobPrivate() { }

      void parseBodyStructure( const QByteArray &structure, int &pos, KMime::Content *content );
      void parsePart( const QByteArray &structure, int &pos, KMime::Content *content );
      QByteArray parseString( const QByteArray &structure, int &pos );
      QByteArray parseSentence( const QByteArray &structure, int &pos );
      void skipLeadingSpaces( const QByteArray &structure, int &pos );

      QSharedPointer<KMime::Message> message(int id)
      {
        if ( !messages.contains(id) ) {
          messages[id] = QSharedPointer<KMime::Message>(new KMime::Message);
        }

        return messages[id];
      }

      QSharedPointer<KMime::Content> part(int id, QByteArray partName)
      {
        if ( !parts[id].contains(partName) ) {
          parts[id][partName] = QSharedPointer<KMime::Content>(new KMime::Content);
        }

        return parts[id][partName];
      }

      QByteArray tag;
      QByteArray set;
      FetchJob::FetchScope scope;

      QMap<int, QSharedPointer<KMime::Message> > messages;
      QMap<int, QMap<QByteArray, QSharedPointer<KMime::Content> > > parts;
      QMap<int, QList<QByteArray> > flags;
      QMap<int, qint64> sizes;

  };
}

using namespace KIMAP;

FetchJob::FetchJob( Session *session )
  : Job( *new FetchJobPrivate(session) )
{
  Q_D(FetchJob);
  d->scope.mode = FetchScope::Content;
}

FetchJob::~FetchJob()
{
}

void FetchJob::setSequenceSet( const QByteArray &set )
{
  Q_D(FetchJob);
  d->set = set;
}

QByteArray FetchJob::sequenceSet() const
{
  Q_D(const FetchJob);
  return d->set;
}

void FetchJob::setScope( const FetchScope &scope )
{
  Q_D(FetchJob);
  d->scope = scope;
}

FetchJob::FetchScope FetchJob::scope() const
{
  Q_D(const FetchJob);
  return d->scope;
}

QMap<int, QSharedPointer<KMime::Message> > FetchJob::messages() const
{
  Q_D(const FetchJob);
  return d->messages;
}

QMap<int, QMap<QByteArray, QSharedPointer<KMime::Content> > > FetchJob::parts() const
{
  Q_D(const FetchJob);
  return d->parts;
}

QMap<int, QList<QByteArray> > FetchJob::flags() const
{
  Q_D(const FetchJob);
  return d->flags;
}

QMap<int, qint64> FetchJob::sizes() const
{
  Q_D(const FetchJob);
  return d->sizes;
}

void FetchJob::doStart()
{
  Q_D(FetchJob);

  QByteArray parameters = d->set+' ';

  switch ( d->scope.mode ) {
  case FetchScope::Headers:
    if ( d->scope.parts.isEmpty() ) {
      parameters+="(RFC822.SIZE INTERNALDATE BODY[HEADER.FIELDS (TO FROM MESSAGE-ID REFERENCES IN-REPLY-TO SUBJECT)])";
    } else {
      parameters+='(';
      foreach ( const QByteArray &part, d->scope.parts ) {
        parameters+="BODY["+part+".MIME] ";
      }
      parameters.chop(1);
      parameters+=')';
    }
    break;
  case FetchScope::Flags:
    parameters+="FLAGS";
    break;
  case FetchScope::Structure:
    parameters+="BODYSTRUCTURE";
    break;
  case FetchScope::Content:
    if ( d->scope.parts.isEmpty() ) {
      parameters+="BODY[]";
    } else {
      parameters+='(';
      foreach ( const QByteArray &part, d->scope.parts ) {
        parameters+="BODY["+part+"] ";
      }
      parameters.chop(1);
      parameters+=')';
    }
    break;
  }

  d->tag = d->sessionInternal()->sendCommand( "FETCH", parameters );
}

void FetchJob::doHandleResponse( const Message &response )
{
  Q_D(FetchJob);

  if ( !response.content.isEmpty()
    && response.content.first().toString()==d->tag ) {
    if ( response.content.size() < 2 ) {
      setErrorText( i18n("Fetch failed, malformed reply from the server") );
    } else if ( response.content[1].toString()!="OK" ) {
      setError( UserDefinedError );
      setErrorText( i18n("Fetch failed, server replied: %1", response.toString().constData()) );
    }

    emitResult();
  } else if ( response.content.size() == 4
           && response.content[2].toString()=="FETCH"
           && response.content[3].type()==Message::Part::List ) {

    int id = response.content[1].toString().toInt();
    QList<QByteArray> content = response.content[3].toList();

    for ( QList<QByteArray>::ConstIterator it = content.begin();
          it!=content.end(); ++it ) {
      QByteArray str = *it;
      ++it;

      if ( str=="RFC822.SIZE" ) {
        d->sizes[id] = it->toInt();
      } else if ( str=="INTERNALDATE" ) {
        d->message(id)->date()->setDateTime( KDateTime::fromString( *it, KDateTime::RFCDate ) );
      } else if ( str=="FLAGS" ) {
        if ( (*it).startsWith('(') && (*it).endsWith(')') ) {
          QByteArray str = *it;
          str.chop(1);
          str.remove(0, 1);
          d->flags[id] = str.split(' ');
        } else {
          d->flags[id] << *it;
        }
      } else if ( str=="BODYSTRUCTURE" ) {
        int pos = 0;
        d->parseBodyStructure(*it, pos, d->message(id).data());
        d->message(id)->assemble();
      } else if ( str.startsWith("BODY[") ) {
        if ( !str.endsWith(']') ) { // BODY[ ... ] might have been split, skip until we find the ]
          while ( !(*it).endsWith(']') ) ++it;
          ++it;
        }

        int index;
        if ( (index=str.indexOf("HEADER"))>0 || (index=str.indexOf("MIME"))>0 ) { // headers
          if ( str[index-1]=='.' ) {
            QByteArray partId = str.mid( 5, index-6 );
            d->part( id, partId )->setHead(*it);
            d->part( id, partId )->parse();
          } else {
            d->message(id)->setHead(*it);
            d->message(id)->parse();
          }
        } else { // full payload
          if ( str=="BODY[]" ) {
            d->message(id)->setContent(*it);
            d->message(id)->parse();
          } else {
            QByteArray partId = str.mid( 5, str.size()-6 );
            d->part( id, partId )->setBody(*it);
            d->part( id, partId )->parse();
          }
        }
      }
    }
  }
}

void FetchJobPrivate::parseBodyStructure(const QByteArray &structure, int &pos, KMime::Content *content)
{
  skipLeadingSpaces(structure, pos);

  if ( structure[pos]!='(' ) {
    return;
  }

  pos++;


  if ( structure[pos]!='(' ) { // simple part
    pos--;
    parsePart( structure, pos, content );
  } else { // multi part
    content->contentType()->setMimeType("MULTIPART/MIXED");
    while ( pos<structure.size() && structure[pos]=='(' ) {
      KMime::Content *child = new KMime::Content;
      content->addContent( child );
      parseBodyStructure( structure, pos, child );
      child->assemble();
    }

    QByteArray subType = parseString( structure, pos );
    content->contentType()->setMimeType( "MULTIPART/"+subType );

    parseSentence( structure, pos ); // Ditch the parameters... FIXME: Read it to get charset and name

    QByteArray disposition = parseSentence( structure, pos );
    if ( disposition.contains("INLINE") ) {
      content->contentDisposition()->setDisposition( KMime::Headers::CDinline );
    } else if ( disposition.contains("ATTACHMENT") ) {
      content->contentDisposition()->setDisposition( KMime::Headers::CDattachment );
    }

    parseSentence( structure, pos ); // Ditch the body language
  }

  // Consume what's left
  while ( pos<structure.size() && structure[pos]!=')' ) {
    skipLeadingSpaces( structure, pos );
    parseSentence( structure, pos );
    skipLeadingSpaces( structure, pos );
  }

  pos++;
}

void FetchJobPrivate::parsePart( const QByteArray &structure, int &pos, KMime::Content *content )
{
  if ( structure[pos]!='(' ) {
    return;
  }

  pos++;

  QByteArray mainType = parseString( structure, pos );
  QByteArray subType = parseString( structure, pos );

  content->contentType()->setMimeType( mainType+'/'+subType );

  parseSentence( structure, pos ); // Ditch the parameters... FIXME: Read it to get charset and name
  parseString( structure, pos ); // ... and the id

  content->contentDescription()->from7BitString( parseString( structure, pos ) );

  parseString( structure, pos ); // Ditch the encoding too
  parseString( structure, pos ); // ... and the size
  if ( mainType=="TEXT" ) {
    parseString( structure, pos ); // ... and the line count
  }

  QByteArray disposition = parseSentence( structure, pos );
  if ( disposition.contains("INLINE") ) {
    content->contentDisposition()->setDisposition( KMime::Headers::CDinline );
  } else if ( disposition.contains("ATTACHMENT") ) {
    content->contentDisposition()->setDisposition( KMime::Headers::CDattachment );
  }

  // Consume what's left
  while ( pos<structure.size() && structure[pos]!=')' ) {
    skipLeadingSpaces( structure, pos );
    parseSentence( structure, pos );
    skipLeadingSpaces( structure, pos );
  }
}

QByteArray FetchJobPrivate::parseSentence( const QByteArray &structure, int &pos )
{
  QByteArray result;
  int stack = 0;

  skipLeadingSpaces( structure, pos );

  if ( structure[pos]!='(' ) {
    return parseString( structure, pos );
  }

  int start = pos;

  do {
    switch ( structure[pos] ) {
    case '(':
      pos++;
      stack++;
      break;
    case ')':
      pos++;
      stack--;
      break;
    case '[':
      pos++;
      stack++;
      break;
    case ']':
      pos++;
      stack--;
      break;
    default:
      skipLeadingSpaces(structure, pos);
      parseString(structure, pos);
      skipLeadingSpaces(structure, pos);
      break;
    }
  } while ( pos<structure.size() && stack!=0 );

  result = structure.mid( start, pos - start );

  return result;
}

QByteArray FetchJobPrivate::parseString( const QByteArray &structure, int &pos )
{
  QByteArray result;

  skipLeadingSpaces( structure, pos );

  int start = pos;
  bool foundSlash = false;

  // quoted string
  if ( structure[pos] == '"' ) {
    pos++;
    Q_FOREVER {
      if ( structure[pos] == '\\' ) {
        pos+= 2;
        foundSlash = true;
        continue;
      }
      if ( structure[pos] == '"' ) {
        result = structure.mid( start+1, pos - start );
        pos++;
        break;
      }
      pos++;
    }
  } else { // unquoted string
    Q_FOREVER {
      if ( structure[pos] == ' ' || structure[pos] == '(' || structure[pos] == ')' || structure[pos] == '[' || structure[pos] == ']' || structure[pos] == '\n' || structure[pos] == '\r' || structure[pos] == '"') {
        break;
      }
      if (structure[pos] == '\\')
        foundSlash = true;
      pos++;
    }

    result = structure.mid( start, pos - start );

    // transform unquoted NIL
    if ( result == "NIL" )
      result.clear();
  }

  // simplify slashes
  if ( foundSlash ) {
    while ( result.contains( "\\\"" ) )
      result.replace( "\\\"", "\"" );
    while ( result.contains( "\\\\" ) )
      result.replace( "\\\\", "\\" );
  }

  return result;
}

void FetchJobPrivate::skipLeadingSpaces( const QByteArray &structure, int &pos )
{
  while ( structure[pos]==' ' && pos<structure.size() ) pos++;
}

#include "fetchjob.moc"
