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
class TermPrivate;

/*!
 * \class KIMAP::Term
 * \inmodule KIMAP
 * \inheaderfile KIMAP/Term
 *
 * A query term.
 * Refer to the IMAP RFC for the meaning of the individual terms.
 * \since 4.13
 */
class KIMAP_EXPORT Term
{
public:
    enum Relation {
        And,
        Or
    };

    enum SearchKey {
        All,
        Bcc,
        Body,
        Cc,
        From,
        Subject,
        Text,
        To,
        Keyword
    };

    enum BooleanSearchKey {
        New,
        Old,
        Recent,
        Seen,
        Draft,
        Deleted,
        Flagged,
        Answered
    };

    enum DateSearchKey {
        Before,
        On,
        Since,
        SentBefore,
        SentOn,
        SentSince
    };
    enum NumberSearchKey {
        Larger,
        Smaller
    };
    enum SequenceSearchKey {
        Uid,
        SequenceNumber
    };

    Term();
    ~Term();
    Term(Relation relation, const QList<Term> &subterms);
    Term(SearchKey key, const QString &value);
    Term(BooleanSearchKey key);
    Term(DateSearchKey key, const QDate &date);
    Term(NumberSearchKey key, int value);
    Term(SequenceSearchKey key, const KIMAP::ImapSet &);
    Term(const QString &header, const QString &value);

    Term(const Term &other);

    Term &operator=(const Term &other);
    bool operator==(const Term &other) const;

    [[nodiscard]] bool isNull() const;

    Term &setFuzzy(bool fuzzy);
    Term &setNegated(bool negated);

    [[nodiscard]] QByteArray serialize() const;

private:
    QSharedDataPointer<TermPrivate> d;
};

class KIMAP_EXPORT SearchJob : public Job
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(SearchJob)

    friend class SessionPrivate;

public:
    explicit SearchJob(Session *session);
    ~SearchJob() override;

    void setUidBased(bool uidBased);
    [[nodiscard]] bool isUidBased() const;

    void setCharset(const QByteArray &charSet);
    [[nodiscard]] QByteArray charset() const;

    /*!
     * Get the search result, as a list of sequence numbers or UIDs, based on the isUidBased status
     * Returns the found items
     * \since 4.6
     */
    [[nodiscard]] QList<qint64> results() const;

    /*!
     * Sets the search term.
     * \a term The search term.
     * \since 4.13
     */
    void setTerm(const Term &);

protected:
    void doStart() override;
    void handleResponse(const Response &response) override;
};

}
