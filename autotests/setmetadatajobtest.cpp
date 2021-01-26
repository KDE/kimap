/*
   SPDX-FileCopyrightText: 2013 Christian Mollekopf <mollekopf@kolabsys.com>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <QTest>

#include "kimap/session.h"
#include "kimap/setmetadatajob.h"
#include "kimaptest/fakeserver.h"

#include <QTest>

using MAP = QMap<QByteArray, QByteArray>;
Q_DECLARE_METATYPE(MAP)

class SetMetadataJobTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    void metadata_data()
    {
        QTest::addColumn<QList<QByteArray>>("scenario");
        QTest::addColumn<QString>("mailbox");
        QTest::addColumn<QMap<QByteArray, QByteArray>>("annotations");

        {
            QList<QByteArray> scenario;
            scenario << FakeServer::preauth() << R"(C: A000001 SETMETADATA "Folder1" ("/public/comment" "comment2" "/shared/comment" "Shared comment"))"
                     << "S: A000001 OK SETMETADATA complete";
            QMap<QByteArray, QByteArray> annotations;
            annotations.insert("/public/comment", "comment2");
            annotations.insert("/shared/comment", "Shared comment");
            QTest::newRow("normal") << scenario << "Folder1" << annotations;
        }
        {
            QList<QByteArray> scenario;
            scenario << FakeServer::preauth()
                     << "C: A000001 SETMETADATA \"Folder1\" (\"/public/comment\" {8}\r\ncomment2 \"/shared/comment\" {14}\r\nShared\ncomment)"
                     << "S: A000001 OK SETMETADATA complete";
            QMap<QByteArray, QByteArray> annotations;
            annotations.insert("/shared/comment", "Shared\ncomment");
            annotations.insert("/public/comment", "comment2");
            QTest::newRow("newline") << scenario << "Folder1" << annotations;
        }
        {
            QList<QByteArray> scenario;
            scenario << FakeServer::preauth() << R"(C: A000001 SETMETADATA "Folder1" ("/shared/comment" NIL))"
                     << "S: A000001 OK SETMETADATA complete";
            QMap<QByteArray, QByteArray> annotations;
            annotations.insert("/shared/comment", "");
            QTest::newRow("newline") << scenario << "Folder1" << annotations;
        }
        {
            QList<QByteArray> scenario;
            scenario << FakeServer::preauth() << "C: A000001 SETMETADATA \"Folder1\" (\"/public/comment\" {12}\r\ncomment\ntest \"/shared/comment\" {3}\r\nNIL)"
                     << "S: A000001 OK SETMETADATA complete";
            QMap<QByteArray, QByteArray> annotations;
            annotations.insert("/shared/comment", "");
            annotations.insert("/public/comment", "comment\ntest");
            QTest::newRow("newline2") << scenario << "Folder1" << annotations;
        }
        {
            QList<QByteArray> scenario;
            scenario << FakeServer::preauth() << "C: A000001 SETMETADATA \"Folder1\" (\"/public/comment\" {3}\r\nNIL \"/shared/comment\" {12}\r\ncomment\ntest)"
                     << "S: A000001 OK SETMETADATA complete";
            QMap<QByteArray, QByteArray> annotations;
            annotations.insert("/shared/comment", "comment\ntest");
            annotations.insert("/public/comment", "");
            QTest::newRow("newline2") << scenario << "Folder1" << annotations;
        }
    }

    void metadata()
    {
        QFETCH(QList<QByteArray>, scenario);
        QFETCH(QString, mailbox);
        QFETCH(MAP, annotations);

        FakeServer fakeServer;
        fakeServer.setScenario(scenario);
        fakeServer.startAndWait();

        KIMAP::Session session(QStringLiteral("127.0.0.1"), 5989);

        auto setMetadataJob = new KIMAP::SetMetaDataJob(&session);
        setMetadataJob->setServerCapability(KIMAP::MetaDataJobBase::Metadata);
        setMetadataJob->setMailBox(mailbox);
        const auto keys = annotations.keys();
        for (const QByteArray &entry : keys) {
            setMetadataJob->addMetaData(entry, annotations[entry]);
        }

        QVERIFY(setMetadataJob->exec());

        fakeServer.quit();
    }

    void annotatemore_data()
    {
        QTest::addColumn<QList<QByteArray>>("scenario");
        QTest::addColumn<QString>("mailbox");
        QTest::addColumn<QMap<QByteArray, QByteArray>>("annotations");
        QTest::addColumn<bool>("legacyMode");

        {
            QList<QByteArray> scenario;
            scenario << FakeServer::preauth() << R"(C: A000001 SETANNOTATION "Folder1" "/comment" ("value.shared" "Shared comment"))"
                     << "S: A000001 OK annotations changed";

            QMap<QByteArray, QByteArray> annotations;
            annotations.insert("/comment", "Shared comment");
            QTest::newRow("normal") << scenario << "Folder1" << annotations << false;
            QTest::newRow("legacy") << scenario << "Folder1" << annotations << true;
        }
    }

    void annotatemore()
    {
        QFETCH(QList<QByteArray>, scenario);
        QFETCH(QString, mailbox);
        QFETCH(MAP, annotations);
        QFETCH(bool, legacyMode);

        FakeServer fakeServer;
        fakeServer.setScenario(scenario);
        fakeServer.startAndWait();

        KIMAP::Session session(QStringLiteral("127.0.0.1"), 5989);

        auto setMetadataJob = new KIMAP::SetMetaDataJob(&session);
        setMetadataJob->setServerCapability(KIMAP::MetaDataJobBase::Annotatemore);
        setMetadataJob->setMailBox(mailbox);
        const auto keys = annotations.keys();
        for (const QByteArray &entry : keys) {
            if (legacyMode) {
                setMetadataJob->setEntry(entry);
                setMetadataJob->addMetaData("value.shared", annotations[entry]);
            } else {
                setMetadataJob->addMetaData(QByteArray("/shared") + entry, annotations[entry]);
            }
        }

        QVERIFY(setMetadataJob->exec());

        fakeServer.quit();
    }
};

QTEST_GUILESS_MAIN(SetMetadataJobTest)

#include "setmetadatajobtest.moc"
