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
class Term::Private : public QSharedData
{
public:
    QByteArray command;
    bool isFuzzy = false;
    bool isNegated = false;
    bool isNull = false;
};

Term::Term()
    : d(new Term::Private)
{
    d->isNull = true;
}

Term::Term(Term::Relation relation, const QList<Term> &subterms)
    : d(new Term::Private)
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
    : d(new Term::Private)
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
    : d(new Term::Private)
{
    d->command += "HEADER";
    d->command += ' ' + QByteArray(header.toUtf8().constData());
    d->command += " \"" + QByteArray(value.toUtf8().constData()) + "\"";
}

Term::Term(Term::BooleanSearchKey key)
    : d(new Term::Private)
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
    : d(new Term::Private)
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
    : d(new Term::Private)
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
    : d(new Term::Private)
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
    }

    QByteArray charset;
    QList<QByteArray> contents;
    QList<qint64> results;
    uint nextContent = 0;
    bool uidBased = false;
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

    const QByteArray term = d->term.serialize();
    if (term.startsWith('(')) {
        searchKey += term.mid(1, term.size() - 2);
    } else {
        searchKey += term;
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
