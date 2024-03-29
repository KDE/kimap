/*
   SPDX-FileCopyrightText: 2013 Christian Mollekopf <mollekopf@kolabsys.com>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <QTest>

#include "kimap/getmetadatajob.h"
#include "kimap/session.h"
#include "kimaptest/fakeserver.h"

#include <QDebug>
#include <QTest>

using MAP = QMap<QByteArray, QByteArray>;
Q_DECLARE_METATYPE(MAP)

class GetMetadataJobTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    void metadata_data()
    {
        QTest::addColumn<QList<QByteArray>>("scenario");
        QTest::addColumn<QString>("mailbox");
        QTest::addColumn<QMap<QByteArray, QByteArray>>("expectedAnnotations");

        {
            // FIXME requesting /shared and getting /private back doesn't make sense => fix scenario
            QList<QByteArray> scenario;
            scenario << FakeServer::preauth() << "C: A000001 GETMETADATA (DEPTH infinity) \"Folder1\" (/shared)"
                     << R"(S: * METADATA "Folder1" (/shared/comment "Shared comment"))"
                     << R"(S: * METADATA "Folder1" (/private/comment "My own comment"))"
                     << "S: A000001 OK GETMETADATA complete";
            QMap<QByteArray, QByteArray> expected;
            expected.insert("/shared/comment", "Shared comment");
            expected.insert("/private/comment", "My own comment");
            QTest::newRow("normal") << scenario << "Folder1" << expected;
        }
        {
            QList<QByteArray> scenario;
            scenario << FakeServer::preauth() << "C: A000001 GETMETADATA (DEPTH infinity) \"Folder1\" (/shared)"
                     << R"(S: * METADATA "Folder1" (/shared/comment "Shared comment" /private/comment "My own comment"))"
                     << "S: A000001 OK GETMETADATA complete";
            QMap<QByteArray, QByteArray> expected;
            expected.insert("/shared/comment", "Shared comment");
            expected.insert("/private/comment", "My own comment");
            QTest::newRow("combined response") << scenario << "Folder1" << expected;
        }
        {
            QList<QByteArray> scenario;
            scenario << FakeServer::preauth() << "C: A000001 GETMETADATA (DEPTH infinity) \"Folder1\" (/shared)"
                     << R"(S: * METADATA "Folder1" (/shared/comment "NIL" /private/comment "NIL"))"
                     << "S: A000001 OK GETMETADATA complete";
            QMap<QByteArray, QByteArray> expected;
            expected.insert("/shared/comment", "");
            expected.insert("/private/comment", "");
            QTest::newRow("NIL response") << scenario << "Folder1" << expected;
        }
    }

    void metadata()
    {
        QFETCH(QList<QByteArray>, scenario);
        QFETCH(QString, mailbox);
        QFETCH(MAP, expectedAnnotations);

        FakeServer fakeServer;
        fakeServer.setScenario(scenario);
        fakeServer.startAndWait();

        KIMAP::Session session(QLatin1StringView("127.0.0.1"), 5989);

        auto getMetadataJob = new KIMAP::GetMetaDataJob(&session);
        getMetadataJob->setServerCapability(KIMAP::MetaDataJobBase::Metadata);
        getMetadataJob->setMailBox(mailbox);
        getMetadataJob->setDepth(KIMAP::GetMetaDataJob::AllLevels);
        getMetadataJob->addRequestedEntry("/shared");

        QVERIFY(getMetadataJob->exec());

        QCOMPARE(getMetadataJob->allMetaData(mailbox).size(), expectedAnnotations.size());
        const QMap<QByteArray, QByteArray> &allMetaData = getMetadataJob->allMetaData();
        QCOMPARE(allMetaData.size(), expectedAnnotations.size());
        const auto keys = expectedAnnotations.keys();
        for (const QByteArray &entry : keys) {
            QCOMPARE(getMetadataJob->metaData(mailbox, entry), expectedAnnotations.value(entry));
            QCOMPARE(getMetadataJob->metaData(entry), expectedAnnotations.value(entry));
            QCOMPARE(allMetaData.value(entry), expectedAnnotations.value(entry));
        }

        fakeServer.quit();
    }

    void testMetadataParameter()
    {
        FakeServer fakeServer;
        QList<QByteArray> scenario;

        scenario << FakeServer::preauth() << "C: A000001 GETMETADATA \"Folder1\" (/shared)"
                 << "S: A000001 OK GETMETADATA complete"
                 << "C: A000002 GETMETADATA (DEPTH 1) \"Folder1\" (/shared)"
                 << "S: A000002 OK GETMETADATA complete"
                 << "C: A000003 GETMETADATA (MAXSIZE 1234) \"Folder1\" (/shared)"
                 << "S: A000003 OK GETMETADATA complete"
                 << "C: A000004 GETMETADATA (DEPTH 1 MAXSIZE 1234) \"Folder1\" (/shared)"
                 << "S: A000004 OK GETMETADATA complete"
                 << "C: A000005 GETMETADATA (DEPTH 1 MAXSIZE 1234) \"Folder1\" (/shared /shared2)"
                 << "S: A000005 OK GETMETADATA complete"
                 << "C: A000006 GETMETADATA (DEPTH 1 MAXSIZE 1234) \"Folder1\""
                 << "S: A000006 OK GETMETADATA complete"
                 << "C: A000007 GETMETADATA (DEPTH 1) \"Folder1\""
                 << "S: A000007 OK GETMETADATA complete"
                 << "C: A000008 GETMETADATA (MAXSIZE 1234) \"Folder1\""
                 << "S: A000008 OK GETMETADATA complete"
                 << "C: A000009 GETMETADATA \"Folder1\""
                 << "S: A000009 OK GETMETADATA complete"
                 << "C: A000010 GETMETADATA \"\""
                 << "S: A000010 OK GETMETADATA complete";
        fakeServer.setScenario(scenario);
        fakeServer.startAndWait();

        KIMAP::Session session(QStringLiteral("127.0.0.1"), 5989);

        // C: A000001 GETMETADATA "Folder1" (/shared)
        auto getMetadataJob = new KIMAP::GetMetaDataJob(&session);
        getMetadataJob->setServerCapability(KIMAP::MetaDataJobBase::Metadata);
        getMetadataJob->setMailBox(QStringLiteral("Folder1"));
        getMetadataJob->addRequestedEntry("/shared");
        getMetadataJob->setDepth(KIMAP::GetMetaDataJob::NoDepth);
        QVERIFY(getMetadataJob->exec());

        QCOMPARE(getMetadataJob->allMetaData(QLatin1StringView("Folder1")).size(), 0);

        // C: A000002 GETMETADATA "Folder1" (DEPTH 1) (/shared)
        getMetadataJob = new KIMAP::GetMetaDataJob(&session);
        getMetadataJob->setServerCapability(KIMAP::MetaDataJobBase::Metadata);
        getMetadataJob->setMailBox(QStringLiteral("Folder1"));
        getMetadataJob->addRequestedEntry("/shared");
        getMetadataJob->setDepth(KIMAP::GetMetaDataJob::OneLevel);
        QVERIFY(getMetadataJob->exec());

        // C: A000003 GETMETADATA "Folder1" (MAXSIZE 1234) (/shared)
        getMetadataJob = new KIMAP::GetMetaDataJob(&session);
        getMetadataJob->setServerCapability(KIMAP::MetaDataJobBase::Metadata);
        getMetadataJob->setMailBox(QStringLiteral("Folder1"));
        getMetadataJob->addRequestedEntry("/shared");
        getMetadataJob->setDepth(KIMAP::GetMetaDataJob::NoDepth);
        getMetadataJob->setMaximumSize(1234);
        QVERIFY(getMetadataJob->exec());

        // C: A000004 GETMETADATA "Folder1" (DEPTH 1) (MAXSIZE 1234) (/shared)
        getMetadataJob = new KIMAP::GetMetaDataJob(&session);
        getMetadataJob->setServerCapability(KIMAP::MetaDataJobBase::Metadata);
        getMetadataJob->setMailBox(QStringLiteral("Folder1"));
        getMetadataJob->addRequestedEntry("/shared");
        getMetadataJob->setDepth(KIMAP::GetMetaDataJob::OneLevel);
        getMetadataJob->setMaximumSize(1234);
        QVERIFY(getMetadataJob->exec());

        // C: A000005 GETMETADATA "Folder1" (DEPTH 1) (MAXSIZE 1234) (/shared /shared2)
        getMetadataJob = new KIMAP::GetMetaDataJob(&session);
        getMetadataJob->setServerCapability(KIMAP::MetaDataJobBase::Metadata);
        getMetadataJob->setMailBox(QStringLiteral("Folder1"));
        getMetadataJob->addRequestedEntry("/shared");
        getMetadataJob->addRequestedEntry("/shared2");
        getMetadataJob->setDepth(KIMAP::GetMetaDataJob::OneLevel);
        getMetadataJob->setMaximumSize(1234);
        QVERIFY(getMetadataJob->exec());

        // C: A000006 GETMETADATA "Folder1" (DEPTH 1) (MAXSIZE 1234)
        getMetadataJob = new KIMAP::GetMetaDataJob(&session);
        getMetadataJob->setServerCapability(KIMAP::MetaDataJobBase::Metadata);
        getMetadataJob->setMailBox(QStringLiteral("Folder1"));
        getMetadataJob->setDepth(KIMAP::GetMetaDataJob::OneLevel);
        getMetadataJob->setMaximumSize(1234);
        QVERIFY(getMetadataJob->exec());

        // C: A000007 GETMETADATA "Folder1" (DEPTH 1)
        getMetadataJob = new KIMAP::GetMetaDataJob(&session);
        getMetadataJob->setServerCapability(KIMAP::MetaDataJobBase::Metadata);
        getMetadataJob->setMailBox(QStringLiteral("Folder1"));
        getMetadataJob->setDepth(KIMAP::GetMetaDataJob::OneLevel);
        QVERIFY(getMetadataJob->exec());

        // C: A000008 GETMETADATA "Folder1" (MAXSIZE 1234)
        getMetadataJob = new KIMAP::GetMetaDataJob(&session);
        getMetadataJob->setServerCapability(KIMAP::MetaDataJobBase::Metadata);
        getMetadataJob->setMailBox(QStringLiteral("Folder1"));
        getMetadataJob->setDepth(KIMAP::GetMetaDataJob::NoDepth);
        getMetadataJob->setMaximumSize(1234);
        QVERIFY(getMetadataJob->exec());

        // C: A000009 GETMETADATA "Folder1"
        getMetadataJob = new KIMAP::GetMetaDataJob(&session);
        getMetadataJob->setServerCapability(KIMAP::MetaDataJobBase::Metadata);
        getMetadataJob->setMailBox(QStringLiteral("Folder1"));
        getMetadataJob->setDepth(KIMAP::GetMetaDataJob::NoDepth);
        QVERIFY(getMetadataJob->exec());

        // C: A000010 GETMETADATA ""
        getMetadataJob = new KIMAP::GetMetaDataJob(&session);
        getMetadataJob->setServerCapability(KIMAP::MetaDataJobBase::Metadata);
        getMetadataJob->setMailBox(QLatin1StringView(""));
        getMetadataJob->setDepth(KIMAP::GetMetaDataJob::NoDepth);
        QVERIFY(getMetadataJob->exec());

        QVERIFY(fakeServer.isAllScenarioDone());
        fakeServer.quit();
    }

    void annotatemore_data()
    {
        QTest::addColumn<QList<QByteArray>>("scenario");
        QTest::addColumn<QString>("mailbox");
        QTest::addColumn<QMap<QByteArray, QByteArray>>("expectedAnnotations");
        QTest::addColumn<QByteArray>("entry");

        {
            QList<QByteArray> scenario;
            scenario << FakeServer::preauth() << R"(C: A000001 GETANNOTATION "Folder1" "*" "value.shared")"
                     << "S: * ANNOTATION Folder1 /comment (value.shared \"Shared comment\")"
                     << "S: * ANNOTATION Folder1 /comment (value.priv \"My own comment\")"
                     << "S: A000001 OK annotations retrieved";

            QMap<QByteArray, QByteArray> expected;
            expected.insert("/shared/comment", "Shared comment");
            expected.insert("/private/comment", "My own comment");
            QTest::newRow("get all") << scenario << "Folder1" << expected << QByteArray("/shared*");
        }
        {
            QList<QByteArray> scenario;
            scenario << FakeServer::preauth() << R"(C: A000001 GETANNOTATION "Folder1" "/comment" "value.shared")"
                     << "S: * ANNOTATION Folder1 /comment (value.shared \"Shared comment\")"
                     << "S: * ANNOTATION Folder1 /comment (value.priv \"My own comment\")"
                     << "S: A000001 OK annotations retrieved";

            QMap<QByteArray, QByteArray> expected;
            expected.insert("/shared/comment", "Shared comment");
            expected.insert("/private/comment", "My own comment");
            QTest::newRow("get single") << scenario << "Folder1" << expected << QByteArray("/shared/comment");
        }
    }

    void annotatemore()
    {
        QFETCH(QList<QByteArray>, scenario);
        QFETCH(QString, mailbox);
        QFETCH(MAP, expectedAnnotations);
        QFETCH(QByteArray, entry);

        FakeServer fakeServer;
        fakeServer.setScenario(scenario);
        fakeServer.startAndWait();

        KIMAP::Session session(QStringLiteral("127.0.0.1"), 5989);

        auto getMetadataJob = new KIMAP::GetMetaDataJob(&session);
        getMetadataJob->setServerCapability(KIMAP::MetaDataJobBase::Annotatemore);
        getMetadataJob->setMailBox(mailbox);
        getMetadataJob->addRequestedEntry(entry);

        QVERIFY(getMetadataJob->exec());

        //   qDebug() << getMetadataJob->allMetaData(mailbox);
        qDebug() << getMetadataJob->allMetaData();
        const QMap<QByteArray, QByteArray> &allMetaData = getMetadataJob->allMetaData();
        QCOMPARE(allMetaData.size(), expectedAnnotations.size());
        const auto keys = expectedAnnotations.keys();
        for (const QByteArray &e : keys) {
            QCOMPARE(getMetadataJob->metaData(e), expectedAnnotations.value(e));
            QCOMPARE(allMetaData.value(e), expectedAnnotations.value(e));
        }

        fakeServer.quit();
    }

    void testAnnotateEntires()
    {
        FakeServer fakeServer;
        QList<QByteArray> scenario;

        scenario << FakeServer::preauth() << "C: A000001 GETANNOTATION \"Folder1\""
                 << "S: A000001 OK annotations retrieved"
                 << R"(C: A000002 GETANNOTATION "Folder1" ("/comment" "/motd") ("value.priv" "value.shared"))"
                 << "S: A000002 OK annotations retrieved";
        fakeServer.setScenario(scenario);
        fakeServer.startAndWait();

        KIMAP::Session session(QStringLiteral("127.0.0.1"), 5989);

        // C: A000001 GETANNOTATION "Folder1"
        auto getMetadataJob = new KIMAP::GetMetaDataJob(&session);
        getMetadataJob->setServerCapability(KIMAP::MetaDataJobBase::Annotatemore);
        getMetadataJob->setMailBox(QStringLiteral("Folder1"));
        QVERIFY(getMetadataJob->exec());

        QCOMPARE(getMetadataJob->allMetaData(QLatin1StringView("Folder1")).size(), 0);

        // C: A000002 GETANNOTATION "Folder1" ("/comment" "/motd") ("value.shared" "value.priv")
        getMetadataJob = new KIMAP::GetMetaDataJob(&session);
        getMetadataJob->setServerCapability(KIMAP::MetaDataJobBase::Annotatemore);
        getMetadataJob->setMailBox(QStringLiteral("Folder1"));
        getMetadataJob->addRequestedEntry("/shared/comment");
        getMetadataJob->addRequestedEntry("/private/motd");
        QVERIFY(getMetadataJob->exec());

        QVERIFY(fakeServer.isAllScenarioDone());
        fakeServer.quit();
    }

    void testAnnotateMutiple()
    {
        // Do not double same parts of the request
        FakeServer fakeServer;
        QList<QByteArray> scenario;

        scenario << FakeServer::preauth() << R"(C: A000001 GETANNOTATION "Folder1" ("/comment" "/motd") "value.shared")"
                 << "S: A000001 OK annotations retrieved"
                 << R"(C: A000002 GETANNOTATION "Folder1" "/comment" ("value.priv" "value.shared"))"
                 << "S: A000002 OK annotations retrieved";
        fakeServer.setScenario(scenario);
        fakeServer.startAndWait();

        KIMAP::Session session(QStringLiteral("127.0.0.1"), 5989);

        // C: A000001 GETANNOTATION "Folder1" ("/comment" "/motd") "value.shared"
        auto getMetadataJob = new KIMAP::GetMetaDataJob(&session);
        getMetadataJob->setServerCapability(KIMAP::MetaDataJobBase::Annotatemore);
        getMetadataJob->setMailBox(QStringLiteral("Folder1"));
        getMetadataJob->addRequestedEntry("/shared/comment");
        getMetadataJob->addRequestedEntry("/shared/motd");
        QVERIFY(getMetadataJob->exec());

        // C: A000002 GETANNOTATION "Folder1" ("/comment") ("value.shared" "value.priv")
        getMetadataJob = new KIMAP::GetMetaDataJob(&session);
        getMetadataJob->setServerCapability(KIMAP::MetaDataJobBase::Annotatemore);
        getMetadataJob->setMailBox(QStringLiteral("Folder1"));
        getMetadataJob->addRequestedEntry("/shared/comment");
        getMetadataJob->addRequestedEntry("/private/comment");
        QVERIFY(getMetadataJob->exec());

        QVERIFY(fakeServer.isAllScenarioDone());
        fakeServer.quit();
    }
};

QTEST_GUILESS_MAIN(GetMetadataJobTest)

#include "getmetadatajobtest.moc"
