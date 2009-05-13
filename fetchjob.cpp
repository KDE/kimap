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

#include <QtCore/QTimer>
#include <KDE/KLocale>

#include "job_p.h"
#include "message_p.h"
#include "session_p.h"

namespace KIMAP
{
  class FetchJobPrivate : public JobPrivate
  {
    public:
      FetchJobPrivate( FetchJob *job, Session *session, const QString& name ) : JobPrivate( session, name ), q(job), uidBased(false) { }
      ~FetchJobPrivate() { }

      void parseBodyStructure( const QByteArray &structure, int &pos, KMime::Content *content );
      void parsePart( const QByteArray &structure, int &pos, KMime::Content *content );
      QByteArray parseString( const QByteArray &structure, int &pos );
      QByteArray parseSentence( const QByteArray &structure, int &pos );
      void skipLeadingSpaces( const QByteArray &structure, int &pos );

      MessagePtr message(int id)
      {
        if ( !messages.contains(id) ) {
          messages[id] = MessagePtr(new KMime::Message);
        }

        return messages[id];
      }

      ContentPtr part(int id, QByteArray partName)
      {
        if ( !parts[id].contains(partName) ) {
          parts[id][partName] = ContentPtr(new KMime::Content);
        }

        return parts[id][partName];
      }

      void emitPendings()
      {
        if ( pendingUids.isEmpty() ) {
          return;
        }

        if ( !pendingParts.isEmpty() ) {
          emit q->partsReceived( sessionInternal()->selectedMailBox(),
                                 pendingUids, pendingParts );

        } else if ( !pendingSizes.isEmpty() || !pendingFlags.isEmpty() ) {
          emit q->headersReceived( sessionInternal()->selectedMailBox(),
                                   pendingUids, pendingSizes,
                                   pendingFlags, pendingMessages );
        } else {
          emit q->messagesReceived( sessionInternal()->selectedMailBox(),
                                    pendingUids, pendingMessages );
        }

        pendingUids.clear();
        pendingMessages.clear();
        pendingParts.clear();
        pendingSizes.clear();
        pendingFlags.clear();
      }

      FetchJob * const q;

      ImapSet set;
      bool uidBased;
      FetchJob::FetchScope scope;

      QMap<qint64, MessagePtr> messages;
      QMap<qint64, MessageParts> parts;
      QMap<qint64, MessageFlags> flags;
      QMap<qint64, qint64> sizes;
      QMap<qint64, qint64> uids;

      QTimer emitPendingsTimer;
      QMap<qint64, MessagePtr> pendingMessages;
      QMap<qint64, MessageParts> pendingParts;
      QMap<qint64, MessageFlags> pendingFlags;
      QMap<qint64, qint64> pendingSizes;
      QMap<qint64, qint64> pendingUids;
  };
}

using namespace KIMAP;

FetchJob::FetchJob( Session *session )
  : Job( *new FetchJobPrivate(this, session, i18n("Fetch")) )
{
  Q_D(FetchJob);
  d->scope.mode = FetchScope::Content;
  connect( &d->emitPendingsTimer, SIGNAL( timeout() ),
           this, SLOT( emitPendings() ) );
}

FetchJob::~FetchJob()
{
}

void FetchJob::setSequenceSet( const ImapSet &set )
{
  Q_D(FetchJob);
  d->set = set;
}

ImapSet FetchJob::sequenceSet() const
{
  Q_D(const FetchJob);
  return d->set;
}

void FetchJob::setUidBased(bool uidBased)
{
  Q_D(FetchJob);
  d->uidBased = uidBased;
}

bool FetchJob::isUidBased() const
{
  Q_D(const FetchJob);
  return d->uidBased;
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

QMap<qint64, MessagePtr> FetchJob::messages() const
{
  Q_D(const FetchJob);
  return d->messages;
}

QMap<qint64, MessageParts> FetchJob::parts() const
{
  Q_D(const FetchJob);
  return d->parts;
}

QMap<qint64, MessageFlags> FetchJob::flags() const
{
  Q_D(const FetchJob);
  return d->flags;
}

QMap<qint64, qint64> FetchJob::sizes() const
{
  Q_D(const FetchJob);
  return d->sizes;
}

QMap<qint64, qint64> FetchJob::uids() const
{
  Q_D(const FetchJob);
  return d->uids;
}

void FetchJob::doStart()
{
  Q_D(FetchJob);

  QByteArray parameters = d->set.toImapSequenceSet()+' ';

  switch ( d->scope.mode ) {
  case FetchScope::Headers:
    if ( d->scope.parts.isEmpty() ) {
      parameters+="(RFC822.SIZE INTERNALDATE BODY[HEADER.FIELDS (TO FROM MESSAGE-ID REFERENCES IN-REPLY-TO SUBJECT)] FLAGS UID)";
    } else {
      parameters+='(';
      foreach ( const QByteArray &part, d->scope.parts ) {
        parameters+="BODY["+part+".MIME] ";
      }
      parameters+="UID)";
    }
    break;
  case FetchScope::Flags:
    parameters+="(FLAGS UID)";
    break;
  case FetchScope::Structure:
    parameters+="(BODYSTRUCTURE UID)";
    break;
  case FetchScope::Content:
    if ( d->scope.parts.isEmpty() ) {
      parameters+="(BODY[] UID)";
    } else {
      parameters+='(';
      foreach ( const QByteArray &part, d->scope.parts ) {
        parameters+="BODY["+part+"] ";
      }
      parameters+="UID)";
    }
    break;
  }

  QByteArray command = "FETCH";
  if ( d->uidBased ) {
    command = "UID "+command;
  }

  d->emitPendingsTimer.start( 100 );
  d->tag = d->sessionInternal()->sendCommand( command, parameters );
}

void FetchJob::handleResponse( const Message &response )
{
  Q_D(FetchJob);

  if (handleErrorReplies(response) == NotHandled ) {
    if ( response.content.size() == 4
           && response.content[2].toString()=="FETCH"
           && response.content[3].type()==Message::Part::List ) {

      qint64 id = response.content[1].toString().toLongLong();
      QList<QByteArray> content = response.content[3].toList();

      for ( QList<QByteArray>::ConstIterator it = content.constBegin();
            it!=content.constEnd(); ++it ) {
        QByteArray str = *it;
        ++it;

        if ( str=="UID" ) {
          d->uids[id] = it->toLongLong();
        } else if ( str=="RFC822.SIZE" ) {
          d->sizes[id] = it->toLongLong();
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
          d->parseBodyStructure(*it, pos, d->message(id).get());
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
              d->message(id)->setContent( KMime::CRLFtoLF(*it) );
              d->message(id)->parse();

              d->pendingUids[id] = d->uids[id];
              d->pendingMessages[id] = d->message(id);
            } else {
              QByteArray partId = str.mid( 5, str.size()-6 );
              d->part( id, partId )->setBody(*it);
              d->part( id, partId )->parse();

              d->pendingUids[id] = d->uids[id];
              d->pendingParts[id] = d->parts[id];
            }
          }
        }
      }

      if ( d->scope.mode == FetchScope::Headers ) {
        d->pendingUids[id] = d->uids[id];
        d->pendingSizes[id] = d->sizes[id];
        d->pendingFlags[id] = d->flags[id];
        d->pendingMessages[id] = d->message(id);
      }
    }
  } else {
    d->emitPendingsTimer.stop();
    d->emitPendings();
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
