/*
    SPDX-FileCopyrightText: 2009 Andras Mantia <amantia@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "searchjob.h"

#include "kimap_debug.h"
#include <KLocalizedString>

#include <QDate>

#include "imapset.h"
#include "job_p.h"
#include "response_p.h"
#include "session_p.h"

namespace KIMAP
{
class TermPrivate : public QSharedData
{
public:
    QByteArray command;
    bool isFuzzy = false;
    bool isNegated = false;
    bool isNull = false;
};

Term::Term()
    : d(new TermPrivate)
{
    d->isNull = true;
}

Term::Term(Term::Relation relation, const QList<Term> &subterms)
    : d(new TermPrivate)
{
    if (subterms.size() >= 2) {
        if (relation == KIMAP::Term::Or) {
            for (int i = 0; i < subterms.size() - 1; ++i) {
                d->command += "(OR " + subterms[i].serialize() + " ";
            }
            d->command += subterms.back().serialize();
            for (int i = 0; i < subterms.size() - 1; ++i) {
                d->command += ")";
            }
        } else {
            d->command += "(";
            for (const Term &t : subterms) {
                d->command += t.serialize() + ' ';
            }
            if (!subterms.isEmpty()) {
                d->command.chop(1);
            }
            d->command += ")";
        }
    } else if (subterms.size() == 1) {
        d->command += subterms.first().serialize();
    } else {
        d->isNull = true;
    }
}

Term::Term(Term::SearchKey key, const QString &value)
    : d(new TermPrivate)
{
    switch (key) {
    case All:
        d->command += "ALL";
        break;
    case Bcc:
        d->command += "BCC";
        break;
    case Cc:
        d->command += "CC";
        break;
    case Body:
        d->command += "BODY";
        break;
    case From:
        d->command += "FROM";
        break;
    case Keyword:
        d->command += "KEYWORD";
        break;
    case Subject:
        d->command += "SUBJECT";
        break;
    case Text:
        d->command += "TEXT";
        break;
    case To:
        d->command += "TO";
        break;
    }
    if (key != All) {
        d->command += " \"" + QByteArray(value.toUtf8().constData()) + "\"";
    }
}

Term::Term(const QString &header, const QString &value)
    : d(new TermPrivate)
{
    d->command += "HEADER";
    d->command += ' ' + QByteArray(header.toUtf8().constData());
    d->command += " \"" + QByteArray(value.toUtf8().constData()) + "\"";
}

Term::Term(Term::BooleanSearchKey key)
    : d(new TermPrivate)
{
    switch (key) {
    case Answered:
        d->command = "ANSWERED";
        break;
    case Deleted:
        d->command = "DELETED";
        break;
    case Draft:
        d->command = "DRAFT";
        break;
    case Flagged:
        d->command = "FLAGGED";
        break;
    case New:
        d->command = "NEW";
        break;
    case Old:
        d->command = "OLD";
        break;
    case Recent:
        d->command = "RECENT";
        break;
    case Seen:
        d->command = "SEEN";
        break;
    }
}

static QByteArrayView monthName(int month)
{
    static const char names[12][4] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
    return (month >= 1 && month <= 12) ? QByteArrayView(names[month - 1]) : QByteArrayView();
}

Term::Term(Term::DateSearchKey key, const QDate &date)
    : d(new TermPrivate)
{
    switch (key) {
    case Before:
        d->command = "BEFORE";
        break;
    case On:
        d->command = "ON";
        break;
    case SentBefore:
        d->command = "SENTBEFORE";
        break;
    case SentOn:
        d->command = "SENTON";
        break;
    case SentSince:
        d->command = "SENTSINCE";
        break;
    case Since:
        d->command = "SINCE";
        break;
    }
    d->command += " \"";
    d->command += QByteArray::number(date.day()) + '-';
    d->command += monthName(date.month()) + '-';
    d->command += QByteArray::number(date.year());
    d->command += '\"';
}

Term::Term(Term::NumberSearchKey key, int value)
    : d(new TermPrivate)
{
    switch (key) {
    case Larger:
        d->command = "LARGER";
        break;
    case Smaller:
        d->command = "SMALLER";
        break;
    }
    d->command += " " + QByteArray::number(value);
}

Term::Term(Term::SequenceSearchKey key, const ImapSet &set)
    : d(new TermPrivate)
{
    switch (key) {
    case Uid:
        d->command = "UID";
        break;
    case SequenceNumber:
        break;
    }
    auto optimizedSet = set;
    optimizedSet.optimize();
    d->command += " " + optimizedSet.toImapSequenceSet();
}

Term::Term(const Term &other) = default;

Term::~Term() = default;

Term &Term::operator=(const Term &other) = default;

bool Term::operator==(const Term &other) const
{
    return d->command == other.d->command && d->isNegated == other.d->isNegated && d->isFuzzy == other.d->isFuzzy;
}

QByteArray Term::serialize() const
{
    QByteArray command;
    if (d->isFuzzy) {
        command = "FUZZY ";
    }
    if (d->isNegated) {
        command = "NOT ";
    }
    return command + d->command;
}

Term &Term::setFuzzy(bool fuzzy)
{
    d->isFuzzy = fuzzy;
    return *this;
}

Term &Term::setNegated(bool negated)
{
    d->isNegated = negated;
    return *this;
}

bool Term::isNull() const
{
    return d->isNull;
}

// TODO: when custom error codes are introduced, handle the NO [TRYCREATE] response

class SearchJobPrivate : public JobPrivate
{
public:
    SearchJobPrivate(Session *session, const QString &name)
        : JobPrivate(session, name)
    {
        criteriaMap[SearchJob::All] = "ALL";
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

        // don't use QDate::shortMonthName(), it returns a localized month name
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

    QByteArray charset;
    QList<QByteArray> criteria;
    QMap<SearchJob::SearchCriteria, QByteArray> criteriaMap;
    QMap<int, QByteArray> months;
    SearchJob::SearchLogic logic = SearchJob::And;
    QList<QByteArray> contents;
    QList<qint64> results;
    uint nextContent;
    bool uidBased;
    Term term;
};
}

using namespace KIMAP;

SearchJob::SearchJob(Session *session)
    : Job(*new SearchJobPrivate(session, i18nc("Name of the search job", "Search")))
{
}

SearchJob::~SearchJob() = default;

void SearchJob::setTerm(const Term &term)
{
    Q_D(SearchJob);
    d->term = term;
}

void SearchJob::doStart()
{
    Q_D(SearchJob);

    QByteArray searchKey;

    if (!d->charset.isEmpty()) {
        searchKey = "CHARSET " + d->charset;
    }

    if (!d->term.isNull()) {
        const QByteArray term = d->term.serialize();
        if (term.startsWith('(')) {
            searchKey += term.mid(1, term.size() - 2);
        } else {
            searchKey += term;
        }
    } else {
        if (d->logic == SearchJob::Not) {
            searchKey += "NOT ";
        } else if (d->logic == SearchJob::Or && d->criteria.size() > 1) {
            searchKey += "OR ";
        }

        if (d->logic == SearchJob::And) {
            const auto numberCriterias(d->criteria.size());
            for (int i = 0; i < numberCriterias; i++) {
                const QByteArray key = d->criteria.at(i);
                if (i > 0) {
                    searchKey += ' ';
                }
                searchKey += key;
            }
        } else {
            const auto numberCriterias(d->criteria.size());
            for (int i = 0; i < numberCriterias; i++) {
                const QByteArray key = d->criteria.at(i);
                if (i > 0) {
                    searchKey += ' ';
                }
                searchKey += '(' + key + ')';
            }
        }
    }

    QByteArray command = "SEARCH";
    if (d->uidBased) {
        command = "UID " + command;
    }

    d->tags << d->sessionInternal()->sendCommand(command, searchKey);
}

void SearchJob::handleResponse(const Response &response)
{
    Q_D(SearchJob);

    if (handleErrorReplies(response) == NotHandled) {
        if (response.content.size() >= 1 && response.content[0].toString() == "+") {
            if (d->term.isNull()) {
                d->sessionInternal()->sendData(d->contents[d->nextContent]);
            } else {
                qCWarning(KIMAP_LOG) << "The term API only supports inline strings.";
            }
            d->nextContent++;
        } else if (response.content.size() >= 2 && response.content[1].toString() == "SEARCH") {
            for (int i = 2; i < response.content.size(); i++) {
                d->results.append(response.content[i].toString().toInt());
            }
        }
    }
}

void SearchJob::setCharset(const QByteArray &charset)
{
    Q_D(SearchJob);
    d->charset = charset;
}

QByteArray SearchJob::charset() const
{
    Q_D(const SearchJob);
    return d->charset;
}

void SearchJob::setSearchLogic(SearchLogic logic)
{
    Q_D(SearchJob);
    d->logic = logic;
}

void SearchJob::addSearchCriteria(SearchCriteria criteria)
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
        d->criteria.append(d->criteriaMap[criteria]);
        break;
    default:
        // TODO Discuss if we keep error checking here, or accept anything, even if it is wrong
        qCDebug(KIMAP_LOG) << "Criteria" << d->criteriaMap[criteria] << "needs an argument, but none was specified.";
        break;
    }
}

void SearchJob::addSearchCriteria(SearchCriteria criteria, int argument)
{
    Q_D(SearchJob);
    switch (criteria) {
    case Larger:
    case Smaller:
        d->criteria.append(d->criteriaMap[criteria] + ' ' + QByteArray::number(argument));
        break;
    default:
        // TODO Discuss if we keep error checking here, or accept anything, even if it is wrong
        qCDebug(KIMAP_LOG) << "Criteria" << d->criteriaMap[criteria] << "doesn't accept an integer as an argument.";
        break;
    }
}

void SearchJob::addSearchCriteria(SearchCriteria criteria, const QByteArray &argument)
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
        d->criteria.append(d->criteriaMap[criteria] + " {" + QByteArray::number(argument.size()) + '}');
        break;
    case Keyword:
    case Unkeyword:
    case Header:
    case Uid:
        d->criteria.append(d->criteriaMap[criteria] + ' ' + argument);
        break;
    default:
        // TODO Discuss if we keep error checking here, or accept anything, even if it is wrong
        qCDebug(KIMAP_LOG) << "Criteria" << d->criteriaMap[criteria] << "doesn't accept any argument.";
        break;
    }
}

void SearchJob::addSearchCriteria(SearchCriteria criteria, const QDate &argument)
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
        d->criteria.append(d->criteriaMap[criteria] + " \"" + date + '\"');
        break;
    }
    default:
        // TODO Discuss if we keep error checking here, or accept anything, even if it is wrong
        qCDebug(KIMAP_LOG) << "Criteria" << d->criteriaMap[criteria] << "doesn't accept a date as argument.";
        break;
    }
}

void SearchJob::addSearchCriteria(const QByteArray &searchCriteria)
{
    Q_D(SearchJob);
    d->criteria.append(searchCriteria);
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

QList<qint64> SearchJob::results() const
{
    Q_D(const SearchJob);
    return d->results;
}

#include "moc_searchjob.cpp"
