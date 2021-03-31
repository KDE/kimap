/*
    SPDX-FileCopyrightText: 2006-2007 Volker Krause <vkrause@kde.org>
    SPDX-FileCopyrightText: 2009 Andras Mantia <amantia@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "kimap_export.h"

#include <exception>

#include <QByteArray>
#include <QList>
#include <QString>

class QIODevice;

namespace KIMAP
{
class ImapParserException : public std::exception
{
public:
    explicit ImapParserException(const char *what) throw()
        : mWhat(what)
    {
    }
    explicit ImapParserException(const QByteArray &what) throw()
        : mWhat(what)
    {
    }
    explicit ImapParserException(const QString &what) throw()
        : mWhat(what.toUtf8())
    {
    }
    ImapParserException(const ImapParserException &other) throw()
        : std::exception(other)
        , mWhat(other.what())
    {
    }
    ~ImapParserException() throw() override
    {
    }
    const char *what() const throw() override
    {
        return mWhat.constData();
    }
    virtual const char *type() const throw()
    {
        return "ImapParserException";
    }

private:
    QByteArray mWhat;
};

/**
  Parser for IMAP messages that operates on a local socket stream.
*/
class KIMAP_EXPORT ImapStreamParser
{
public:
    /**
     * Construct the parser.
     * @param socket the local socket to work with.
     * @param serverModeEnabled true if the parser has to assume we're writing a server (e.g. sends
     * continuation message automatically)
     */
    explicit ImapStreamParser(QIODevice *socket, bool serverModeEnabled = false);

    /**
     * Get a string from the message. If the upcoming data is not a quoted string, unquoted string or a literal,
     * the behavior is undefined. Use @ref hasString to be sure a string comes. This call might block.
     * @return the next string from the message as an utf8 string
     */
    QString readUtf8String();

    /**
     * Same as above, but without decoding it to utf8.
     * @return the next string from the message
     */
    QByteArray readString();

    /**
     * Get he next parenthesized list. If the upcoming data is not a parenthesized list,
     * the behavior is undefined. Use @ref hasList to be sure a string comes. This call might block.
     * @return the next parenthesized list.
     */
    QList<QByteArray> readParenthesizedList();

    /**
     * Get the next data as a number. This call might block.
     * @param ok true if the data found was a number
     * @return the number
     */
    qint64 readNumber(bool *ok = nullptr);

    /**
     * Check if the next data is a string or not. This call might block.
     * @return true if a string follows
     */
    bool hasString();

    /**
     * Check if the next data is a literal data or not. If a literal is found, the
     * internal position pointer is set to the beginning of the literal data.
     * This call might block.
     * @return true if a literal follows
     */
    bool hasLiteral();

    /**
     * Read the next literal sequence. This might or might not be the full data. Example code to read a literal would be:
     * @code
     * ImapStreamParser parser;
     *  ...
     * if (parser.hasLiteral())
     * {
     *   while (!parser.atLiteralEnd())
     *   {
     *      QByteArray data = parser.readLiteralPart();
     *      // do something with the data
     *   }
     * }
     * @endcode
     *
     * This call might block.
     *
     * @return part of a literal data
     */
    QByteArray readLiteralPart();

    /**
     * Check if the literal data end was reached. See @ref hasLiteral and @ref readLiteralPart .
     * @return true if the literal was completely read.
     */
    bool atLiteralEnd() const;

    /**
     * Check if the next data is a parenthesized list. This call might block.
     * @return true if a parenthesized list comes.
     */
    bool hasList();

    /**
     * Check if the next data is a parenthesized list end. This call might block.
     * @return true if a parenthesized list end.
     */
    bool atListEnd();

    /**
     * Check if the next data is a response code. This call might block.
     * @return true if a response code comes.
     */
    bool hasResponseCode();

    /**
     * Check if the next data is a response code end. This call might block.
     * @return true if a response code end.
     */
    bool atResponseCodeEnd();

    /**
     * Check if the command end was reached
     * @return true if the end of command is reached
     */
    bool atCommandEnd();

    /**
     * Return everything that remained from the command.
     * @return the remaining command data
     */
    QByteArray readUntilCommandEnd();

    /**
     * Return all the data that was read from the socket, but not processed yet.
     * @return the remaining unprocessed data
     */
    QByteArray readRemainingData();

    int availableDataSize() const;

    void setData(const QByteArray &data);

private:
    void stripLeadingSpaces();
    QByteArray parseQuotedString();

    /**
     * If the condition is true, wait for more data to be available from the socket.
     * If no data comes after a timeout (30000ms), it aborts and returns false.
     * @param wait the condition
     * @return true if more data is available
     */
    bool waitForMoreData(bool wait);

    /**
     * Inform the client to send more literal data.
     */
    void sendContinuationResponse(qint64 size);

    /**
     * Remove already read data from the internal buffer if necessary.
     */
    void trimBuffer();

    QIODevice *m_socket = nullptr;
    bool m_isServerModeEnabled = false;
    QByteArray m_data;
    int m_position = -1;
    qint64 m_literalSize = -1;
};

}

