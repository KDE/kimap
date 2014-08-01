/*
   Copyright (C) 2013 Christian Mollekopf <mollekopf@kolabsys.com>

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

#include <qtest.h>

#include "imapstreamparser.h"
#include <message_p.h>

#include <QLocalSocket>
#include <QtTest>
#include <QDebug>

using namespace KIMAP;

class StreamParserTest: public QObject
{
    Q_OBJECT

    QByteArray part1;
    QByteArray part2;
    QByteArray part3;
    QByteArray part4;
    QByteArray completeMessage;
    QList<QByteArray> expectedList;

private Q_SLOTS:

    void init()
    {

        part1 = "* 230 FETCH (FLAGS (\\Recent \\Seen) UID 230 INTERNALDATE \" 1-Nov-2013 13:31:17 +0100\" RFC822.SIZE 37 BODY[] {37}\r";
        part2 = "\nDate: Fri, 01 Nov 2013 12:31:13 +0000\n";
        part3 = "body\n";
        part4 = ")\n\r";
        completeMessage = part1 + part2 + part3 + part4;
        expectedList.clear();
        expectedList << "FLAGS";
        expectedList << "(\\Recent \\Seen)";
        expectedList << "UID";
        expectedList << "230";
        expectedList << "INTERNALDATE";
        expectedList << " 1-Nov-2013 13:31:17 +0100";
        expectedList << "RFC822.SIZE";
        expectedList << "37";
        expectedList << "BODY[]";
        expectedList << "Date: Fri, 01 Nov 2013 12:31:13 +0000";
        expectedList << "body";
    }

    /**
    * Test parsing of the example command if the complete command is in the buffer
    */
    void testParse()
    {
        QByteArray buffer;
        QBuffer socket(&buffer);
        socket.open(QBuffer::WriteOnly);
        QVERIFY(socket.write(completeMessage) != -1);

        QBuffer readSocket(&buffer);
        readSocket.open(QBuffer::ReadOnly);
        ImapStreamParser parser(&readSocket);

        QVERIFY(parser.availableDataSize() != 0);

        Message message;
        QList<Message::Part> *payload = &message.content;
        QVERIFY(!parser.atCommandEnd());
        QVERIFY(parser.hasString());
        *payload << Message::Part(parser.readString());   //*
        QVERIFY(parser.hasString());
        *payload << Message::Part(parser.readString());   //230
        QVERIFY(parser.hasString());
        *payload << Message::Part(parser.readString());   //FETCH
        QVERIFY(parser.hasList());
        *payload << Message::Part(parser.readParenthesizedList());
        QVERIFY(parser.atCommandEnd());

        QCOMPARE(message.content.last().toList(), expectedList);
    }

    void testLeadingNewline()
    {
        /**
        * Test a the special case when the CRLF after the string octet count, is separated and not initially loaded into the buffer.
        */
        QByteArray buffer;
        QBuffer socket(&buffer);
        socket.open(QBuffer::WriteOnly);

        QVERIFY(socket.write(part1) != -1);

        QBuffer readSocket(&buffer);
        readSocket.open(QBuffer::ReadOnly);
        ImapStreamParser parser(&readSocket);

        QVERIFY(parser.availableDataSize() != 0);

        Message message;
        QList<Message::Part> *payload = &message.content;
        QVERIFY(!parser.atCommandEnd());
        //We wait with writing part2 until the first part is already loaded into the buffer
        QVERIFY(socket.write(part2) != -1);
        QVERIFY(parser.hasString());
        *payload << Message::Part(parser.readString());   //*
        QVERIFY(parser.hasString());
        *payload << Message::Part(parser.readString());   //230
        QVERIFY(parser.hasString());
        *payload << Message::Part(parser.readString());   //FETCH

        QVERIFY(socket.write(part3) != -1);
        QVERIFY(socket.write(part4) != -1);

        QVERIFY(parser.hasList());
        *payload << Message::Part(parser.readParenthesizedList());
        QVERIFY(parser.atCommandEnd());
    }

    void testNewLineInString()
    {
        QByteArray command2;
        command2 = "* 33 FETCH (FLAGS (\\Recent \\Seen) UID 33 INTERNALDATE \" 1-Nov-2013 18:43:34 +0100\" RFC822.SIZE 1203 BODY[HEADER.FIELDS (TO FROM MESSAGE-ID REFERENCES IN-REPLY-TO SUBJECT DATE)] {54}\r\n";
        command2 += "Date: Fri, 01 Nov 2013 17:43:29 +0000\n";
        command2 += "Subject: uid32\n";
        command2 += "\n";
        command2 += ")\n\r";

        QByteArray buffer;
        QBuffer socket(&buffer);
        socket.open(QBuffer::WriteOnly);
        QVERIFY(socket.write(command2) != -1);

        QBuffer readSocket(&buffer);
        readSocket.open(QBuffer::ReadOnly);
        ImapStreamParser parser(&readSocket);

        QVERIFY(parser.availableDataSize() != 0);

        Message message;
        QList<Message::Part> *payload = &message.content;
        QVERIFY(!parser.atCommandEnd());
        QVERIFY(parser.hasString());
        *payload << Message::Part(parser.readString());   //*
        QVERIFY(parser.hasString());
        *payload << Message::Part(parser.readString());   //33
        QVERIFY(parser.hasString());
        *payload << Message::Part(parser.readString());   //FETCH
        QVERIFY(parser.hasList());
        *payload << Message::Part(parser.readParenthesizedList());
        QVERIFY(parser.atCommandEnd());
    }

};

QTEST_GUILESS_MAIN(StreamParserTest)

#include "streamparsertest.moc"
