/*
    SPDX-FileCopyrightText: 2009 Andras Mantia <amantia@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "getmetadatajob.h"

#include "kimap_debug.h"
#include <KLocalizedString>

#include "metadatajobbase_p.h"
#include "response_p.h"
#include "rfccodecs.h"
#include "session_p.h"

namespace KIMAP
{
class GetMetaDataJobPrivate : public MetaDataJobBasePrivate
{
public:
    GetMetaDataJobPrivate(Session *session, const QString &name)
        : MetaDataJobBasePrivate(session, name)
        , depth("0")
    {
    }
    ~GetMetaDataJobPrivate()
    {
    }

    qint64 maxSize = -1;
    QByteArray depth;
    QSet<QByteArray> entries;
    QSet<QByteArray> attributes;
    QMap<QString, QMap<QByteArray, QMap<QByteArray, QByteArray>>> metadata;
    //    ^ mailbox        ^ entry          ^attribute  ^ value
};
}

using namespace KIMAP;

GetMetaDataJob::GetMetaDataJob(Session *session)
    : MetaDataJobBase(*new GetMetaDataJobPrivate(session, i18n("GetMetaData")))
{
}

GetMetaDataJob::~GetMetaDataJob()
{
}

static QList<QByteArray> sort(const QSet<QByteArray> &set)
{
    QList<QByteArray> sortedEntries = set.values();
    std::sort(sortedEntries.begin(), sortedEntries.end());
    return sortedEntries;
}

void GetMetaDataJob::doStart()
{
    Q_D(GetMetaDataJob);
    QByteArray parameters;
    parameters = '\"' + KIMAP::encodeImapFolderName(d->mailBox.toUtf8()) + "\" ";

    QByteArray command = "GETMETADATA";
    if (d->serverCapability == Annotatemore) {
        d->m_name = i18n("GetAnnotation");
        command = "GETANNOTATION";
        if (d->entries.size() > 1) {
            parameters += '(';
        }
        const auto sortedEntries = sort(d->entries);
        for (const QByteArray &entry : sortedEntries) {
            parameters += '\"' + entry + "\" ";
        }
        if (d->entries.size() > 1) {
            parameters[parameters.length() - 1] = ')';
            parameters += ' ';
        }

        if (d->attributes.size() > 1) {
            parameters += '(';
        }
        const auto sortedAttributes = sort(d->attributes);
        for (const QByteArray &attribute : sortedAttributes) {
            parameters += '\"' + attribute + "\" ";
        }
        if (d->attributes.size() > 1) {
            parameters[parameters.length() - 1] = ')';
        } else {
            parameters.chop(1);
        }

    } else {
        QByteArray options;
        if (d->depth != "0") {
            options = "DEPTH " + d->depth;
        }
        if (d->maxSize != -1) {
            if (!options.isEmpty()) {
                options += ' ';
            }
            options += "MAXSIZE " + QByteArray::number(d->maxSize);
        }

        if (!options.isEmpty()) {
            parameters = "(" + options + ") " + parameters;
        }

        if (d->entries.size() >= 1) {
            parameters += '(';
            const auto sortedEntries = sort(d->entries);
            for (const QByteArray &entry : sortedEntries) {
                parameters += entry + " ";
            }
            parameters[parameters.length() - 1] = ')';
        } else {
            parameters.chop(1);
        }
    }

    d->tags << d->sessionInternal()->sendCommand(command, parameters);
    //  qCDebug(KIMAP_LOG) << "SENT: " << command << " " << parameters;
}

void GetMetaDataJob::handleResponse(const Response &response)
{
    Q_D(GetMetaDataJob);
    //  qCDebug(KIMAP_LOG) << "GOT: " << response.toString();

    // TODO: handle NO error messages having [METADATA MAXSIZE NNN], [METADATA TOOMANY], [METADATA NOPRIVATE] (see rfc5464)
    // or [ANNOTATEMORE TOOBIG], [ANNOTATEMORE TOOMANY] respectively
    if (handleErrorReplies(response) == NotHandled) {
        if (response.content.size() >= 4) {
            if (d->serverCapability == Annotatemore && response.content[1].toString() == "ANNOTATION") {
                QString mailBox = QString::fromUtf8(KIMAP::decodeImapFolderName(response.content[2].toString()));

                int i = 3;
                while (i < response.content.size() - 1) {
                    QByteArray entry = response.content[i].toString();
                    QList<QByteArray> attributes = response.content[i + 1].toList();
                    int j = 0;
                    while (j < attributes.size() - 1) {
                        d->metadata[mailBox][entry][attributes[j]] = attributes[j + 1];
                        j += 2;
                    }
                    i += 2;
                }
            } else if (d->serverCapability == Metadata && response.content[1].toString() == "METADATA") {
                QString mailBox = QString::fromUtf8(KIMAP::decodeImapFolderName(response.content[2].toString()));

                const QList<QByteArray> &entries = response.content[3].toList();
                int i = 0;
                while (i < entries.size() - 1) {
                    const QByteArray &value = entries[i + 1];
                    QByteArray &targetValue = d->metadata[mailBox][entries[i]][""];
                    if (value != "NIL") { // This just indicates no value
                        targetValue = value;
                    }
                    i += 2;
                }
            }
        }
    }
}

void GetMetaDataJob::addEntry(const QByteArray &entry, const QByteArray &attribute)
{
    Q_D(GetMetaDataJob);
    if (d->serverCapability == Annotatemore && attribute.isNull()) {
        qCWarning(KIMAP_LOG) << "In ANNOTATEMORE mode an attribute must be specified with addEntry!";
    }
    d->entries.insert(entry);
    d->attributes.insert(attribute);
}

void GetMetaDataJob::addRequestedEntry(const QByteArray &entry)
{
    Q_D(GetMetaDataJob);
    d->entries.insert(d->removePrefix(entry));
    d->attributes.insert(d->getAttribute(entry));
}

void GetMetaDataJob::setMaximumSize(qint64 size)
{
    Q_D(GetMetaDataJob);
    d->maxSize = size;
}

void GetMetaDataJob::setDepth(Depth depth)
{
    Q_D(GetMetaDataJob);

    switch (depth) {
    case OneLevel:
        d->depth = "1"; // krazy:exclude=doublequote_chars
        break;
    case AllLevels:
        d->depth = "infinity";
        break;
    default:
        d->depth = "0"; // krazy:exclude=doublequote_chars
    }
}

QByteArray GetMetaDataJob::metaData(const QString &mailBox, const QByteArray &entry, const QByteArray &attribute) const
{
    Q_D(const GetMetaDataJob);
    QByteArray attr = attribute;

    if (d->serverCapability == Metadata) {
        attr = "";
    }

    QByteArray result;
    if (d->metadata.contains(mailBox)) {
        if (d->metadata[mailBox].contains(entry)) {
            result = d->metadata[mailBox][entry].value(attr);
        }
    }
    return result;
}

QByteArray GetMetaDataJob::metaData(const QByteArray &entry) const
{
    qCDebug(KIMAP_LOG) << entry;
    Q_D(const GetMetaDataJob);
    return d->metadata.value(d->mailBox).value(d->removePrefix(entry)).value(d->getAttribute(entry));
}

QMap<QByteArray, QMap<QByteArray, QByteArray>> GetMetaDataJob::allMetaData(const QString &mailBox) const
{
    Q_D(const GetMetaDataJob);
    return d->metadata[mailBox];
}

QMap<QByteArray, QByteArray> GetMetaDataJob::allMetaData() const
{
    Q_D(const GetMetaDataJob);
    return allMetaDataForMailbox(d->mailBox);
}

QMap<QByteArray, QByteArray> GetMetaDataJob::allMetaDataForMailbox(const QString &mailbox) const
{
    Q_D(const GetMetaDataJob);
    const QMap<QByteArray, QMap<QByteArray, QByteArray>> &entries = d->metadata[mailbox];
    QMap<QByteArray, QByteArray> map;
    const auto entriesKeys = entries.keys();
    for (const QByteArray &entry : entriesKeys) {
        const QMap<QByteArray, QByteArray> &values = entries[entry];
        const auto valuesKeys = values.keys();
        for (const QByteArray &attribute : valuesKeys) {
            map.insert(d->addPrefix(entry, attribute), values[attribute]);
        }
    }
    return map;
}

QHash<QString, QMap<QByteArray, QByteArray>> GetMetaDataJob::allMetaDataForMailboxes() const
{
    Q_D(const GetMetaDataJob);
    QHash<QString, QMap<QByteArray, QByteArray>> mailboxHash;

    QMapIterator<QString, QMap<QByteArray, QMap<QByteArray, QByteArray>>> i(d->metadata);
    while (i.hasNext()) {
        i.next();
        mailboxHash.insert(i.key(), allMetaDataForMailbox(i.key()));
    }
    return mailboxHash;
}
