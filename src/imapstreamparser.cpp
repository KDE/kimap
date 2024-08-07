/*
    SPDX-FileCopyrightText: 2006-2007 Volker Krause <vkrause@kde.org>
    SPDX-FileCopyrightText: 2009 Andras Mantia <amantia@kde.org>

    SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
    SPDX-FileContributor: Kevin Ottens <kevin@kdab.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "imapstreamparser.h"

#include <QIODevice>
#include <ctype.h>

using namespace KIMAP;

ImapStreamParser::ImapStreamParser(QIODevice *socket, bool serverModeEnabled)
    : m_position(0)
    , m_literalSize(0)
{
    m_socket = socket;
    m_isServerModeEnabled = serverModeEnabled;
}

QString ImapStreamParser::readUtf8String()
{
    QByteArray tmp;
    tmp = readString();
    QString result = QString::fromUtf8(tmp);
    return result;
}

QByteArray ImapStreamParser::readString()
{
    QByteArray result;
    if (!waitForMoreData(m_data.isEmpty())) {
        throw ImapParserException("Unable to read more data");
    }
    stripLeadingSpaces();
    if (!waitForMoreData(m_position >= m_data.length())) {
        throw ImapParserException("Unable to read more data");
    }

    // literal string
    // TODO: error handling
    if (hasLiteral()) {
        while (!atLiteralEnd()) {
            result += readLiteralPart();
        }
        return result;
    }

    // quoted string
    return parseQuotedString();
}

bool ImapStreamParser::hasString()
{
    if (!waitForMoreData(m_position >= m_data.length())) {
        throw ImapParserException("Unable to read more data");
    }
    int savedPos = m_position;
    stripLeadingSpaces();
    int pos = m_position;
    m_position = savedPos;
    const char dataChar = m_data.at(pos);
    if (dataChar == '{') {
        return true; // literal string
    } else if (dataChar == '"') {
        return true; // quoted string
    } else if (dataChar != ' ' && dataChar != '(' && dataChar != ')' && dataChar != '[' && dataChar != ']' && dataChar != '\n' && dataChar != '\r') {
        return true; // unquoted string
    }

    return false; // something else, not a string
}

bool ImapStreamParser::hasLiteral()
{
    if (!waitForMoreData(m_position >= m_data.length())) {
        throw ImapParserException("Unable to read more data");
    }
    int savedPos = m_position;
    stripLeadingSpaces();
    if (m_data.at(m_position) == '{') {
        int end = -1;
        do {
            end = m_data.indexOf('}', m_position);
            if (!waitForMoreData(end == -1)) {
                throw ImapParserException("Unable to read more data");
            }
        } while (end == -1);
        Q_ASSERT(end > m_position);
        m_literalSize = m_data.mid(m_position + 1, end - m_position - 1).toInt();
        // strip CRLF
        m_position = end + 1;
        // ensure that the CRLF is available
        if (!waitForMoreData(m_position + 1 >= m_data.length())) {
            throw ImapParserException("Unable to read more data");
        }
        if (m_position < m_data.length() && m_data.at(m_position) == '\r') {
            ++m_position;
        }
        if (m_position < m_data.length() && m_data.at(m_position) == '\n') {
            ++m_position;
        }

        // FIXME: Makes sense only on the server side?
        if (m_isServerModeEnabled && m_literalSize > 0) {
            sendContinuationResponse(m_literalSize);
        }
        return true;
    } else {
        m_position = savedPos;
        return false;
    }
}

bool ImapStreamParser::atLiteralEnd() const
{
    return (m_literalSize == 0);
}

QByteArray ImapStreamParser::readLiteralPart()
{
    static const qint64 maxLiteralPartSize = 4096;
    int size = qMin(maxLiteralPartSize, m_literalSize);

    if (!waitForMoreData(m_data.length() < m_position + size)) {
        throw ImapParserException("Unable to read more data");
    }

    if (m_data.length() < m_position + size) { // Still not enough data
        // Take what's already there
        size = m_data.length() - m_position;
    }

    QByteArray result = m_data.mid(m_position, size);
    m_position += size;
    m_literalSize -= size;
    Q_ASSERT(m_literalSize >= 0);
    trimBuffer();

    return result;
}

bool ImapStreamParser::hasList()
{
    if (!waitForMoreData(m_position >= m_data.length())) {
        throw ImapParserException("Unable to read more data");
    }
    int savedPos = m_position;
    stripLeadingSpaces();
    int pos = m_position;
    m_position = savedPos;
    if (m_data.at(pos) == '(') {
        return true;
    }
    return false;
}

bool ImapStreamParser::atListEnd()
{
    if (!waitForMoreData(m_position >= m_data.length())) {
        throw ImapParserException("Unable to read more data");
    }
    int savedPos = m_position;
    stripLeadingSpaces();
    int pos = m_position;
    m_position = savedPos;
    if (m_data.at(pos) == ')') {
        m_position = pos + 1;
        return true;
    }
    return false;
}

QList<QByteArray> ImapStreamParser::readParenthesizedList()
{
    QList<QByteArray> result;
    if (!waitForMoreData(m_data.length() <= m_position)) {
        throw ImapParserException("Unable to read more data");
    }

    stripLeadingSpaces();
    if (m_data.at(m_position) != '(') {
        return result; // no list found
    }

    bool concatToLast = false;
    int count = 0;
    int sublistbegin = m_position;
    int i = m_position + 1;
    for (;;) {
        if (!waitForMoreData(m_data.length() <= i)) {
            m_position = i;
            throw ImapParserException("Unable to read more data");
        }
        if (m_data.at(i) == '(') {
            ++count;
            if (count == 1) {
                sublistbegin = i;
            }
            ++i;
            continue;
        }
        if (m_data.at(i) == ')') {
            if (count <= 0) {
                m_position = i + 1;
                return result;
            }
            if (count == 1) {
                result.append(m_data.mid(sublistbegin, i - sublistbegin + 1));
            }
            --count;
            ++i;
            continue;
        }
        if (m_data.at(i) == ' ') {
            ++i;
            continue;
        }
        if (m_data.at(i) == '"') {
            if (count > 0) {
                m_position = i;
                parseQuotedString();
                i = m_position;
                continue;
            }
        }
        if (m_data.at(i) == '[') {
            concatToLast = true;
            if (result.isEmpty()) {
                result.append(QByteArray());
            }
            result.last() += '[';
            ++i;
            continue;
        }
        if (m_data.at(i) == ']') {
            concatToLast = false;
            result.last() += ']';
            ++i;
            continue;
        }
        if (count == 0) {
            m_position = i;
            QByteArray ba;
            if (hasLiteral()) {
                while (!atLiteralEnd()) {
                    ba += readLiteralPart();
                }
            } else {
                ba = readString();
            }

            // We might sometime get some unwanted CRLF, but we're still not at the end
            // of the list, would make further string reads fail so eat the CRLFs.
            while ((m_position < m_data.size()) && (m_data.at(m_position) == '\r' || m_data.at(m_position) == '\n')) {
                m_position++;
            }

            i = m_position - 1;
            if (concatToLast) {
                result.last() += ba;
            } else {
                result.append(ba);
            }
        }
        ++i;
    }

    throw ImapParserException("Something went very very wrong!");
}

bool ImapStreamParser::hasResponseCode()
{
    if (!waitForMoreData(m_position >= m_data.length())) {
        throw ImapParserException("Unable to read more data");
    }
    int savedPos = m_position;
    stripLeadingSpaces();
    int pos = m_position;
    m_position = savedPos;
    if (m_data.at(pos) == '[') {
        m_position = pos + 1;
        return true;
    }
    return false;
}

bool ImapStreamParser::atResponseCodeEnd()
{
    if (!waitForMoreData(m_position >= m_data.length())) {
        throw ImapParserException("Unable to read more data");
    }
    int savedPos = m_position;
    stripLeadingSpaces();
    int pos = m_position;
    m_position = savedPos;
    if (m_data.at(pos) == ']') {
        m_position = pos + 1;
        return true;
    }
    return false;
}

QByteArray ImapStreamParser::parseQuotedString()
{
    QByteArray result;
    if (!waitForMoreData(m_data.length() == 0)) {
        throw ImapParserException("Unable to read more data");
    }
    stripLeadingSpaces();
    int end = m_position;
    result.clear();
    if (!waitForMoreData(m_position >= m_data.length())) {
        throw ImapParserException("Unable to read more data");
    }
    if (!waitForMoreData(m_position >= m_data.length())) {
        throw ImapParserException("Unable to read more data");
    }

    bool foundSlash = false;
    // quoted string
    if (m_data.at(m_position) == '"') {
        ++m_position;
        int i = m_position;
        for (;;) {
            if (!waitForMoreData(m_data.length() <= i)) {
                m_position = i;
                throw ImapParserException("Unable to read more data");
            }
            if (m_data.at(i) == '\\') {
                i += 2;
                foundSlash = true;
                continue;
            }
            if (m_data.at(i) == '"') {
                result = m_data.mid(m_position, i - m_position);
                end = i + 1; // skip the '"'
                break;
            }
            ++i;
        }
    }

    // unquoted string
    else {
        bool reachedInputEnd = true;
        int i = m_position;
        for (;;) {
            if (!waitForMoreData(m_data.length() <= i)) {
                m_position = i;
                throw ImapParserException("Unable to read more data");
            }
            if (m_data.at(i) == ' ' || m_data.at(i) == '(' || m_data.at(i) == ')' || m_data.at(i) == '[' || m_data.at(i) == ']' || m_data.at(i) == '\n'
                || m_data.at(i) == '\r' || m_data.at(i) == '"') {
                end = i;
                reachedInputEnd = false;
                break;
            }
            if (m_data.at(i) == '\\') {
                foundSlash = true;
            }
            i++;
        }
        if (reachedInputEnd) { // FIXME: how can it get here?
            end = m_data.length();
        }

        result = m_data.mid(m_position, end - m_position);
    }

    // strip quotes
    if (foundSlash) {
        while (result.contains("\\\"")) {
            result.replace("\\\"", "\"");
        }
        while (result.contains("\\\\")) {
            result.replace("\\\\", "\\");
        }
    }
    m_position = end;
    return result;
}

qint64 ImapStreamParser::readNumber(bool *ok)
{
    qint64 result;
    if (ok) {
        *ok = false;
    }
    if (!waitForMoreData(m_data.length() == 0)) {
        throw ImapParserException("Unable to read more data");
    }
    stripLeadingSpaces();
    if (!waitForMoreData(m_position >= m_data.length())) {
        throw ImapParserException("Unable to read more data");
    }
    if (m_position >= m_data.length()) {
        throw ImapParserException("Unable to read more data");
    }
    int i = m_position;
    for (;;) {
        if (!waitForMoreData(m_data.length() <= i)) {
            m_position = i;
            throw ImapParserException("Unable to read more data");
        }
        if (!isdigit(m_data.at(i))) {
            break;
        }
        ++i;
    }
    const auto tmp = QByteArrayView(m_data).mid(m_position, i - m_position);
    result = tmp.toLongLong(ok);
    m_position = i;
    return result;
}

void ImapStreamParser::stripLeadingSpaces()
{
    for (int i = m_position; i < m_data.length(); ++i) {
        if (m_data.at(i) != ' ') {
            m_position = i;
            return;
        }
    }
    m_position = m_data.length();
}

bool ImapStreamParser::waitForMoreData(bool wait)
{
    if (wait) {
        if (m_socket->bytesAvailable() > 0 || m_socket->waitForReadyRead(30000)) {
            m_data.append(m_socket->readAll());
        } else {
            return false;
        }
    }
    return true;
}

void ImapStreamParser::setData(const QByteArray &data)
{
    m_data = data;
}

QByteArray ImapStreamParser::readRemainingData()
{
    return m_data.mid(m_position);
}

int ImapStreamParser::availableDataSize() const
{
    return m_socket->bytesAvailable() + m_data.size() - m_position;
}

bool ImapStreamParser::atCommandEnd()
{
    int savedPos = m_position;
    do {
        if (!waitForMoreData(m_position >= m_data.length())) {
            throw ImapParserException("Unable to read more data");
        }
        stripLeadingSpaces();
    } while (m_position >= m_data.size());

    if (m_data.at(m_position) == '\n' || m_data.at(m_position) == '\r') {
        if (m_data.at(m_position) == '\r') {
            ++m_position;
        }
        if (m_position < m_data.length() && m_data.at(m_position) == '\n') {
            ++m_position;
        }

        // We'd better empty m_data from time to time before it grows out of control
        trimBuffer();

        return true; // command end
    }
    m_position = savedPos;
    return false; // something else
}

QByteArray ImapStreamParser::readUntilCommandEnd()
{
    QByteArray result;
    int i = m_position;
    int paranthesisBalance = 0;
    for (;;) {
        if (!waitForMoreData(m_data.length() <= i)) {
            m_position = i;
            throw ImapParserException("Unable to read more data");
        }
        if (m_data.at(i) == '{') {
            m_position = i - 1;
            hasLiteral(); // init literal size
            result.append(QByteArrayView(m_data).mid(i, m_position + 1));
            while (!atLiteralEnd()) {
                result.append(readLiteralPart());
            }
            i = m_position;
        }
        if (m_data.at(i) == '(') {
            paranthesisBalance++;
        }
        if (m_data.at(i) == ')') {
            paranthesisBalance--;
        }
        if ((i == m_data.length() && paranthesisBalance == 0) || m_data.at(i) == '\n' || m_data.at(i) == '\r') {
            break; // command end
        }
        result.append(m_data.at(i));
        ++i;
    }
    m_position = i;
    atCommandEnd();
    return result;
}

void ImapStreamParser::sendContinuationResponse(qint64 size)
{
    QByteArray block = "+ Ready for literal data (expecting " + QByteArray::number(size) + " bytes)\r\n";
    m_socket->write(block);
    m_socket->waitForBytesWritten(30000);
}

void ImapStreamParser::trimBuffer()
{
    if (m_position < 4096) { // right() is expensive, so don't do it for every line
        return;
    }
    m_data = std::move(m_data).right(m_data.size() - m_position);
    m_position = 0;
}
