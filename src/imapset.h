/*
    SPDX-FileCopyrightText: 2007 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "kimap_export.h"

#include <QByteArray>
#include <QDebug>
#include <QList>
#include <QMetaType>
#include <QSharedDataPointer>

namespace KIMAP
{
/*!
  \class KIMAP::ImapInterval
  \inmodule KIMAP
  \inheaderfile KIMAP/ImapInterval

  Represents a single interval in an ImapSet.
  This class is implicitly shared.
*/
class KIMAP_EXPORT ImapInterval
{
public:
    /*!
     * \typealias KIMAP::ImapInterval::Id
     *
     * Describes the ids stored in the interval.
     */
    using Id = qint64;

    /*!
      \typealias KIMAP::ImapInterval::List

      A list of ImapInterval objects.
    */
    using List = QList<ImapInterval>;

    /*!
      Constructs an interval that covers all positive numbers.
    */
    ImapInterval();

    /*!
      Copy constructor.
    */
    ImapInterval(const ImapInterval &other);

    /*!
      Create a new interval.
      \a begin The begin of the interval.
      \a end Keep default (0) to just set the interval begin
    */
    explicit ImapInterval(Id begin, Id end = 0);

    /*!
      Destructor.
    */
    ~ImapInterval();

    /*!
      Assignment operator.
    */
    ImapInterval &operator=(const ImapInterval &other);

    /*!
      Comparison operator.
    */
    bool operator==(const ImapInterval &other) const;

    /*!
      Returns the size of this interval.
      Size is only defined for finite intervals.
    */
    [[nodiscard]] Id size() const;

    /*!
      Returns true if this interval has a defined begin.
    */
    [[nodiscard]] bool hasDefinedBegin() const;

    /*!
      Returns the begin of this interval. The value is the smallest value part of the interval.
      Only valid if begin is defined.
    */
    [[nodiscard]] Id begin() const;

    /*!
      Returns true if this intercal has been defined.
    */
    [[nodiscard]] bool hasDefinedEnd() const;

    /*!
      Returns the end of this interval. This value is the largest value part of the interval.
      Only valid if hasDefinedEnd() returned true.
    */
    [[nodiscard]] Id end() const;

    /*!
      Sets the begin of the interval.
    */
    void setBegin(Id value);

    /*!
      Sets the end of this interval.
    */
    void setEnd(Id value);

    /*!
      Converts this set into an IMAP compatible sequence.
    */
    [[nodiscard]] QByteArray toImapSequence() const;

    /*!
      Return the interval corresponding to the given IMAP-compatible QByteArray representation
    */
    static ImapInterval fromImapSequence(const QByteArray &sequence);

private:
    class Private;
    QSharedDataPointer<Private> d;
};

/*!
  Represents a set of natural numbers (1->∞) in a as compact as possible form.
  Used to address Akonadi items via the IMAP protocol or in the database.
  This class is implicitly shared.
*/
class KIMAP_EXPORT ImapSet
{
public:
    /*!
     * \typealias KIMAP::ImapSet::Id
     *
     * Describes the ids stored in the set.
     */
    using Id = qint64;

    /*!
      Constructs an empty set.
    */
    ImapSet();

    /*!
      Constructs a set containing a single interval.
    */
    ImapSet(Id begin, Id end);

    /*!
      Constructs a set containing a single value.
    */
    explicit ImapSet(Id value);

    /*!
      Copy constructor.
    */
    ImapSet(const ImapSet &other);

    /*!
      Destructor.
    */
    ~ImapSet();

    /*!
      Assignment operator.
    */
    ImapSet &operator=(const ImapSet &other);

    /*!
      Comparison operator.
    */
    bool operator==(const ImapSet &other) const;

    /*!
      Adds a single positive integer numbers to the set.
      The list is sorted and split into as large as possible intervals.
      No interval merging is performed.
      \a value A positive integer number
    */
    void add(Id value);

    /*!
      Adds the given list of positive integer numbers to the set.
      The list is sorted and split into as large as possible intervals.
      No interval merging is performed.
      \a values List of positive integer numbers in arbitrary order
    */
    void add(const QList<Id> &values);

    /*!
      Adds the given ImapInterval to this set.
      No interval merging is performed.
      \a interval the interval to add
    */
    void add(const ImapInterval &interval);

    /*!
      Returns a IMAP-compatible QByteArray representation of this set.
    */
    [[nodiscard]] QByteArray toImapSequenceSet() const;

    /*!
      Return the set corresponding to the given IMAP-compatible QByteArray representation
    */
    static ImapSet fromImapSequenceSet(const QByteArray &sequence);

    /*!
      Returns the intervals this set consists of.
    */
    [[nodiscard]] ImapInterval::List intervals() const;

    /*!
      Returns true if this set doesn't contains any values.
    */
    [[nodiscard]] bool isEmpty() const;

    /*!
     * Optimizes the ImapSet by sorting and merging overlapping intervals.
     *
     * Normally you shouldn't need to call this method. KIMAP will make sure
     * to optimize the ImapSet before serializing it to string and sending it
     * to the IMAP server.
     */
    void optimize();

private:
    class Private;
    QSharedDataPointer<Private> d;
};

}

/*!
 * \fn operator<<(QDebug &d, const KIMAP::ImapInterval &interval)
 * \inmodule KIMAP
 * \inheaderfile KIMAP/ImapSet
 *
 * Debug stream operator for ImapInterval.
 */
KIMAP_EXPORT QDebug &operator<<(QDebug &d, const KIMAP::ImapInterval &interval);
/*!
 * \fn operator<<(QDebug &d, const KIMAP::ImapSet &set)
 * \inmodule KIMAP
 * \inheaderfile KIMAP/ImapSet
 *
 * Debug stream operator for ImapSet.
 */
KIMAP_EXPORT QDebug &operator<<(QDebug &d, const KIMAP::ImapSet &set);

Q_DECLARE_METATYPE(KIMAP::ImapInterval)
Q_DECLARE_METATYPE(KIMAP::ImapInterval::List)
Q_DECLARE_METATYPE(KIMAP::ImapSet)
