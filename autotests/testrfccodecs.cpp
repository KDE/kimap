/*
   This file is part of the kimap library.
   SPDX-FileCopyrightText: 2007 Tom Albers <tomalbers@kde.nl>
   SPDX-FileCopyrightText: 2007 Allen Winter <winter@kde.org>

   SPDX-License-Identifier: LGPL-2.0-only
*/
#include <QTest>

#include "testrfccodecs.h"

QTEST_GUILESS_MAIN(RFCCodecsTest)

#include "rfccodecs.h"
using namespace KIMAP;

void RFCCodecsTest::testIMAPEncoding()
{
    QString encoded, decoded;
    QByteArray bEncoded, bDecoded;

    encoded = encodeImapFolderName(QString::fromUtf8("Test.Frode Rønning"));
    QCOMPARE(encoded, QString::fromUtf8("Test.Frode R&APg-nning"));
    bEncoded = encodeImapFolderName(QString::fromUtf8("Test.Frode Rønning").toUtf8());
    QCOMPARE(bEncoded, QString::fromUtf8("Test.Frode R&APg-nning").toUtf8());

    decoded = decodeImapFolderName(QString::fromLatin1("Test.Frode R&APg-nning"));
    QCOMPARE(decoded, QString::fromUtf8("Test.Frode Rønning"));
    bDecoded = decodeImapFolderName(QString::fromUtf8("Test.Frode Rønning").toUtf8());
    QCOMPARE(bDecoded, QString::fromUtf8("Test.Frode Rønning").toUtf8());

    encoded = encodeImapFolderName(QString::fromUtf8("Test.tom & jerry"));
    QCOMPARE(encoded, QString::fromUtf8("Test.tom &- jerry"));
    bEncoded = encodeImapFolderName(QString::fromUtf8("Test.tom & jerry").toUtf8());
    QCOMPARE(bEncoded, QString::fromUtf8("Test.tom &- jerry").toUtf8());

    decoded = decodeImapFolderName(QString::fromUtf8("Test.tom &- jerry"));
    QCOMPARE(decoded, QString::fromUtf8("Test.tom & jerry"));
    bDecoded = decodeImapFolderName(QString::fromUtf8("Test.tom &- jerry").toUtf8());
    QCOMPARE(bDecoded, QString::fromUtf8("Test.tom & jerry").toUtf8());

    // Try to feed already encoded
    encoded = encodeImapFolderName(QString::fromUtf8("Test.Cl&AOE-udio"));
    QCOMPARE(encoded, QString::fromUtf8("Test.Cl&-AOE-udio"));
    bEncoded = encodeImapFolderName(QString::fromUtf8("Test.Cl&AOE-udio").toUtf8());
    QCOMPARE(bEncoded, QString::fromUtf8("Test.Cl&-AOE-udio").toUtf8());

    decoded = decodeImapFolderName(QString::fromUtf8("Test.Cl&-AOE-udio"));
    QCOMPARE(decoded, QString::fromUtf8("Test.Cl&AOE-udio"));
    bDecoded = decodeImapFolderName(QString::fromUtf8("Test.Cl&-AOE-udio").toUtf8());
    QCOMPARE(bDecoded, QString::fromUtf8("Test.Cl&AOE-udio").toUtf8());

    // With UTF8 characters
    bEncoded = "INBOX/&AOQ- &APY- &APw- @ &IKw-";
    QCOMPARE(decodeImapFolderName(bEncoded), QByteArray("INBOX/ä ö ü @ €"));
}

void RFCCodecsTest::testQuotes()
{
    QString test(QStringLiteral("tom\"allen"));
    QCOMPARE(quoteIMAP(test), QString::fromLatin1("tom\\\"allen"));
    test = QStringLiteral("tom\'allen");
    QCOMPARE(quoteIMAP(test), QString::fromLatin1("tom\'allen"));
    test = QStringLiteral("tom\\allen");
    QCOMPARE(quoteIMAP(test), QString::fromLatin1("tom\\\\allen"));
}
