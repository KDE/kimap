/*
   SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
   SPDX-FileContributor: Kevin Ottens <kevin@kdab.com>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <QTest>

#include "kimap/fetchjob.h"
#include "kimap/session.h"
#include "kimaptest/fakeserver.h"

#include <QSignalSpy>
#include <QTest>

Q_DECLARE_METATYPE(KIMAP::FetchJob::FetchScope)

class FetchJobTest : public QObject
{
    Q_OBJECT

public:
    FetchJobTest()
    {
        qRegisterMetaType<KIMAP::ImapSet>();
    }

private:
    QStringList m_signals;

    QMap<qint64, qint64> m_uids;
    QMap<qint64, qint64> m_sizes;
    QMap<qint64, KIMAP::MessageFlags> m_flags;
    QMap<qint64, KIMAP::MessagePtr> m_messages;
    QMap<qint64, KIMAP::MessageParts> m_parts;
    QMap<qint64, KIMAP::MessageAttribute> m_attrs;
    QMap<qint64, KIMAP::Message> m_msgs;

public Q_SLOTS:
    void onHeadersReceived(const QString & /*mailBox*/,
                           const QMap<qint64, qint64> &uids,
                           const QMap<qint64, qint64> &sizes,
                           const QMap<qint64, KIMAP::MessageAttribute> &attrs,
                           const QMap<qint64, KIMAP::MessageFlags> &flags,
                           const QMap<qint64, KIMAP::MessagePtr> &messages)
    {
        m_signals << QStringLiteral("headersReceived");
        m_uids.unite(uids);
        m_sizes.unite(sizes);
        m_flags.unite(flags);
        m_messages.unite(messages);
        m_attrs.unite(attrs);
    }

    void onMessagesReceived(const QString & /*mailbox*/,
                            const QMap<qint64, qint64> &uids,
                            const QMap<qint64, KIMAP::MessageAttribute> &attrs,
                            const QMap<qint64, KIMAP::MessagePtr> &messages)
    {
        m_signals << QStringLiteral("messagesReceived");
        m_uids.unite(uids);
        m_messages.unite(messages);
        m_attrs.unite(attrs);
    }

    void onPartsReceived(const QString & /*mailbox*/,
                         const QMap<qint64, qint64> & /*uids*/,
                         const QMap<qint64, KIMAP::MessageAttribute> &attrs,
                         const QMap<qint64, KIMAP::MessageParts> &parts)
    {
        m_signals << QStringLiteral("partsReceived");
        m_attrs.unite(attrs);
        m_parts.unite(parts);
    }

    void onMessagesAvailable(const QMap<qint64, KIMAP::Message> &messages)
    {
        m_signals << QStringLiteral("messagesAvailable");
        m_msgs.unite(messages);
    }

private Q_SLOTS:

    void testFetch_data()
    {
        qRegisterMetaType<KIMAP::FetchJob::FetchScope>();

        QTest::addColumn<bool>("uidBased");
        QTest::addColumn<KIMAP::ImapSet>("set");
        QTest::addColumn<int>("expectedMessageCount");
        QTest::addColumn<QList<QByteArray>>("scenario");
        QTest::addColumn<KIMAP::FetchJob::FetchScope>("scope");
        QTest::addColumn<KIMAP::ImapSet>("expectedVanished");

        KIMAP::FetchJob::FetchScope scope;
        scope.mode = KIMAP::FetchJob::FetchScope::Flags;
        scope.changedSince = 123456789;

        QList<QByteArray> scenario;
        scenario << FakeServer::preauth() << "C: A000001 FETCH 1:4 (FLAGS UID) (CHANGEDSINCE 123456789)"
                 << "S: * 1 FETCH ( FLAGS () UID 1 )"
                 << "S: * 2 FETCH ( FLAGS () UID 2 )"
                 << "S: * 3 FETCH ( FLAGS () UID 3 )"
                 << "S: * 4 FETCH ( FLAGS () UID 4 )"
                 << "S: A000001 OK fetch done";

        QTest::newRow("messages have empty flags (with changedsince)") << false << KIMAP::ImapSet(1, 4) << 4 << scenario << scope << KIMAP::ImapSet{};

        scenario.clear();
        scope.changedSince = 0;
        scenario << FakeServer::preauth() << "C: A000001 FETCH 1:4 (FLAGS UID)"
                 << "S: * 1 FETCH ( FLAGS () UID 1 )"
                 << "S: * 2 FETCH ( FLAGS () UID 2 )"
                 << "S: * 3 FETCH ( FLAGS () UID 3 )"
                 << "S: * 4 FETCH ( FLAGS () UID 4 )"
                 << "S: A000001 OK fetch done";

        QTest::newRow("messages have empty flags") << false << KIMAP::ImapSet(1, 4) << 4 << scenario << scope << KIMAP::ImapSet{};

        scenario.clear();
        // kill the connection part-way through a list, with carriage returns at end
        // BUG 253619
        // this should fail, but it shouldn't crash
        scenario << FakeServer::preauth()
                 << "C: A000001 FETCH 11 (RFC822.SIZE INTERNALDATE BODY.PEEK[HEADER.FIELDS (TO FROM MESSAGE-ID REFERENCES IN-REPLY-TO SUBJECT DATE)] FLAGS UID)"
                 << "S: * 11 FETCH (RFC822.SIZE 770 INTERNALDATE \"11-Oct-2010 03:33:50 +0100\" BODY[HEADER.FIELDS (TO FROM MESSAGE-ID REFERENCES IN-REPLY-TO "
                    "SUBJECT DATE)] {246}"
                 << "S: From: John Smith <jonathanr.smith@foobarbaz.com>\r\nTo: "
                    "\"amagicemailaddress@foobarbazbarfoo.com\"\r\n\t<amagicemailaddress@foobarbazbarfoo.com>\r\nDate: Mon, 11 Oct 2010 03:34:48 "
                    "+0100\r\nSubject: unsubscribe\r\nMessage-ID: <ASDFFDSASDFFDS@foobarbaz.com>\r\n\r\n"
                 << "X";
        scope.mode = KIMAP::FetchJob::FetchScope::Headers;
        QTest::newRow("connection drop") << false << KIMAP::ImapSet(11, 11) << 1 << scenario << scope << KIMAP::ImapSet{};

        scenario.clear();
        // Important bit here if "([127.0.0.1])" which used to crash the stream parser
        scenario << FakeServer::preauth()
                 << "C: A000001 FETCH 11 (RFC822.SIZE INTERNALDATE BODY.PEEK[HEADER.FIELDS (TO FROM MESSAGE-ID REFERENCES IN-REPLY-TO SUBJECT DATE)] FLAGS UID)"
                 << "S: * 11 FETCH (RFC822.SIZE 770 INTERNALDATE \"11-Oct-2010 03:33:50 +0100\" BODY[HEADER.FIELDS (TO FROM MESSAGE-ID REFERENCES IN-REPLY-TO "
                    "SUBJECT DATE)] {246}"
                 << "S: ([127.0.0.1])\r\nDate: Mon, 11 Oct 2010 03:34:48 +0100\r\nSubject: unsubscribe\r\nMessage-ID: <ASDFFDSASDFFDS@foobarbaz.com>\r\n\r\n"
                 << "X";
        scope.mode = KIMAP::FetchJob::FetchScope::Headers;
        QTest::newRow("buffer overwrite") << false << KIMAP::ImapSet(11, 11) << 1 << scenario << scope << KIMAP::ImapSet{};

        scenario.clear();
        // We're assuming a buffer overwrite here which made us miss the opening parenthesis
        // for the properties list
        scenario << FakeServer::preauth()
                 << "C: A000001 FETCH 11 (RFC822.SIZE INTERNALDATE BODY.PEEK[HEADER.FIELDS (TO FROM MESSAGE-ID REFERENCES IN-REPLY-TO SUBJECT DATE)] FLAGS UID)"
                 << "S: * 11 FETCH {10}doh!\r\n\r\n\r\n)\r\n"
                 << "X";
        scope.mode = KIMAP::FetchJob::FetchScope::Headers;
        QTest::newRow("buffer overwrite 2") << false << KIMAP::ImapSet(11, 11) << 1 << scenario << scope << KIMAP::ImapSet{};

        scenario.clear();
        scenario << FakeServer::preauth() << "C: A000001 FETCH 11 (RFC822.SIZE INTERNALDATE BODY.PEEK[HEADER] FLAGS UID) (CHANGEDSINCE 123456789)"
                 << "S: * 11 FETCH (UID 123 RFC822.SIZE 770 INTERNALDATE \"11-Oct-2010 03:33:50 +0100\" BODY[HEADER] {245}"
                 << "S: From: John Smith <jonathanr.smith@foobarbaz.com>\r\nTo: "
                    "\"amagicemailaddress@foobarbazbarfoo.com\"\r\n\t<amagicemailaddress@foobarbazbarfoo.com>\r\nDate: Mon, 11 Oct 2010 03:34:48 "
                    "+0100\r\nSubject: unsubscribe\r\nMessage-ID: <ASDFFDSASDFFDS@foobarbaz.com>\r\n\r\n  FLAGS ())"
                 << "S: A000001 OK fetch done";
        scope.mode = KIMAP::FetchJob::FetchScope::FullHeaders;
        scope.changedSince = 123456789;
        QTest::newRow("fetch full headers") << false << KIMAP::ImapSet(11, 11) << 1 << scenario << scope << KIMAP::ImapSet{};

        scenario.clear();
        scenario << FakeServer::preauth() << "C: A000001 UID FETCH 300:500 (FLAGS UID) (CHANGEDSINCE 12345 VANISHED)"
                 << "S: * VANISHED (EARLIER) 300:310,405,411"
                 << "S: * 1 FETCH (UID 404 MODSEQ (65402) FLAGS (\\Seen))"
                 << "S: * 2 FETCH (UID 406 MODSEQ (75403) FLAGS (\\Deleted))"
                 << "S: * 4 FETCH (UID 408 MODSEQ (29738) FLAGS ($Nojunk $AutoJunk $MDNSent))"
                 << "S: A000001 OK Fetch completed";
        scope.mode = KIMAP::FetchJob::FetchScope::Flags;
        scope.changedSince = 12345;
        scope.qresync = true;
        KIMAP::ImapSet vanished;
        vanished.add(KIMAP::ImapInterval{300, 310});
        vanished.add(QVector<qint64>{405, 411});
        QTest::newRow("qresync") << true << KIMAP::ImapSet(300, 500) << 3 << scenario << scope << vanished;
    }

    void testFetch()
    {
        QFETCH(bool, uidBased);
        QFETCH(KIMAP::ImapSet, set);
        QFETCH(int, expectedMessageCount);
        QFETCH(QList<QByteArray>, scenario);
        QFETCH(KIMAP::FetchJob::FetchScope, scope);
        QFETCH(KIMAP::ImapSet, expectedVanished);

        FakeServer fakeServer;
        fakeServer.setScenario(scenario);
        fakeServer.startAndWait();

        KIMAP::Session session(QStringLiteral("127.0.0.1"), 5989);

        auto job = new KIMAP::FetchJob(&session);
        job->setUidBased(uidBased);
        job->setSequenceSet(set);
        job->setScope(scope);

        connect(job,
                SIGNAL(headersReceived(QString,
                                       QMap<qint64, qint64>,
                                       QMap<qint64, qint64>,
                                       QMap<qint64, KIMAP::MessageAttribute>,
                                       QMap<qint64, KIMAP::MessageFlags>,
                                       QMap<qint64, KIMAP::MessagePtr>)),
                this,
                SLOT(onHeadersReceived(QString,
                                       QMap<qint64, qint64>,
                                       QMap<qint64, qint64>,
                                       QMap<qint64, KIMAP::MessageAttribute>,
                                       QMap<qint64, KIMAP::MessageFlags>,
                                       QMap<qint64, KIMAP::MessagePtr>)));
        connect(job, &KIMAP::FetchJob::messagesAvailable, this, &FetchJobTest::onMessagesAvailable);

        QSignalSpy vanishedSpy(job, &KIMAP::FetchJob::messagesVanished);
        QVERIFY(vanishedSpy.isValid());

        bool result = job->exec();
        QEXPECT_FAIL("connection drop", "Expected failure on connection drop", Continue);
        QEXPECT_FAIL("buffer overwrite", "Expected failure on confused list", Continue);
        QEXPECT_FAIL("buffer overwrite 2", "Expected beginning of message missing", Continue);
        QVERIFY(result);
        if (result) {
            QVERIFY(m_signals.count() > 0);
            QCOMPARE(m_uids.count(), expectedMessageCount);
            QCOMPARE(m_msgs.count(), expectedMessageCount);
            if (scope.qresync) {
                QCOMPARE(vanishedSpy.size(), 1);
                QCOMPARE(vanishedSpy.at(0).at(0).value<KIMAP::ImapSet>(), expectedVanished);
            }
        }

        QVERIFY(fakeServer.isAllScenarioDone());
        fakeServer.quit();

        m_signals.clear();
        m_uids.clear();
        m_sizes.clear();
        m_flags.clear();
        m_messages.clear();
        m_parts.clear();
        m_attrs.clear();
        m_msgs.clear();
    }

    void testFetchStructure()
    {
        QList<QByteArray> scenario;
        scenario
            << FakeServer::preauth() << "C: A000001 FETCH 1:2 (BODYSTRUCTURE UID)"
            << R"(S: * 1 FETCH (UID 10 BODYSTRUCTURE ("TEXT" "PLAIN" ("CHARSET" "ISO-8859-1") NIL NIL "7BIT" 5 1 NIL NIL NIL)))"
            << R"(S: * 2 FETCH (UID 20 BODYSTRUCTURE (((("TEXT" "PLAIN" ("CHARSET" "ISO-8859-1") NIL NIL "7BIT" 72 4 NIL NIL NIL)("TEXT" "HTML" ("CHARSET" "ISO-8859-1") NIL NIL "QUOTED-PRINTABLE" 281 5 NIL NIL NIL) "ALTERNATIVE" ("BOUNDARY" "0001") NIL NIL)("IMAGE" "GIF" ("NAME" "B56.gif") "<B56@goomoji.gmail>" NIL "BASE64" 528 NIL NIL NIL) "RELATED" ("BOUNDARY" "0002") NIL NIL)("IMAGE" "JPEG" ("NAME" "photo.jpg") NIL NIL "BASE64" 53338 NIL ("ATTACHMENT" ("FILENAME" "photo.jpg")) NIL) "MIXED" ("BOUNDARY" "0003") NIL NIL)))"
            << "S: A000001 OK fetch done";

        KIMAP::FetchJob::FetchScope scope;
        scope.mode = KIMAP::FetchJob::FetchScope::Structure;

        FakeServer fakeServer;
        fakeServer.setScenario(scenario);
        fakeServer.startAndWait();

        KIMAP::Session session(QStringLiteral("127.0.0.1"), 5989);

        auto job = new KIMAP::FetchJob(&session);
        job->setUidBased(false);
        job->setSequenceSet(KIMAP::ImapSet(1, 2));
        job->setScope(scope);

        connect(job,
                SIGNAL(messagesReceived(QString, QMap<qint64, qint64>, QMap<qint64, KIMAP::MessageAttribute>, QMap<qint64, KIMAP::MessagePtr>)),
                this,
                SLOT(onMessagesReceived(QString, QMap<qint64, qint64>, QMap<qint64, KIMAP::MessageAttribute>, QMap<qint64, KIMAP::MessagePtr>)));
        connect(job, &KIMAP::FetchJob::messagesAvailable, this, &FetchJobTest::onMessagesAvailable);

        bool result = job->exec();
        QVERIFY(result);
        QVERIFY(m_signals.count() > 0);
        QCOMPARE(m_uids.count(), 2);
        QCOMPARE(m_messages[1]->attachments().count(), 0);
        QCOMPARE(m_messages[2]->attachments().count(), 1);
        QCOMPARE(m_messages[2]->contents().size(), 2);
        QCOMPARE(m_messages[2]->contents()[0]->contents().size(), 2);
        QCOMPARE(m_messages[2]->attachments().at(0)->contentDisposition()->filename(), QStringLiteral("photo.jpg"));
        QCOMPARE(m_msgs.count(), 2);
        QCOMPARE(m_msgs[1].message->attachments().count(), 0);
        QCOMPARE(m_msgs[2].message->attachments().count(), 1);
        QCOMPARE(m_msgs[2].message->contents().size(), 2);
        QCOMPARE(m_msgs[2].message->contents()[0]->contents().size(), 2);
        QCOMPARE(m_msgs[2].message->attachments().at(0)->contentDisposition()->filename(), QStringLiteral("photo.jpg"));

        fakeServer.quit();

        m_signals.clear();
        m_uids.clear();
        m_sizes.clear();
        m_flags.clear();
        m_messages.clear();
        m_parts.clear();
        m_attrs.clear();
        m_msgs.clear();
    }

    void testFetchParts()
    {
        QList<QByteArray> scenario;
        scenario << FakeServer::preauth()
                 << "C: A000001 FETCH 2 (BODY.PEEK[HEADER.FIELDS (TO FROM MESSAGE-ID REFERENCES IN-REPLY-TO SUBJECT DATE)] BODY.PEEK[1.1.1.MIME] "
                    "BODY.PEEK[1.1.1] FLAGS UID)"
                 << "S: * 2 FETCH (UID 20 FLAGS (\\Seen) BODY[HEADER.FIELDS (TO FROM MESSAGE-ID REFERENCES IN-REPLY-TO SUBJECT DATE)] {154}\r\nFrom: Joe Smith "
                    "<smith@example.com>\r\nDate: Wed, 2 Mar 2011 11:33:24 +0700\r\nMessage-ID: <1234@example.com>\r\nSubject: hello\r\nTo: Jane "
                    "<jane@example.com>\r\n\r\n BODY[1.1.1] {28}\r\nHi Jane, nice to meet you!\r\n BODY[1.1.1.MIME] {48}\r\nContent-Type: text/plain; "
                    "charset=ISO-8859-1\r\n\r\n)\r\n"
                 << "S: A000001 OK fetch done";

        KIMAP::FetchJob::FetchScope scope;
        scope.mode = KIMAP::FetchJob::FetchScope::HeaderAndContent;
        scope.parts.clear();
        scope.parts.append("1.1.1");

        FakeServer fakeServer;
        fakeServer.setScenario(scenario);
        fakeServer.startAndWait();

        KIMAP::Session session(QStringLiteral("127.0.0.1"), 5989);

        auto job = new KIMAP::FetchJob(&session);
        job->setUidBased(false);
        job->setSequenceSet(KIMAP::ImapSet(2, 2));
        job->setScope(scope);

        connect(job,
                SIGNAL(headersReceived(QString,
                                       QMap<qint64, qint64>,
                                       QMap<qint64, qint64>,
                                       QMap<qint64, KIMAP::MessageAttribute>,
                                       QMap<qint64, KIMAP::MessageFlags>,
                                       QMap<qint64, KIMAP::MessagePtr>)),
                this,
                SLOT(onHeadersReceived(QString,
                                       QMap<qint64, qint64>,
                                       QMap<qint64, qint64>,
                                       QMap<qint64, KIMAP::MessageAttribute>,
                                       QMap<qint64, KIMAP::MessageFlags>,
                                       QMap<qint64, KIMAP::MessagePtr>)));
        connect(job,
                SIGNAL(partsReceived(QString, QMap<qint64, qint64>, QMap<qint64, KIMAP::MessageAttribute>, QMap<qint64, KIMAP::MessageParts>)),
                this,
                SLOT(onPartsReceived(QString, QMap<qint64, qint64>, QMap<qint64, KIMAP::MessageAttribute>, QMap<qint64, KIMAP::MessageParts>)));
        connect(job, &KIMAP::FetchJob::messagesAvailable, this, &FetchJobTest::onMessagesAvailable);
        bool result = job->exec();

        QVERIFY(result);
        QVERIFY(m_signals.count() > 0);
        QCOMPARE(m_uids.count(), 1);
        QCOMPARE(m_parts.count(), 1);
        QCOMPARE(m_attrs.count(), 0);
        QCOMPARE(m_msgs.count(), 1);

        // Check that we received the message header
        QCOMPARE(m_messages[2]->messageID()->identifier(), QByteArray("1234@example.com"));
        QCOMPARE(m_msgs[2].message->messageID()->identifier(), QByteArray("1234@example.com"));

        // Check that we received the flags
        QMap<qint64, KIMAP::MessageFlags> expectedFlags;
        expectedFlags.insert(2, KIMAP::MessageFlags() << "\\Seen");
        QCOMPARE(m_flags, expectedFlags);
        QCOMPARE(m_msgs[2].flags, expectedFlags[2]);

        // Check that we didn't received the full message body, since we only requested a specific part
        QCOMPARE(m_messages[2]->decodedText().length(), 0);
        QCOMPARE(m_messages[2]->attachments().count(), 0);
        QCOMPARE(m_msgs[2].message->decodedText().length(), 0);
        QCOMPARE(m_msgs[2].message->attachments().count(), 0);

        // Check that we received the part we requested
        QByteArray partId = m_parts[2].keys().first();
        QString text = m_parts[2].value(partId)->decodedText(true, true);
        QCOMPARE(partId, QByteArray("1.1.1"));
        QCOMPARE(text, QStringLiteral("Hi Jane, nice to meet you!"));

        QCOMPARE(m_msgs[2].parts.keys().first(), QByteArray("1.1.1"));
        QCOMPARE(m_msgs[2].parts.value(partId)->decodedText(true, true), QStringLiteral("Hi Jane, nice to meet you!"));

        fakeServer.quit();

        m_signals.clear();
        m_uids.clear();
        m_sizes.clear();
        m_flags.clear();
        m_messages.clear();
        m_parts.clear();
        m_attrs.clear();
        m_msgs.clear();
    }
};

QTEST_GUILESS_MAIN(FetchJobTest)

#include "fetchjobtest.moc"
