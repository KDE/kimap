/*
    SPDX-FileCopyrightText: 2009 Andras Mantia <amantia@kde.org>
    SPDX-FileCopyrightText: 2014 Christian Mollekopf <mollekopf@kolabsys.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "kimap_export.h"

#include "job.h"

#include <QSharedDataPointer>

class QDate;

namespace KIMAP
{
class ImapSet;

class Session;
struct Response;
class SearchJobPrivate;

/**
 * A query term.
 * Refer to the IMAP RFC for the meaning of the individual terms.
 * @since 4.13
 */
class KIMAP_EXPORT Term
{
public:
    enum Relation { And, Or };

    enum SearchKey { All, Bcc, Body, Cc, From, Subject, Text, To, Keyword };

    enum BooleanSearchKey { New, Old, Recent, Seen, Draft, Deleted, Flagged, Answered };

    enum DateSearchKey { Before, On, Since, SentBefore, SentOn, SentSince };
    enum NumberSearchKey { Larger, Smaller };
    enum SequenceSearchKey { Uid, SequenceNumber };

    Term();
    ~Term();
    Term(Relation relation, const QVector<Term> &subterms);
    Term(SearchKey key, const QString &value);
    Term(BooleanSearchKey key);
    Term(DateSearchKey key, const QDate &date);
    Term(NumberSearchKey key, int value);
    Term(SequenceSearchKey key, const KIMAP::ImapSet &);
    Term(const QString &header, const QString &value);

    Term(const Term &other);

    Term &operator=(const Term &other);
    bool operator==(const Term &other) const;

    Q_REQUIRED_RESULT bool isNull() const;

    Term &setFuzzy(bool fuzzy);
    Term &setNegated(bool negated);

    Q_REQUIRED_RESULT QByteArray serialize() const;

private:
    class Private;
    QSharedDataPointer<Private> d;
};

class KIMAP_EXPORT SearchJob : public Job
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(SearchJob)

    friend class SessionPrivate;

public:
    enum SearchLogic { And = 0, Or, Not };

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

    explicit SearchJob(Session *session);
    ~SearchJob() override;

    void setUidBased(bool uidBased);
    bool isUidBased() const;

    void setCharset(const QByteArray &charSet);
    QByteArray charset() const;

    /**
     * Get the search result, as a list of sequence numbers or UIDs, based on the isUidBased status
     * @return the found items
     * @since 4.6
     */
    QVector<qint64> results() const;

    /**
     * Add a search criteria that doesn't have an argument. Passing a criteria that
     * should have an argument will be ignored.
     * @param criteria a criteria from SearchCriterias
     * @deprecated since 4.13
     */
    KIMAP_DEPRECATED void addSearchCriteria(SearchCriteria criteria);

    /**
     * Add a search criteria that has one or more space separate string arguments.
     * Passing a criteria that accepts a different type or argument or no
     * argument will be ignored.
     * @param criteria a criteria from SearchCriterias
     * @param argument the arguments
     * @deprecated since 4.13
     */
    KIMAP_DEPRECATED void addSearchCriteria(SearchCriteria criteria, const QByteArray &argument);

    /**
     * Add a search criteria that has an integer argument.
     * Passing a criteria that accepts a different type or argument or no
     * argument will be ignored.
     * @param criteria a criteria from SearchCriterias
     * @param argument a number argument
     * @deprecated since 4.13
     */
    KIMAP_DEPRECATED void addSearchCriteria(SearchCriteria criteria, int argument);

    /**
     * Add a search criteria that has a date as argument.
     * Passing a criteria that accepts a different type or argument or no
     * argument will be ignored.
     * @param criteria a criteria from SearchCriterias
     * @param argument a date
     * @deprecated since 4.13
     */
    KIMAP_DEPRECATED void addSearchCriteria(SearchCriteria criteria, const QDate &argument);

    /**
     * Add a custom criteria. No checks are done, the data is sent as it is
     * to the server.
     * @param searchCriteria free form search criteria.
     * @deprecated since 4.13
     */
    KIMAP_DEPRECATED void addSearchCriteria(const QByteArray &searchCriteria);

    /**
     * Set the logic combining the search criterias.
     * @param logic AND (the default), OR, NOT. See SearchLogics.
     * @deprecated since 4.13
     */
    KIMAP_DEPRECATED void setSearchLogic(SearchLogic logic);

    /**
     * Sets the search term.
     * @param term The search term.
     * @since 4.13
     */
    void setTerm(const Term &);

protected:
    void doStart() override;
    void handleResponse(const Response &response) override;
};

}

