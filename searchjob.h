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

#ifndef KIMAP_SEARCHJOB_H
#define KIMAP_SEARCHJOB_H

#include "kimap_export.h"

#include "job.h"

class QDate;

namespace KIMAP {

class Session;
struct Message;
class SearchJobPrivate;

class KIMAP_EXPORT SearchJob : public Job
{
  Q_OBJECT
  Q_DECLARE_PRIVATE(SearchJob)

  friend class SessionPrivate;

  public:
    enum SearchLogic {
      And = 0,
      Or,
      Not
    };

    enum SearchCriteria {
       All = 0,
       Answered,
       BCC,
       Before,
       Body,
       CC,
       Deleted,
       Draft,
       Flagged,
       From,
       Header,
       Keyword,
       Larger,
       New,
       Old,
       On,
       Recent,
       Seen,
       SentBefore,
       SentOn,
       SentSince,
       Since,
       Smaller,
       Subject,
       Text,
       To,
       Uid,
       Unanswered,
       Undeleted,
       Undraft,
       Unflagged,
       Unkeyword,
       Unseen
    };

    explicit SearchJob( Session *session );
    virtual ~SearchJob();

    void setUidBased(bool uidBased);
    bool isUidBased() const;

    void setCharset( const QByteArray &charSet );
    QByteArray charset() const;

    /**
     * Get the search result, as a list of sequence numbers or UIDs, based on the isUidBased status
     * @return the found items
     */
    QList<int> foundItems();

    /**
     * Add a search criteria that doesn't have an argument. Passing a criteria that
     * should have an argument will be ignored.
     * @param criteria a criteria from SearchCriterias
     */
    void addSearchCriteria( SearchCriteria criteria );

    /**
     * Add a search criteria that has one or more space separate string arguments.
     * Passing a criteria that accepts a different type or argument or no
     * argument will be ignored.
     * @param criteria a criteria from SearchCriterias
     * @param argument the arguments
     */
    void addSearchCriteria( SearchCriteria criteria, const QByteArray &argument );

    /**
     * Add a search criteria that has an integer argument.
     * Passing a criteria that accepts a different type or argument or no
     * argument will be ignored.
     * @param criteria a criteria from SearchCriterias
     * @param argument a number argument
     */
    void addSearchCriteria( SearchCriteria criteria, int argument );

    /**
     * Add a search criteria that has a date as argument.
     * Passing a criteria that accepts a different type or argument or no
     * argument will be ignored.
     * @param criteria a criteria from SearchCriterias
     * @param argument a date
     */
    void addSearchCriteria( SearchCriteria criteria, const QDate& argument );

    /**
     * Add a custom criteria. No checks are done, the data is sent as it is
     * to the server.
     * @param searchCriteria free form search criteria.
     */
    void addSearchCriteria( const QByteArray &searchCriteria );

    /**
     * Set the logic combining the search criterias.
     * @param logic AND (the default), OR, NOT. See SearchLogics.
     */
    void setSearchLogic(SearchLogic logic);

  protected:
    virtual void doStart();
    virtual void handleResponse(const Message &response);
};

}

#endif
