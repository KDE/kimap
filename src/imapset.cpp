/*
    SPDX-FileCopyrightText: 2007 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "imapset.h"

#include <QSharedData>

using namespace KIMAP;

class ImapInterval::Private : public QSharedData
{
public:
    Private()
        : QSharedData()
        , begin(0)
        , end(0)
    {
    }

    Private(const Private &other)
        : QSharedData(other)
    {
        begin = other.begin;
        end = other.end;
    }

    Id begin;
    Id end;
};

class ImapSet::Private : public QSharedData
{
public:
    Private()
        : QSharedData()
    {
    }
    Private(const Private &other)
        : QSharedData(other)
    {
        intervals = other.intervals;
    }

    ImapInterval::List intervals;
};

ImapInterval::ImapInterval()
    : d(new Private)
{
}

ImapInterval::ImapInterval(const ImapInterval &other)
    : d(other.d)
{
}

ImapInterval::ImapInterval(Id begin, Id end)
    : d(new Private)
{
    d->begin = begin;
    d->end = end;
}

ImapInterval::~ImapInterval()
{
}

ImapInterval &ImapInterval::operator=(const ImapInterval &other)
{
    if (this != &other) {
        d = other.d;
    }
    return *this;
}

bool ImapInterval::operator==(const ImapInterval &other) const
{
    return (d->begin == other.d->begin && d->end == other.d->end);
}

ImapInterval::Id ImapInterval::size() const
{
    if (!d->begin && !d->end) {
        return 0;
    }
    if (d->begin && !d->end) {
        return Q_INT64_C(0x7FFFFFFFFFFFFFFF) - d->begin + 1;
    }
    return d->end - d->begin + 1;
}

bool ImapInterval::hasDefinedBegin() const
{
    return d->begin != 0;
}

ImapInterval::Id ImapInterval::begin() const
{
    return d->begin;
}

bool ImapInterval::hasDefinedEnd() const
{
    return d->end != 0;
}

ImapInterval::Id ImapInterval::end() const
{
    if (hasDefinedEnd()) {
        return d->end;
    }
    return std::numeric_limits<qint64>::max();
}

void ImapInterval::setBegin(Id value)
{
    Q_ASSERT(value >= 0);
    Q_ASSERT(value <= d->end || !hasDefinedEnd());
    d->begin = value;
}

void ImapInterval::setEnd(Id value)
{
    Q_ASSERT(value >= 0);
    Q_ASSERT(value >= d->begin || !hasDefinedBegin());
    d->end = value;
}

QByteArray ImapInterval::toImapSequence() const
{
    if (size() == 0) {
        return QByteArray();
    }
    if (size() == 1) {
        return QByteArray::number(d->begin);
    }
    QByteArray rv = QByteArray::number(d->begin) + ':';
    if (hasDefinedEnd()) {
        rv += QByteArray::number(d->end);
    } else {
        rv += '*';
    }
    return rv;
}

ImapInterval ImapInterval::fromImapSequence(const QByteArray &sequence)
{
    QList<QByteArray> values = sequence.split(':');
    if (values.isEmpty() || values.size() > 2) {
        return ImapInterval();
    }

    bool ok = false;
    Id begin = values[0].toLongLong(&ok);

    if (!ok) {
        return ImapInterval();
    }

    Id end;

    if (values.size() == 1) {
        end = begin;
    } else if (values[1] == QByteArray("*")) {
        end = 0;
    } else {
        ok = false;
        end = values[1].toLongLong(&ok);
        if (!ok) {
            return ImapInterval();
        }
    }

    return ImapInterval(begin, end);
}

ImapSet::ImapSet()
    : d(new Private)
{
}

ImapSet::ImapSet(Id begin, Id end)
    : d(new Private)
{
    add(ImapInterval(begin, end));
}

ImapSet::ImapSet(Id value)
    : d(new Private)
{
    add(QVector<Id>() << value);
}

ImapSet::ImapSet(const ImapSet &other)
    : d(other.d)
{
}

ImapSet::~ImapSet()
{
}

ImapSet &ImapSet::operator=(const ImapSet &other)
{
    if (this != &other) {
        d = other.d;
    }
    return *this;
}

bool ImapSet::operator==(const ImapSet &other) const
{
    if (d->intervals.size() != other.d->intervals.size()) {
        return false;
    }

    for (const ImapInterval &interval : std::as_const(d->intervals)) {
        if (!other.d->intervals.contains(interval)) {
            return false;
        }
    }

    return true;
}

void ImapSet::add(Id value)
{
    add(QVector<Id>() << value);
}

void ImapSet::add(const QVector<Id> &values)
{
    QVector<Id> vals = values;
    std::sort(vals.begin(), vals.end());
    for (int i = 0; i < vals.count(); ++i) {
        const Id begin = vals[i];
        Q_ASSERT(begin >= 0);
        if (i == vals.count() - 1) {
            d->intervals << ImapInterval(begin, begin);
            break;
        }
        do {
            ++i;
            Q_ASSERT(vals[i] >= 0);
            if (vals[i] != (vals[i - 1] + 1)) {
                --i;
                break;
            }
        } while (i < vals.count() - 1);
        d->intervals << ImapInterval(begin, vals[i]);
    }
}

void ImapSet::add(const ImapInterval &interval)
{
    d->intervals << interval;
}

QByteArray ImapSet::toImapSequenceSet() const
{
    QList<QByteArray> rv;
    rv.reserve(d->intervals.count());
    for (const ImapInterval &interval : std::as_const(d->intervals)) {
        rv << interval.toImapSequence();
    }

    QByteArray result;

    if (!rv.isEmpty()) {
        result = rv.first();
        QList<QByteArray>::ConstIterator it = rv.constBegin();
        const QList<QByteArray>::ConstIterator end = rv.constEnd();
        ++it;
        for (; it != end; ++it) {
            result += ',' + (*it);
        }
    }

    return result;
}

ImapSet ImapSet::fromImapSequenceSet(const QByteArray &sequence)
{
    ImapSet result;

    const QList<QByteArray> intervals = sequence.split(',');

    for (const QByteArray &interval : std::as_const(intervals)) {
        if (!interval.isEmpty()) {
            result.add(ImapInterval::fromImapSequence(interval));
        }
    }

    return result;
}

ImapInterval::List ImapSet::intervals() const
{
    return d->intervals;
}

bool ImapSet::isEmpty() const
{
    return d->intervals.isEmpty();
}

void ImapSet::optimize()
{
    // There's nothing to optimize if we have fewer than 2 intervals
    if (d->intervals.size() < 2) {
        return;
    }

    // Sort the intervals in ascending order by their beginning value
    std::sort(d->intervals.begin(), d->intervals.end(), [](const ImapInterval &lhs, const ImapInterval &rhs) {
        return lhs.begin() < rhs.begin();
    });

    auto it = d->intervals.begin();
    while (it != d->intervals.end() && it != std::prev(d->intervals.end())) {
        auto next = std::next(it);
        // +1 so that we also merge neighbouring intervals, e.g. 1:2,3:4 -> 1:4
        if (it->hasDefinedEnd() && it->end() + 1 >= next->begin()) {
            next->setBegin(it->begin());
            if (next->hasDefinedEnd() && it->end() > next->end()) {
                next->setEnd(it->end());
            }
            it = d->intervals.erase(it);
        } else if (!it->hasDefinedEnd()) {
            // We can eat up all the remaining intervals
            it = d->intervals.erase(next, d->intervals.end());
        } else {
            ++it;
        }
    }
}

QDebug &operator<<(QDebug &d, const ImapInterval &interval)
{
    d << interval.toImapSequence();
    return d;
}

QDebug &operator<<(QDebug &d, const ImapSet &set)
{
    d << set.toImapSequenceSet();
    return d;
}
