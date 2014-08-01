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

#include <KLocalizedString>
#include <QDebug>

#include <QtCore/QDate>

#include "job_p.h"
#include "message_p.h"
#include "session_p.h"
#include "imapset.h"

namespace KIMAP
{

class Term::Private
{
public:
    Private(): isFuzzy(false), isNegated(false), isNull(false) {};
    QByteArray command;
    bool isFuzzy;
    bool isNegated;
    bool isNull;
};

Term::Term()
    :  d(new Term::Private)
{
    d->isNull = true;
}

Term::Term(Term::Relation relation, const QVector<Term> &subterms)
    :  d(new Term::Private)
{
    if (subterms.size() >= 2) {
        d->command += "(";
        if (relation == KIMAP::Term::Or) {
            d->command += "OR ";
            d->command += subterms.at(0).serialize() + ' ';
            if (subterms.size() >= 3) {
                Term t(relation, subterms.mid(1));
                d->command += t.serialize();
            } else if (subterms.size() == 2) {
                d->command += subterms.at(1).serialize();
            }
        } else {
            Q_FOREACH (const Term &t, subterms) {
                d->command += t.serialize() + ' ';
            }
            if (!subterms.isEmpty()) {
                d->command.chop(1);
            }
        }
        d->command += ")";
    } else if (subterms.size() == 1) {
        d->command += subterms.first().serialize();
    } else {
        d->isNull = true;
    }
}

Term::Term(Term::SearchKey key, const QString &value)
    :  d(new Term::Private)
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
    :  d(new Term::Private)
{
    d->command += "HEADER";
    d->command += ' ' + QByteArray(header.toUtf8().constData());
    d->command += " \"" + QByteArray(value.toUtf8().constData()) + "\"";
}

Term::Term(Term::BooleanSearchKey key)
    :  d(new Term::Private)
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

QMap<int, QByteArray> initializeMonths()
{
    QMap<int, QByteArray> months;
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
    return months;
}

static QMap<int, QByteArray> months = initializeMonths();

Term::Term(Term::DateSearchKey key, const QDate &date)
    :  d(new Term::Private)
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
    d->command += months[date.month()] + '-';
    d->command += QByteArray::number(date.year());
    d->command += '\"';
}

Term::Term(Term::NumberSearchKey key, int value)
    :  d(new Term::Private)
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
    :  d(new Term::Private)
{
    switch (key) {
    case Uid:
        d->command = "UID";
        break;
    case SequenceNumber:
        break;
    }
    d->command += " " + set.toImapSequenceSet();
}

Term::Term(const Term &other)
    :  d(new Term::Private)
{
    *d = *other.d;
}

Term &Term::operator=(const Term &other)
{
    *d = *other.d;
    return *this;
}

bool Term::operator==(const Term &other) const
{
    return d->command == other.d->command &&
           d->isNegated == other.d->isNegated &&
           d->isFuzzy == other.d->isFuzzy;
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

//TODO: when custom error codes are introduced, handle the NO [TRYCREATE] response

class SearchJobPrivate : public JobPrivate
{
public:
    SearchJobPrivate(Session *session, const QString &name) : JobPrivate(session, name), logic(SearchJob::And)
    {
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

SearchJob::~SearchJob()
{
}

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
        } else if (d->logic == SearchJob::Or && d->criterias.size() > 1) {
            searchKey += "OR ";
        }

        if (d->logic == SearchJob::And) {
            for (int i = 0; i < d->criterias.size(); i++) {
                const QByteArray key = d->criterias.at(i);
                if (i > 0) {
                    searchKey += ' ';
                }
                searchKey += key;
            }
        } else {
            for (int i = 0; i < d->criterias.size(); i++) {
                const QByteArray key = d->criterias.at(i);
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

void SearchJob::handleResponse(const Message &response)
{
    Q_D(SearchJob);

    if (handleErrorReplies(response) == NotHandled) {
        if (response.content[0].toString() == "+") {
            if (d->term.isNull()) {
                d->sessionInternal()->sendData(d->contents[d->nextContent]);
            } else {
                qWarning() << "The term API only supports inline strings.";
            }
            d->nextContent++;
        } else if (response.content[1].toString() == "SEARCH") {
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
        d->criterias.append(d->criteriaMap[criteria]);
        break;
    default:
        //TODO Discuss if we keep error checking here, or accept anything, even if it is wrong
        qDebug() << "Criteria " << d->criteriaMap[criteria] << " needs an argument, but none was specified.";
        break;
    }
}

void SearchJob::addSearchCriteria(SearchCriteria criteria, int argument)
{
    Q_D(SearchJob);
    switch (criteria) {
    case Larger:
    case Smaller:
        d->criterias.append(d->criteriaMap[criteria] + ' ' + QByteArray::number(argument));
        break;
    default:
        //TODO Discuss if we keep error checking here, or accept anything, even if it is wrong
        qDebug() << "Criteria " << d->criteriaMap[criteria] << " doesn't accept an integer as an argument.";
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
        d->criterias.append(d->criteriaMap[criteria] + " {" + QByteArray::number(argument.size()) + '}');
        break;
    case Keyword:
    case Unkeyword:
    case Header:
    case Uid:
        d->criterias.append(d->criteriaMap[criteria] + ' ' + argument);
        break;
    default:
        //TODO Discuss if we keep error checking here, or accept anything, even if it is wrong
        qDebug() << "Criteria " << d->criteriaMap[criteria] << " doesn't accept any argument.";
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
        d->criterias.append(d->criteriaMap[criteria] + " \"" + date + '\"');
        break;
    }
    default:
        //TODO Discuss if we keep error checking here, or accept anything, even if it is wrong
        qDebug() << "Criteria " << d->criteriaMap[criteria] << " doesn't accept a date as argument.";
        break;
    }
}

void SearchJob::addSearchCriteria(const QByteArray &searchCriteria)
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

QList<qint64> SearchJob::results() const
{
    Q_D(const SearchJob);
    return d->results;
}

QList<int> SearchJob::foundItems()
{
    Q_D(const SearchJob);

    QList<int> results;
    qCopy(d->results.begin(), d->results.end(), results.begin());

    return results;
}
