/*
    SPDX-FileCopyrightText: 2009 Kevin Ottens <ervin@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "fetchjob.h"

#include "kimap_debug.h"
#include <KLocalizedString>
#include <QTimer>

#include "job_p.h"
#include "response_p.h"
#include "session_p.h"

namespace KIMAP
{
class FetchJobPrivate : public JobPrivate
{
public:
    FetchJobPrivate(FetchJob *job, Session *session, const QString &name)
        : JobPrivate(session, name)
        , q(job)
    {
    }

    ~FetchJobPrivate()
    {
    }

    void parseBodyStructure(const QByteArray &structure, int &pos, KMime::Content *content);
    void parsePart(const QByteArray &structure, int &pos, KMime::Content *content);
    QByteArray parseString(const QByteArray &structure, int &pos);
    QByteArray parseSentence(const QByteArray &structure, int &pos);
    void skipLeadingSpaces(const QByteArray &structure, int &pos);

    void emitPendings()
    {
        if (pendingMsgs.isEmpty()) {
            return;
        }

        Q_EMIT q->messagesAvailable(pendingMsgs);

        if (!pendingParts.isEmpty()) {
            Q_EMIT q->partsReceived(selectedMailBox, pendingUids, pendingParts);
            Q_EMIT q->partsReceived(selectedMailBox, pendingUids, pendingAttributes, pendingParts);
        }
        if (!pendingSizes.isEmpty() || !pendingFlags.isEmpty() || !pendingMessages.isEmpty()) {
            Q_EMIT q->headersReceived(selectedMailBox, pendingUids, pendingSizes, pendingFlags, pendingMessages);
            Q_EMIT q->headersReceived(selectedMailBox, pendingUids, pendingSizes, pendingAttributes, pendingFlags, pendingMessages);
        }
        if (!pendingMessages.isEmpty()) {
            Q_EMIT q->messagesReceived(selectedMailBox, pendingUids, pendingMessages);
            Q_EMIT q->messagesReceived(selectedMailBox, pendingUids, pendingAttributes, pendingMessages);
        }

        pendingUids.clear();
        pendingMessages.clear();
        pendingParts.clear();
        pendingSizes.clear();
        pendingFlags.clear();
        pendingAttributes.clear();
        pendingMsgs.clear();
    }

    FetchJob *const q;

    ImapSet set;
    bool uidBased = false;
    FetchJob::FetchScope scope;
    QString selectedMailBox;
    bool gmailEnabled = false;

    QTimer emitPendingsTimer;
    QMap<qint64, MessagePtr> pendingMessages;
    QMap<qint64, MessageParts> pendingParts;
    QMap<qint64, MessageFlags> pendingFlags;
    QMap<qint64, MessageAttribute> pendingAttributes;
    QMap<qint64, qint64> pendingSizes;
    QMap<qint64, qint64> pendingUids;
    QMap<qint64, Message> pendingMsgs;
};
}

using namespace KIMAP;

FetchJob::FetchScope::FetchScope()
    : mode(FetchScope::Content)
    , changedSince(0)
    , qresync(false)
{
}

FetchJob::FetchJob(Session *session)
    : Job(*new FetchJobPrivate(this, session, i18n("Fetch")))
{
    Q_D(FetchJob);
    connect(&d->emitPendingsTimer, SIGNAL(timeout()), this, SLOT(emitPendings()));
}

void FetchJob::setSequenceSet(const ImapSet &set)
{
    Q_D(FetchJob);
    Q_ASSERT(!set.isEmpty());
    d->set = set;
}

ImapSet FetchJob::sequenceSet() const
{
    Q_D(const FetchJob);
    return d->set;
}

void FetchJob::setUidBased(bool uidBased)
{
    Q_D(FetchJob);
    d->uidBased = uidBased;
}

bool FetchJob::isUidBased() const
{
    Q_D(const FetchJob);
    return d->uidBased;
}

void FetchJob::setScope(const FetchScope &scope)
{
    Q_D(FetchJob);
    d->scope = scope;
}

FetchJob::FetchScope FetchJob::scope() const
{
    Q_D(const FetchJob);
    return d->scope;
}

bool FetchJob::setGmailExtensionsEnabled() const
{
    Q_D(const FetchJob);
    return d->gmailEnabled;
}

void FetchJob::setGmailExtensionsEnabled(bool enabled)
{
    Q_D(FetchJob);
    d->gmailEnabled = enabled;
}

QString FetchJob::mailBox() const
{
    Q_D(const FetchJob);
    return d->selectedMailBox;
}

void FetchJob::doStart()
{
    Q_D(FetchJob);

    d->set.optimize();
    QByteArray parameters = d->set.toImapSequenceSet() + ' ';
    Q_ASSERT(!parameters.trimmed().isEmpty());

    switch (d->scope.mode) {
    case FetchScope::Headers:
        if (d->scope.parts.isEmpty()) {
            parameters += "(RFC822.SIZE INTERNALDATE BODY.PEEK[HEADER.FIELDS (TO FROM MESSAGE-ID REFERENCES IN-REPLY-TO SUBJECT DATE)] FLAGS UID";
        } else {
            parameters += '(';
            for (const QByteArray &part : std::as_const(d->scope.parts)) {
                parameters += "BODY.PEEK[" + part + ".MIME] ";
            }
            parameters += "UID";
        }
        break;
    case FetchScope::Flags:
        parameters += "(FLAGS UID";
        break;
    case FetchScope::Structure:
        parameters += "(BODYSTRUCTURE UID";
        break;
    case FetchScope::Content:
        if (d->scope.parts.isEmpty()) {
            parameters += "(BODY.PEEK[] UID";
        } else {
            parameters += '(';
            for (const QByteArray &part : std::as_const(d->scope.parts)) {
                parameters += "BODY.PEEK[" + part + "] ";
            }
            parameters += "UID";
        }
        break;
    case FetchScope::Full:
        parameters += "(RFC822.SIZE INTERNALDATE BODY.PEEK[] FLAGS UID";
        break;
    case FetchScope::HeaderAndContent:
        if (d->scope.parts.isEmpty()) {
            parameters += "(BODY.PEEK[] FLAGS UID";
        } else {
            parameters += "(BODY.PEEK[HEADER.FIELDS (TO FROM MESSAGE-ID REFERENCES IN-REPLY-TO SUBJECT DATE)]";
            for (const QByteArray &part : std::as_const(d->scope.parts)) {
                parameters += " BODY.PEEK[" + part + ".MIME] BODY.PEEK[" + part + "]"; // krazy:exclude=doublequote_chars
            }
            parameters += " FLAGS UID";
        }
        break;
    case FetchScope::FullHeaders:
        parameters += "(RFC822.SIZE INTERNALDATE BODY.PEEK[HEADER] FLAGS UID";
        break;
    }

    if (d->gmailEnabled) {
        parameters += " X-GM-LABELS X-GM-MSGID X-GM-THRID";
    }
    parameters += ")";

    if (d->scope.changedSince > 0) {
        parameters += " (CHANGEDSINCE " + QByteArray::number(d->scope.changedSince);
        if (d->scope.qresync) {
            parameters += " VANISHED";
        }
        parameters += ")";
    }

    QByteArray command = "FETCH";
    if (d->uidBased) {
        command = "UID " + command;
    }

    d->emitPendingsTimer.start(100);
    d->selectedMailBox = d->m_session->selectedMailBox();
    d->tags << d->sessionInternal()->sendCommand(command, parameters);
}

void FetchJob::handleResponse(const Response &response)
{
    Q_D(FetchJob);

    // We can predict it'll be handled by handleErrorReplies() so stop
    // the timer now so that result() will really be the last emitted signal.
    if (!response.content.isEmpty() && d->tags.size() == 1 && d->tags.contains(response.content.first().toString())) {
        d->emitPendingsTimer.stop();
        d->emitPendings();
    }

    if (handleErrorReplies(response) == NotHandled) {
        if (response.content.size() == 4 && response.content[1].toString() == "VANISHED") {
            const auto vanishedSet = ImapSet::fromImapSequenceSet(response.content[3].toString());
            Q_EMIT messagesVanished(vanishedSet);
        } else if (response.content.size() == 4 && response.content[2].toString() == "FETCH" && response.content[3].type() == Response::Part::List) {
            const qint64 id = response.content[1].toString().toLongLong();
            const QList<QByteArray> content = response.content[3].toList();

            Message msg;
            MessagePtr message(new KMime::Message);
            bool shouldParseMessage = false;
            MessageParts parts;

            for (QList<QByteArray>::ConstIterator it = content.constBegin(); it != content.constEnd(); ++it) {
                QByteArray str = *it;
                ++it;

                if (it == content.constEnd()) { // Uh oh, message was truncated?
                    qCWarning(KIMAP_LOG) << "FETCH reply got truncated, skipping.";
                    break;
                }

                if (str == "UID") {
                    d->pendingUids[id] = msg.uid = it->toLongLong();
                } else if (str == "RFC822.SIZE") {
                    d->pendingSizes[id] = msg.size = it->toLongLong();
                } else if (str == "INTERNALDATE") {
                    message->date()->setDateTime(QDateTime::fromString(QLatin1String(*it), Qt::RFC2822Date));
                } else if (str == "FLAGS") {
                    if ((*it).startsWith('(') && (*it).endsWith(')')) {
                        QByteArray str = *it;
                        str.chop(1);
                        str.remove(0, 1);
                        const auto flags = str.split(' ');
                        d->pendingFlags[id] = flags;
                        msg.flags = flags;
                    } else {
                        d->pendingFlags[id] << *it;
                        msg.flags << *it;
                    }
                } else if (str == "X-GM-LABELS") {
                    d->pendingAttributes.insert(id, {"X-GM-LABELS", *it});
                    msg.attributes.insert("X-GM-LABELS", *it);
                } else if (str == "X-GM-THRID") {
                    d->pendingAttributes.insert(id, {"X-GM-THRID", *it});
                    msg.attributes.insert("X-GM-THRID", *it);
                } else if (str == "X-GM-MSGID") {
                    d->pendingAttributes.insert(id, {"X-GM-MSGID", *it});
                    msg.attributes.insert("X-GM-MSGID", *it);
                } else if (str == "BODYSTRUCTURE") {
                    int pos = 0;
                    d->parseBodyStructure(*it, pos, message.data());
                    message->assemble();
                    d->pendingMessages[id] = message;
                    msg.message = message;
                } else if (str.startsWith("BODY[")) { // krazy:exclude=strings
                    if (!str.endsWith(']')) { // BODY[ ... ] might have been split, skip until we find the ]
                        while (!(*it).endsWith(']')) {
                            ++it;
                        }
                        ++it;
                    }

                    int index;
                    if ((index = str.indexOf("HEADER")) > 0 || (index = str.indexOf("MIME")) > 0) { // headers
                        if (str[index - 1] == '.') {
                            QByteArray partId = str.mid(5, index - 6);
                            if (!parts.contains(partId)) {
                                parts[partId] = ContentPtr(new KMime::Content);
                            }
                            parts[partId]->setHead(*it);
                            parts[partId]->parse();
                            d->pendingParts[id] = parts;
                            msg.parts = parts;
                        } else {
                            message->setHead(*it);
                            shouldParseMessage = true;
                        }
                    } else { // full payload
                        if (str == "BODY[]") {
                            message->setContent(KMime::CRLFtoLF(*it));
                            shouldParseMessage = true;

                            d->pendingMessages[id] = message;
                            msg.message = message;
                        } else {
                            QByteArray partId = str.mid(5, str.size() - 6);
                            if (!parts.contains(partId)) {
                                parts[partId] = ContentPtr(new KMime::Content);
                            }
                            parts[partId]->setBody(*it);
                            parts[partId]->parse();

                            d->pendingParts[id] = parts;
                            msg.parts = parts;
                        }
                    }
                }
            }

            if (shouldParseMessage) {
                message->parse();
            }

            // For the headers mode the message is built in several
            // steps, hence why we wait it to be done until putting it
            // in the pending queue.
            if (d->scope.mode == FetchScope::Headers || d->scope.mode == FetchScope::HeaderAndContent || d->scope.mode == FetchScope::FullHeaders) {
                d->pendingMessages[id] = message;
                msg.message = message;
            }

            d->pendingMsgs[id] = msg;
        }
    }
}

void FetchJobPrivate::parseBodyStructure(const QByteArray &structure, int &pos, KMime::Content *content)
{
    skipLeadingSpaces(structure, pos);

    if (structure[pos] != '(') {
        return;
    }

    pos++;

    if (structure[pos] != '(') { // simple part
        pos--;
        parsePart(structure, pos, content);
    } else { // multi part
        content->contentType()->setMimeType("MULTIPART/MIXED");
        while (pos < structure.size() && structure[pos] == '(') {
            auto child = new KMime::Content;
            content->addContent(child);
            parseBodyStructure(structure, pos, child);
            child->assemble();
        }

        QByteArray subType = parseString(structure, pos);
        content->contentType()->setMimeType("MULTIPART/" + subType);

        QByteArray parameters = parseSentence(structure, pos); // FIXME: Read the charset
        if (parameters.contains("BOUNDARY")) {
            content->contentType()->setBoundary(parameters.remove(0, parameters.indexOf("BOUNDARY") + 11).split('\"')[0]);
        }

        QByteArray disposition = parseSentence(structure, pos);
        if (disposition.contains("INLINE")) {
            content->contentDisposition()->setDisposition(KMime::Headers::CDinline);
        } else if (disposition.contains("ATTACHMENT")) {
            content->contentDisposition()->setDisposition(KMime::Headers::CDattachment);
        }

        parseSentence(structure, pos); // Ditch the body language
    }

    // Consume what's left
    while (pos < structure.size() && structure[pos] != ')') {
        skipLeadingSpaces(structure, pos);
        parseSentence(structure, pos);
        skipLeadingSpaces(structure, pos);
    }

    pos++;
}

void FetchJobPrivate::parsePart(const QByteArray &structure, int &pos, KMime::Content *content)
{
    if (structure[pos] != '(') {
        return;
    }

    pos++;

    QByteArray mainType = parseString(structure, pos);
    QByteArray subType = parseString(structure, pos);

    content->contentType()->setMimeType(mainType + '/' + subType);

    parseSentence(structure, pos); // Ditch the parameters... FIXME: Read it to get charset and name
    parseString(structure, pos); // ... and the id

    content->contentDescription()->from7BitString(parseString(structure, pos));

    parseString(structure, pos); // Ditch the encoding too
    parseString(structure, pos); // ... and the size
    parseString(structure, pos); // ... and the line count

    QByteArray disposition = parseSentence(structure, pos);
    if (disposition.contains("INLINE")) {
        content->contentDisposition()->setDisposition(KMime::Headers::CDinline);
    } else if (disposition.contains("ATTACHMENT")) {
        content->contentDisposition()->setDisposition(KMime::Headers::CDattachment);
    }
    if ((content->contentDisposition()->disposition() == KMime::Headers::CDattachment
         || content->contentDisposition()->disposition() == KMime::Headers::CDinline)
        && disposition.contains("FILENAME")) {
        QByteArray filename = disposition.remove(0, disposition.indexOf("FILENAME") + 11).split('\"')[0];
        content->contentDisposition()->setFilename(QLatin1String(filename));
    }

    // Consume what's left
    while (pos < structure.size() && structure[pos] != ')') {
        skipLeadingSpaces(structure, pos);
        parseSentence(structure, pos);
        skipLeadingSpaces(structure, pos);
    }
}

QByteArray FetchJobPrivate::parseSentence(const QByteArray &structure, int &pos)
{
    QByteArray result;
    int stack = 0;

    skipLeadingSpaces(structure, pos);

    if (structure[pos] != '(') {
        return parseString(structure, pos);
    }

    int start = pos;

    do {
        switch (structure[pos]) {
        case '(':
            pos++;
            stack++;
            break;
        case ')':
            pos++;
            stack--;
            break;
        case '[':
            pos++;
            stack++;
            break;
        case ']':
            pos++;
            stack--;
            break;
        default:
            skipLeadingSpaces(structure, pos);
            parseString(structure, pos);
            skipLeadingSpaces(structure, pos);
            break;
        }
    } while (pos < structure.size() && stack != 0);

    result = structure.mid(start, pos - start);

    return result;
}

QByteArray FetchJobPrivate::parseString(const QByteArray &structure, int &pos)
{
    QByteArray result;

    skipLeadingSpaces(structure, pos);

    int start = pos;
    bool foundSlash = false;

    // quoted string
    if (structure[pos] == '"') {
        pos++;
        Q_FOREVER {
            if (structure[pos] == '\\') {
                pos += 2;
                foundSlash = true;
                continue;
            }
            if (structure[pos] == '"') {
                result = structure.mid(start + 1, pos - start - 1);
                pos++;
                break;
            }
            pos++;
        }
    } else { // unquoted string
        Q_FOREVER {
            if (structure[pos] == ' ' || structure[pos] == '(' || structure[pos] == ')' || structure[pos] == '[' || structure[pos] == ']'
                || structure[pos] == '\n' || structure[pos] == '\r' || structure[pos] == '"') {
                break;
            }
            if (structure[pos] == '\\') {
                foundSlash = true;
            }
            pos++;
        }

        result = structure.mid(start, pos - start);

        // transform unquoted NIL
        if (result == "NIL") {
            result.clear();
        }
    }

    // simplify slashes
    if (foundSlash) {
        while (result.contains("\\\"")) {
            result.replace("\\\"", "\"");
        }
        while (result.contains("\\\\")) {
            result.replace("\\\\", "\\");
        }
    }

    return result;
}

void FetchJobPrivate::skipLeadingSpaces(const QByteArray &structure, int &pos)
{
    while (pos < structure.size() && structure[pos] == ' ') {
        pos++;
    }
}

#include "moc_fetchjob.cpp"
