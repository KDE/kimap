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

#include "imapset.h"

#include <QtCore/QSharedData>

using namespace KIMAP;

class ImapInterval::Private : public QSharedData
{
  public:
    Private() :
      QSharedData(),
      begin( 0 ),
      end( 0 )
    {}

    Private( const Private &other ) :
      QSharedData( other )
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
    Private() : QSharedData() {}
    Private( const Private &other ) :
      QSharedData( other )
    {
    }

    ImapInterval::List intervals;
};


ImapInterval::ImapInterval() :
    d( new Private )
{
}

ImapInterval::ImapInterval(const ImapInterval & other) :
    d( other.d )
{
}

ImapInterval::ImapInterval(Id begin, Id end) :
    d( new Private )
{
  d->begin = begin;
  d->end = end;
}

ImapInterval::~ ImapInterval()
{
}

ImapInterval& ImapInterval::operator =(const ImapInterval & other)
{
  if ( this != & other )
    d = other.d;
  return *this;
}

bool ImapInterval::operator ==(const ImapInterval & other) const
{
  return ( d->begin == other.d->begin && d->end == other.d->end );
}

ImapInterval::Id ImapInterval::size() const
{
  if ( !d->begin && !d->end )
    return 0;
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
  if ( hasDefinedEnd() )
    return d->end;
  return 0xFFFFFFFF; // should be INT_MAX, but where is that defined again?
}

void ImapInterval::setBegin(Id value)
{
  Q_ASSERT( value >= 0 );
  Q_ASSERT( value <= d->end || !hasDefinedEnd() );
  d->begin = value;
}

void ImapInterval::setEnd(Id value)
{
  Q_ASSERT( value >= 0 );
  Q_ASSERT( value >= d->begin || !hasDefinedBegin() );
  d->end = value;
}

QByteArray ImapInterval::toImapSequence() const
{
  if ( size() == 0 )
    return QByteArray();
  if ( size() == 1 )
    return QByteArray::number( d->begin );
  QByteArray rv;
  rv += QByteArray::number( d->begin ) + ':';
  if ( hasDefinedEnd() )
    rv += QByteArray::number( d->end );
  else
    rv += '*';
  return rv;
}

ImapInterval ImapInterval::fromImapSequence( const QByteArray &sequence )
{
  QList<QByteArray> values = sequence.split( ':' );
  if ( values.isEmpty() || values.size() > 2 ) {
    return ImapInterval();
  }

  bool ok = false;
  Id begin = values[0].toLongLong(&ok);

  if ( !ok ) {
    return ImapInterval();
  }

  Id end;

  if ( values.size() == 1 ) {
    end = begin;
  } else if ( values[1] == QByteArray( "*" ) ) {
    end = 0;
  } else {
    ok = false;
    end = values[1].toLongLong(&ok);
    if ( !ok ) {
      return ImapInterval();
    }
  }

  return ImapInterval( begin, end );
}

ImapSet::ImapSet() :
    d( new Private )
{
}

ImapSet::ImapSet( Id begin, Id end ) :
    d( new Private )
{
  add( ImapInterval( begin, end ) );
}

ImapSet::ImapSet( Id value ) :
    d( new Private )
{
  add( QList<Id>() << value );
}

ImapSet::ImapSet(const ImapSet & other) :
    d( other.d )
{
}

ImapSet::~ImapSet()
{
}

ImapSet & ImapSet::operator =(const ImapSet & other)
{
  if ( this != &other )
    d = other.d;
  return *this;
}

bool ImapSet::operator ==(const ImapSet &other) const
{
  if ( d->intervals.size()!=other.d->intervals.size() ) {
    return false;
  }

  foreach ( const ImapInterval &interval, d->intervals ) {
    if ( !other.d->intervals.contains( interval ) ) {
      return false;
    }
  }

  return true;
}

void ImapSet::add( Id value )
{
  add( QList<Id>() << value );
}

void ImapSet::add(const QList<Id> & values)
{
  QList<Id> vals = values;
  qSort( vals );
  for( int i = 0; i < vals.count(); ++i ) {
    const int begin = vals[i];
    Q_ASSERT( begin >= 0 );
    if ( i == vals.count() - 1 ) {
      d->intervals << ImapInterval( begin, begin );
      break;
    }
    do {
      ++i;
      Q_ASSERT( vals[i] >= 0 );
      if ( vals[i] != (vals[i - 1] + 1) ) {
        --i;
        break;
      }
    } while ( i < vals.count() - 1 );
    d->intervals << ImapInterval( begin, vals[i] );
  }
}

void ImapSet::add(const ImapInterval & interval)
{
  d->intervals << interval;
}

QByteArray ImapSet::toImapSequenceSet() const
{
  QList<QByteArray> rv;
  foreach ( const ImapInterval &interval, d->intervals ) {
    rv << interval.toImapSequence();
  }

  QByteArray result;

  if ( !rv.isEmpty() ) {
    result = rv.first();
    QList<QByteArray>::ConstIterator it = rv.constBegin();
    ++it;
    for ( ; it != rv.constEnd(); ++it ) {
      result += ',' + (*it);
    }
  }

  return result;
}

ImapSet ImapSet::fromImapSequenceSet( const QByteArray &sequence )
{
  ImapSet result;

  QList<QByteArray> intervals = sequence.split( ',' );

  foreach( const QByteArray &interval, intervals ) {
    if ( !interval.isEmpty() ) {
      result.add( ImapInterval::fromImapSequence( interval ) );
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

QDebug& operator<<( QDebug &d, const ImapInterval &interval )
{
  d << interval.toImapSequence();
  return d;
}

QDebug& operator<<( QDebug &d, const ImapSet &set )
{
  d << set.toImapSequenceSet();
  return d;
}

