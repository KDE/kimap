/*
 * SPDX-FileCopyrightText: 2013 Daniel Vrátil <dvratil@redhat.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */

#include <QTest>

#include "kimap/loginjob.h"
#include "kimap/searchjob.h"
#include "kimap/session.h"
#include "kimaptest/fakeserver.h"

#include <QTest>

using SearchCriteriaValuePair = QPair<KIMAP::SearchJob::SearchCriteria, QByteArray>;

Q_DECLARE_METATYPE(QList<SearchCriteriaValuePair>)
Q_DECLARE_METATYPE(KIMAP::SearchJob::SearchLogic)
Q_DECLARE_METATYPE(KIMAP::Term)

#define searchPair(a, b) qMakePair<KIMAP::SearchJob::SearchCriteria, QByteArray>(a, b)

class SearchJobTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    void testSearch_data()
    {
        QTest::addColumn<QList<QByteArray>>("scenario");
        QTest::addColumn<bool>("uidbased");
        QTest::addColumn<int>("expectedResultsCount");
        QTest::addColumn<KIMAP::SearchJob::SearchLogic>("searchLogic");
        QTest::addColumn<QList<SearchCriteriaValuePair>>("searchCriteria");

        QList<QByteArray> scenario;
        QList<SearchCriteriaValuePair> criteria;
        scenario << FakeServer::preauth() << "C: A000001 UID SEARCH HEADER Message-Id <12345678@mail.box>"
                 << "S: * SEARCH 10 12"
                 << "S: A000001 OK search done";

        criteria << searchPair(KIMAP::SearchJob::Header, "Message-Id <12345678@mail.box>");
        QTest::newRow("uidbased header search") << scenario << true << 2 << KIMAP::SearchJob::And << criteria;

        scenario.clear();
        criteria.clear();
        scenario << FakeServer::preauth() << "C: A000001 SEARCH OR (NEW) (HEADER Message-Id <12345678@mail.box>)"
                 << "S: * SEARCH"
                 << "S: A000001 OK search done";

        criteria << searchPair(KIMAP::SearchJob::New, QByteArray()) << searchPair(KIMAP::SearchJob::Header, "Message-Id <12345678@mail.box>");
        QTest::newRow("OR search with no results") << scenario << false << 0 << KIMAP::SearchJob::Or << criteria;

        scenario.clear();
        criteria.clear();
        scenario << FakeServer::preauth() << "C: A000001 SEARCH TO {25}\r\n<testuser@kde.testserver>"
                 << "S: * SEARCH 1"
                 << "S: A000001 OK search done";
        criteria << searchPair(KIMAP::SearchJob::To, "<testuser@kde.testserver>");
        QTest::newRow("literal data search") << scenario << false << 1 << KIMAP::SearchJob::And << criteria;

        scenario.clear();
        criteria.clear();
        scenario << FakeServer::preauth() << "C: A000001 UID SEARCH NOT (NEW)"
                 << "S: * SEARCH 1 2 3 4 5 6"
                 << "S: A000001 OK search done";
        criteria << searchPair(KIMAP::SearchJob::New, QByteArray());
        QTest::newRow("uidbased NOT NEW search") << scenario << true << 6 << KIMAP::SearchJob::Not << criteria;
    }

    void testSearch()
    {
        QFETCH(QList<QByteArray>, scenario);
        QFETCH(bool, uidbased);
        QFETCH(int, expectedResultsCount);
        QFETCH(KIMAP::SearchJob::SearchLogic, searchLogic);
        QFETCH(QList<SearchCriteriaValuePair>, searchCriteria);

        FakeServer fakeServer;
        fakeServer.setScenario(scenario);
        fakeServer.startAndWait();

        KIMAP::Session session(QStringLiteral("127.0.0.1"), 5989);

        auto job = new KIMAP::SearchJob(&session);
        job->setUidBased(uidbased);
        job->setSearchLogic(searchLogic);
        for (const SearchCriteriaValuePair &pair : std::as_const(searchCriteria)) {
            if (pair.second.isEmpty()) {
                job->addSearchCriteria(pair.first);
            } else {
                job->addSearchCriteria(pair.first, pair.second);
            }
        }

        bool result = job->exec();
        QVERIFY(result);
        if (result) {
            QVector<qint64> foundItems = job->results();
            QCOMPARE(foundItems.size(), expectedResultsCount);
        }

        fakeServer.quit();
    }

    void testSearchTerm_data()
    {
        QTest::addColumn<QList<QByteArray>>("scenario");
        QTest::addColumn<bool>("uidbased");
        QTest::addColumn<int>("expectedResultsCount");
        QTest::addColumn<KIMAP::Term>("searchTerm");

        {
            QList<QByteArray> scenario;
            scenario << FakeServer::preauth() << "C: A000001 UID SEARCH HEADER Message-Id \"<12345678@mail.box>\""
                     << "S: * SEARCH 10 12"
                     << "S: A000001 OK search done";

            QTest::newRow("uidbased header search") << scenario << true << 2
                                                    << KIMAP::Term(QStringLiteral("Message-Id"), QStringLiteral("<12345678@mail.box>"));
        }
        {
            QList<QByteArray> scenario;
            scenario << FakeServer::preauth() << "C: A000001 SEARCH OR NEW HEADER Message-Id \"<12345678@mail.box>\""
                     << "S: * SEARCH"
                     << "S: A000001 OK search done";

            QTest::newRow("OR search with no results")
                << scenario << false << 0
                << KIMAP::Term(KIMAP::Term::Or,
                               QVector<KIMAP::Term>() << KIMAP::Term(KIMAP::Term::New)
                                                      << KIMAP::Term(QStringLiteral("Message-Id"), QStringLiteral("<12345678@mail.box>")));
        }
        {
            QList<QByteArray> scenario;
            scenario << FakeServer::preauth() << "C: A000001 SEARCH TO \"<testuser@kde.testserver>\""
                     << "S: * SEARCH 1"
                     << "S: A000001 OK search done";
            QTest::newRow("literal data search") << scenario << false << 1 << KIMAP::Term(KIMAP::Term::To, QStringLiteral("<testuser@kde.testserver>"));
        }
        {
            QList<QByteArray> scenario;
            scenario << FakeServer::preauth() << "C: A000001 UID SEARCH NOT NEW"
                     << "S: * SEARCH 1 2 3 4 5 6"
                     << "S: A000001 OK search done";
            QTest::newRow("uidbased NOT NEW search") << scenario << true << 6 << KIMAP::Term(KIMAP::Term::New).setNegated(true);
        }

        {
            QList<QByteArray> scenario;
            scenario
                << FakeServer::preauth()
                << R"(C: A000001 UID SEARCH OR HEADER Message-Id "<1234567@mail.box>" (OR HEADER Message-Id "<7654321@mail.box>" (OR HEADER Message-Id "<abcdefg@mail.box>" HEADER Message-Id "<gfedcba@mail.box>")))"
                << "S: * SEARCH 1 2 3 4"
                << "S: A000001 OK search done";
            KIMAP::Term term{KIMAP::Term::Or,
                             {KIMAP::Term{QStringLiteral("Message-Id"), QStringLiteral("<1234567@mail.box>")},
                              KIMAP::Term{QStringLiteral("Message-Id"), QStringLiteral("<7654321@mail.box>")},
                              KIMAP::Term{QStringLiteral("Message-Id"), QStringLiteral("<abcdefg@mail.box>")},
                              KIMAP::Term{QStringLiteral("Message-Id"), QStringLiteral("<gfedcba@mail.box>")}}};
            QTest::newRow("OR with multiple subterms") << scenario << true << 4 << term;
        }
    }

    void testSearchTerm()
    {
        QFETCH(QList<QByteArray>, scenario);
        QFETCH(bool, uidbased);
        QFETCH(int, expectedResultsCount);
        QFETCH(KIMAP::Term, searchTerm);

        FakeServer fakeServer;
        fakeServer.setScenario(scenario);
        fakeServer.startAndWait();

        KIMAP::Session session(QStringLiteral("127.0.0.1"), 5989);

        auto job = new KIMAP::SearchJob(&session);
        job->setUidBased(uidbased);
        job->setTerm(searchTerm);

        bool result = job->exec();
        QVERIFY(result);
        if (result) {
            QVector<qint64> foundItems = job->results();
            QCOMPARE(foundItems.size(), expectedResultsCount);
        }

        fakeServer.quit();
    }
};

QTEST_GUILESS_MAIN(SearchJobTest)

#include "searchjobtest.moc"
