/*
   Copyright (C) 2009 Kevin Ottens <ervin@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include <qtest_kde.h>

#include "imapset.h"

#include <QtTest>
#include <QDebug>

using namespace KIMAP;

class ImapSetTest : public QObject
{
  Q_OBJECT

private Q_SLOTS:
  void shouldConvertToAndFromByteArray_data()
  {
    ImapSet set;

    QTest::addColumn<ImapSet>( "imapSet" );
    QTest::addColumn<QByteArray>( "byteArray" );

    QTest::newRow( "empty set" ) << ImapSet() << QByteArray();
    QTest::newRow( "unique value" ) << ImapSet( 7 ) << QByteArray( "7" );
    QTest::newRow( "single interval" ) << ImapSet( 7, 10 ) << QByteArray( "7:10" );
    QTest::newRow( "single interval with no upper bound" ) << ImapSet( 1, 0 ) << QByteArray( "1:*" );

    set = ImapSet( 7, 10 );
    set.add( ImapInterval( 12, 14 ) );
    QTest::newRow( "two intervals" ) << set << QByteArray( "7:10,12:14" );

    set = ImapSet( 7, 10 );
    set.add( ImapInterval( 12 ) );
    QTest::newRow( "two intervals with an infinite one" ) << set << QByteArray( "7:10,12:*" );

    set = ImapSet( 7, 10 );
    set.add( 5 );
    QTest::newRow( "one interval and a value" ) << set << QByteArray( "7:10,5" );

    set = ImapSet( 7, 10 );
    set.add( QList<ImapSet::Id>() << 5 << 3 );
    QTest::newRow( "one interval and two values" ) << set << QByteArray( "7:10,3,5" );
  }

  void shouldConvertToAndFromByteArray()
  {
    QFETCH( ImapSet, imapSet );
    QFETCH( QByteArray, byteArray );

    QCOMPARE( QString::fromUtf8( imapSet.toImapSequenceSet() ),
              QString::fromUtf8( byteArray ) );
    //qDebug() << "Expects" << imapSet << "got" << ImapSet::fromImapSequenceSet( byteArray );
    QCOMPARE( ImapSet::fromImapSequenceSet( byteArray ), imapSet );
  }
};

QTEST_KDEMAIN_CORE( ImapSetTest )

#include "imapsettest.moc"
