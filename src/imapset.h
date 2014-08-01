/*
    Copyright (c) 2007 Volker Krause <vkrause@kde.org>

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

#ifndef KIMAP_IMAPSET_H
#define KIMAP_IMAPSET_H

#include "kimap_export.h"

#include <QtCore/QByteArray>
#include <QtCore/QDebug>
#include <QtCore/QList>
#include <QtCore/QMetaType>
#include <QtCore/QSharedDataPointer>

namespace KIMAP
{

/**
  Represents a single interval in an ImapSet.
  This class is implicitly shared.
*/
class KIMAP_EXPORT ImapInterval
{
public:
    /**
     * Describes the ids stored in the interval.
     */
    typedef qint64 Id;

    /**
      A list of ImapInterval objects.
    */
    typedef QList<ImapInterval> List;

    /**
      Constructs an interval that covers all positive numbers.
    */
    ImapInterval();

    /**
      Copy constructor.
    */
    ImapInterval(const ImapInterval &other);

    /**
      Create a new interval.
      @param begin The begin of the interval.
      @param end Keep default (0) to just set the interval begin
    */
    explicit ImapInterval(Id begin, Id end = 0);

    /**
      Destructor.
    */
    ~ImapInterval();

    /**
      Assignment operator.
    */
    ImapInterval &operator=(const ImapInterval &other);

    /**
      Comparison operator.
    */
    bool operator==(const ImapInterval &other) const;

    /**
      Returns the size of this interval.
      Size is only defined for finite intervals.
    */
    Id size() const;

    /**
      Returns true if this interval has a defined begin.
    */
    bool hasDefinedBegin() const;

    /**
      Returns the begin of this interval. The value is the smallest value part of the interval.
      Only valid if begin is defined.
    */
    Id begin() const;

    /**
      Returns true if this intercal has been defined.
    */
    bool hasDefinedEnd() const;

    /**
      Returns the end of this interval. This value is the largest value part of the interval.
      Only valid if hasDefinedEnd() returned true.
    */
    Id end() const;

    /**
      Sets the begin of the interval.
    */
    void setBegin(Id value);

    /**
      Sets the end of this interval.
    */
    void setEnd(Id value);

    /**
      Converts this set into an IMAP compatible sequence.
    */
    QByteArray toImapSequence() const;

    /**
      Return the interval corresponding to the given IMAP-compatible QByteArray representation
    */
    static ImapInterval fromImapSequence(const QByteArray &sequence);

private:
    class Private;
    QSharedDataPointer<Private> d;
};

/**
  Represents a set of natural numbers (1->\f$\infty\f$) in a as compact as possible form.
  Used to address Akonadi items via the IMAP protocol or in the database.
  This class is implicitly shared.
*/
class KIMAP_EXPORT ImapSet
{
public:
    /**
     * Describes the ids stored in the set.
     */
    typedef qint64 Id;

    /**
      Constructs an empty set.
    */
    ImapSet();

    /**
      Constructs a set containing a single interval.
    */
    ImapSet(Id begin, Id end);

    /**
      Constructs a set containing a single value.
    */
    explicit ImapSet(Id value);

    /**
      Copy constructor.
    */
    ImapSet(const ImapSet &other);

    /**
      Destructor.
    */
    ~ImapSet();

    /**
      Assignment operator.
    */
    ImapSet &operator=(const ImapSet &other);

    /**
      Comparison operator.
    */
    bool operator==(const ImapSet &other) const;

    /**
      Adds a single positive integer numbers to the set.
      The list is sorted and split into as large as possible intervals.
      No interval merging is performed.
      @param value A positive integer number
    */
    void add(Id value);

    /**
      Adds the given list of positive integer numbers to the set.
      The list is sorted and split into as large as possible intervals.
      No interval merging is performed.
      @param values List of positive integer numbers in arbitrary order
    */
    void add(const QList<Id> &values);

    /**
      Adds the given ImapInterval to this set.
      No interval merging is performed.
      @param interval the interval to add
    */
    void add(const ImapInterval &interval);

    /**
      Returns a IMAP-compatible QByteArray representation of this set.
    */
    QByteArray toImapSequenceSet() const;

    /**
      Return the set corresponding to the given IMAP-compatible QByteArray representation
    */
    static ImapSet fromImapSequenceSet(const QByteArray &sequence);

    /**
      Returns the intervals this set consists of.
    */
    ImapInterval::List intervals() const;

    /**
      Returns true if this set doesn't contains any values.
    */
    bool isEmpty() const;

private:
    class Private;
    QSharedDataPointer<Private> d;
};

}

KIMAP_EXPORT QDebug &operator<<(QDebug &d, const KIMAP::ImapInterval &interval);
KIMAP_EXPORT QDebug &operator<<(QDebug &d, const KIMAP::ImapSet &set);

Q_DECLARE_METATYPE(KIMAP::ImapInterval)
Q_DECLARE_METATYPE(KIMAP::ImapInterval::List)
Q_DECLARE_METATYPE(KIMAP::ImapSet)

#endif
