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

#include "searchjob.h"

#include <KDE/KLocale>
#include <KDE/KDebug>

#include <QtCore/QDate>

#include "job_p.h"
#include "message_p.h"
#include "session_p.h"

//TODO: when custom error codes are introduced, handle the NO [TRYCREATE] response

namespace KIMAP
{
  class SearchJobPrivate : public JobPrivate
  {
    public:
      SearchJobPrivate( Session *session, const QString& name ) : JobPrivate(session, name), logic(SearchJob::And) {
        criteriaMap[SearchJob::All]  = "ALL";
        criteriaMap[SearchJob::Answered] = "ANSWERED";
        criteriaMap[SearchJob::BCC] = "BCC";
        criteriaMap[SearchJob::Before] = "BEFORE";
        criteriaMap[SearchJob::Body] = "BODY";
        criteriaMap[SearchJob::CC] = "CC";
        criteriaMap[SearchJob::Deleted] = "DELETED";
        criteriaMap[SearchJob::Draft] = "DRAFT";
        criteriaMap[SearchJob::Flagged] = "FLAGGED";
        criteriaMap[SearchJob::From] = "FROM";
        criteriaMap[SearchJob::Header] = "HEADER";
        criteriaMap[SearchJob::Keyword] = "KEYWORD";
        criteriaMap[SearchJob::Larger] = "LARGER";
        criteriaMap[SearchJob::New] = "NEW";
        criteriaMap[SearchJob::Old] = "OLD";
        criteriaMap[SearchJob::On] = "ON";
        criteriaMap[SearchJob::Recent] = "RECENT";
        criteriaMap[SearchJob::Seen] = "SEEN";
        criteriaMap[SearchJob::SentBefore] = "SENTBEFORE";
        criteriaMap[SearchJob::SentOn] = "SENTON";
        criteriaMap[SearchJob::SentSince] = "SENTSINCE";
        criteriaMap[SearchJob::Since] = "SINCE";
        criteriaMap[SearchJob::Smaller] = "SMALLER";
        criteriaMap[SearchJob::Subject] = "SUBJECT";
        criteriaMap[SearchJob::Text] = "TEXT";
        criteriaMap[SearchJob::To] = "TO";
        criteriaMap[SearchJob::Uid] = "UID";
        criteriaMap[SearchJob::Unanswered] = "UNANSWERED";
        criteriaMap[SearchJob::Undeleted] = "UNDELETED";
        criteriaMap[SearchJob::Undraft] = "UNDRAFT";
        criteriaMap[SearchJob::Unflagged] = "UNFLAGGED";
        criteriaMap[SearchJob::Unkeyword] = "UNKEYWORD";
        criteriaMap[SearchJob::Unseen] = "UNSEEN";

        //don't use QDate::shortMonthName(), it returns a localized month name
        months[1] = "Jan";
        months[2] = "Feb";
        months[3] = "Mar";
        months[4] = "Apr";
        months[5] = "May";
        months[6] = "Jun";
        months[7] = "Jul";
        months[8] = "Aug";
        months[9] = "Sep";
        months[10] = "Oct";
        months[11] = "Nov";
        months[12] = "Dec";

        nextContent = 0;
        uidBased = false;    
      }
      ~SearchJobPrivate() { }


      QByteArray charset;
      QList<QByteArray> criterias;
      QMap<SearchJob::SearchCriteria, QByteArray > criteriaMap;
      QMap<int, QByteArray> months;
      SearchJob::SearchLogic logic;
      QList<QByteArray> contents;
      QList<int> results;
      uint nextContent;
      bool uidBased;
  };
}

using namespace KIMAP;

SearchJob::SearchJob( Session *session )
  : Job( *new SearchJobPrivate(session, i18n("Search")) )
{
}

SearchJob::~SearchJob()
{
}

void SearchJob::doStart()
{
  Q_D(SearchJob);

  QByteArray searchKey;

  if (!d->charset.isEmpty()) {
    searchKey = "[CHARSET] " + d->charset; 
  }

  if (d->logic == SearchJob::Not) {
    searchKey += "NOT";
  } else if (d->logic == SearchJob::Or) {
    searchKey += "OR";
  }
  Q_FOREACH(QByteArray key, d->criterias) {
    searchKey += " (" + key + ")";
  }

  QByteArray command = "SEARCH";
  if ( d->uidBased ) {
    command = "UID "+ command;
  }

  d->tag = d->sessionInternal()->sendCommand( command, searchKey );
}

void SearchJob::doHandleResponse( const Message &response )
{
  Q_D(SearchJob);

  if (handleErrorReplies(response) == NotHandled ) {
    if ( response.content[0].toString() == "+" ) {
      d->sessionInternal()->sendData( d->contents[d->nextContent] );
      d->nextContent++;
    } else if ( response.content[1].toString() == "SEARCH" ) {
      for(int i = 2; i < response.content.size(); i++) {
        d->results.append(response.content[i].toString().toInt());
      }          
    }
  }
}


void SearchJob::setCharset( const QByteArray &charset )
{
  Q_D(SearchJob);
  d->charset = charset;
}

QByteArray SearchJob::charset() const
{
  Q_D(const SearchJob);
  return d->charset;
}

void SearchJob::setSearchLogic( SearchLogic logic )
{
  Q_D(SearchJob);
  d->logic = logic;
}

void SearchJob::addSearchCriteria( SearchCriteria criteria ) 
{
  Q_D(SearchJob);

  switch (criteria) {
    case All:
    case Answered:
    case Deleted:
    case Draft:
    case Flagged:
    case New:
    case Old:
    case Recent:
    case Seen:
    case Unanswered:
    case Undeleted:
    case Undraft:
    case Unflagged:
    case Unseen:
      d->criterias.append(d->criteriaMap[criteria]);
      break;
    default:
      //TODO Discuss if we keep error checking here, or accept anything, even if it is wrong
      kDebug() << "Criteria " << d->criteriaMap[criteria] << " needs an argument, but none was specified.";
      break;
  }
}


void SearchJob::addSearchCriteria( SearchCriteria criteria, int argument ) 
{
  Q_D(SearchJob);
  switch (criteria) {
    case Larger:
    case Smaller:
      d->criterias.append(d->criteriaMap[criteria] + " " + QByteArray::number(argument));
      break;
    default:
      //TODO Discuss if we keep error checking here, or accept anything, even if it is wrong
      kDebug() << "Criteria " << d->criteriaMap[criteria] << " doesn't accept an integer as an argument.";
      break;
  }
}


void SearchJob::addSearchCriteria( SearchCriteria criteria, const QByteArray &argument ) 
{
  Q_D(SearchJob);
  switch (criteria) {
    case BCC:
    case Body:
    case CC:
    case From:
    case Subject:
    case Text:
    case To:
      d->contents.append(argument);
      d->criterias.append(d->criteriaMap[criteria] + " {" + QByteArray::number(argument.size()) + '}');
      break;
    case Keyword:
    case Unkeyword:
      d->criterias.append(d->criteriaMap[criteria] + ' ' + argument);
      break;
    case Header: {
      int pos = argument.indexOf(' ');
      QByteArray fieldName = argument.left(pos);
      QByteArray content = argument.mid(pos + 1);
      d->contents.append(content);
      d->criterias.append(d->criteriaMap[criteria] + ' ' + fieldName + " {" + QByteArray::number(content.size()) + '}');
      break;
    }
    default:
      //TODO Discuss if we keep error checking here, or accept anything, even if it is wrong
      kDebug() << "Criteria " << d->criteriaMap[criteria] << " doesn't accept any argument.";
      break;
  }
}

void SearchJob::addSearchCriteria( SearchCriteria criteria, const QDate &argument )
{
  Q_D(SearchJob);
  switch (criteria) {
    case Before:
    case On:
    case SentBefore:
    case SentSince:
    case Since: {
      QByteArray date = QByteArray::number(argument.day()) + '-';
      date += d->months[argument.month()] + '-';
      date += QByteArray::number(argument.year());
      d->criterias.append(d->criteriaMap[criteria] + " \"" + date + '\"');
      break;
    }
    default:
      //TODO Discuss if we keep error checking here, or accept anything, even if it is wrong
      kDebug() << "Criteria " << d->criteriaMap[criteria] << " doesn't accept a date as argument.";
      break;
  }
}

void SearchJob::addSearchCriteria( const QByteArray &searchCriteria ) 
{
  Q_D(SearchJob);
  d->criterias.append(searchCriteria);
}

void SearchJob::setUidBased(bool uidBased)
{
  Q_D(SearchJob);
  d->uidBased = uidBased;
}

bool SearchJob::isUidBased() const
{
  Q_D(const SearchJob);
  return d->uidBased;
}

QList<int> SearchJob::foundItems()
{
  Q_D(const SearchJob);
  return d->results;  
}
#include "searchjob.moc"
