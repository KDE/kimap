/*
   SPDX-FileCopyrightText: 2009 Kevin Ottens <ervin@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <QTest>

#include "kimap/imapset.h"

#include <QDebug>
#include <QTest>

using namespace KIMAP;

QByteArray operator""_ba(const char *str, std::size_t len)
{
    return QByteArray{str, static_cast<int>(len)};
}

class ImapSetTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void shouldConvertToAndFromByteArray_data()
    {
        ImapSet set;

        QTest::addColumn<ImapSet>("imapSet");
        QTest::addColumn<QByteArray>("byteArray");

        QTest::newRow("empty set") << ImapSet() << QByteArray();
        QTest::newRow("unique value") << ImapSet(7) << QByteArray("7");
        QTest::newRow("single interval") << ImapSet(7, 10) << QByteArray("7:10");
        QTest::newRow("single interval with no upper bound") << ImapSet(1, 0) << QByteArray("1:*");

        set = ImapSet(7, 10);
        set.add(ImapInterval(12, 14));
        QTest::newRow("two intervals") << set << QByteArray("7:10,12:14");

        set = ImapSet(7, 10);
        set.add(ImapInterval(12));
        QTest::newRow("two intervals with an infinite one") << set << QByteArray("7:10,12:*");

        set = ImapSet(7, 10);
        set.add(5);
        QTest::newRow("one interval and a value") << set << QByteArray("7:10,5");

        set = ImapSet(7, 10);
        set.add(QVector<ImapSet::Id>() << 5 << 3);
        QTest::newRow("one interval and two values") << set << QByteArray("7:10,3,5");
    }

    void shouldConvertToAndFromByteArray()
    {
        QFETCH(ImapSet, imapSet);
        QFETCH(QByteArray, byteArray);

        QCOMPARE(QString::fromUtf8(imapSet.toImapSequenceSet()), QString::fromUtf8(byteArray));
        // qDebug() << "Expects" << imapSet << "got" << ImapSet::fromImapSequenceSet( byteArray );
        QCOMPARE(ImapSet::fromImapSequenceSet(byteArray), imapSet);
    }

    void testOptimize_data()
    {
        QTest::addColumn<ImapSet>("imapSet");
        QTest::addColumn<QByteArray>("originalString");
        QTest::addColumn<QByteArray>("expectedString");

        {
            ImapSet imapSet;
            for (int i = 1; i <= 10; ++i) {
                imapSet.add(i);
            }
            QTest::newRow("Neighbouring numbers") << imapSet << "1,2,3,4,5,6,7,8,9,10"_ba
                                                  << "1:10"_ba;
        }
        {
            ImapSet imapSet;
            imapSet.add(ImapInterval{1, 3});
            imapSet.add(ImapInterval{5, 7});
            QTest::newRow("Neighbouring intervals with a gap") << imapSet << "1:3,5:7"_ba
                                                               << "1:3,5:7"_ba;
        }
        {
            ImapSet imapSet;
            for (int i : {5, 8, 3, 1, 9, 2, 7, 4, 6}) {
                imapSet.add(i);
            }
            QTest::newRow("Random order") << imapSet << "5,8,3,1,9,2,7,4,6"_ba
                                          << "1:9"_ba;
        }
        {
            ImapSet imapSet;
            imapSet.add(ImapInterval{1, 3});
            imapSet.add(ImapInterval{2, 4});
            QTest::newRow("Overlapping") << imapSet << "1:3,2:4"_ba
                                         << "1:4"_ba;
        }
        {
            ImapSet imapSet;
            imapSet.add(ImapInterval{2, 4});
            imapSet.add(ImapInterval{1, 3});
            imapSet.add(4);
            imapSet.add(ImapInterval{7, 8});
            imapSet.add(ImapInterval{8, 9});
            QTest::newRow("Multiple overlapping with a gap") << imapSet << "2:4,1:3,4,7:8,8:9"_ba
                                                             << "1:4,7:9"_ba;
        }
        {
            ImapSet imapSet;
            imapSet.add(5);
            imapSet.add(8);
            imapSet.add(10);
            imapSet.add(ImapInterval{0, 20});
            QTest::newRow("Overlapping multiple intervals") << imapSet << "5,8,10,0:20"_ba
                                                            << "0:20"_ba;
        }
        {
            ImapSet imapSet;
            imapSet.add(1);
            imapSet.add(ImapInterval{3, 5});
            imapSet.add(ImapInterval{4, 0});
            QTest::newRow("Open end overlap") << imapSet << "1,3:5,4:*"_ba
                                              << "1,3:*"_ba;
        }
        {
            ImapSet imapSet;
            imapSet.add(ImapInterval{1, 4});
            imapSet.add(3);
            QTest::newRow("Value within interval") << imapSet << "1:4,3"_ba
                                                   << "1:4"_ba;
        }
        {
            ImapSet imapSet;
            imapSet.add(ImapInterval{1, 0});
            imapSet.add(ImapInterval{3, 0});
            imapSet.add(5);
            QTest::newRow("Multiple open end intervals") << imapSet << "1:*,3:*,5"_ba
                                                         << "1:*"_ba;
        }
        {
            ImapSet imapSet;
            for (ImapSet::Id id : {1, 2, 3, 5, 6, 8, 9, 10, 15, 16, 19, 20, 21, 23}) {
                imapSet.add(id);
            }
            QTest::newRow("Merge single values") << imapSet << "1,2,3,5,6,8,9,10,15,16,19,20,21,23"_ba
                                                 << "1:3,5:6,8:10,15:16,19:21,23"_ba;
        }
    }

    void testOptimize()
    {
        QFETCH(ImapSet, imapSet);
        QFETCH(QByteArray, originalString);
        QFETCH(QByteArray, expectedString);

        QCOMPARE(imapSet.intervals().size(), originalString.count(",") + 1);
        QCOMPARE(imapSet.toImapSequenceSet(), originalString);

        imapSet.optimize();

        QCOMPARE(imapSet.intervals().size(), expectedString.count(",") + 1);
        QCOMPARE(imapSet.toImapSequenceSet(), expectedString);
    }
};

QTEST_GUILESS_MAIN(ImapSetTest)

#include "imapsettest.moc"
