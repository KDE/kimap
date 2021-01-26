/*
    SPDX-FileCopyrightText: 2009 Andras Mantia <amantia@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "setmetadatajob.h"

#include "kimap_debug.h"
#include <KLocalizedString>

#include "metadatajobbase_p.h"
#include "response_p.h"
#include "rfccodecs.h"
#include "session_p.h"

namespace KIMAP
{
class SetMetaDataJobPrivate : public MetaDataJobBasePrivate
{
public:
    SetMetaDataJobPrivate(Session *session, const QString &name)
        : MetaDataJobBasePrivate(session, name)
        , metaDataErrors({})
        , maxAcceptedSize(-1)
    {
    }
    ~SetMetaDataJobPrivate()
    {
    }

    QMap<QByteArray, QByteArray> entries;
    QMap<QByteArray, QByteArray>::ConstIterator entriesIt;
    QByteArray entryName;
    SetMetaDataJob::MetaDataErrors metaDataErrors;
    qint64 maxAcceptedSize;
};
}

using namespace KIMAP;

SetMetaDataJob::SetMetaDataJob(Session *session)
    : MetaDataJobBase(*new SetMetaDataJobPrivate(session, i18n("SetMetaData")))
{
}

SetMetaDataJob::~SetMetaDataJob()
{
}

void SetMetaDataJob::doStart()
{
    Q_D(SetMetaDataJob);
    QByteArray parameters = '\"' + KIMAP::encodeImapFolderName(d->mailBox.toUtf8()) + "\" ";
    d->entriesIt = d->entries.constBegin();

    QByteArray command = "SETMETADATA";
    bool bSimpleData = true;

    if (d->serverCapability == Annotatemore) {
        command = "SETANNOTATION";
        parameters += '\"' + d->entryName + "\" ";
    } else {
        for (; d->entriesIt != d->entries.constEnd(); ++d->entriesIt) {
            if (d->entriesIt.value().contains('\r') || d->entriesIt.value().contains('\n')) {
                bSimpleData = false;
                break;
            }
        }
        d->entriesIt = d->entries.constBegin();
    }

    parameters += '(';
    if (bSimpleData == true) {
        for (; d->entriesIt != d->entries.constEnd(); ++d->entriesIt) {
            parameters += '\"' + d->entriesIt.key() + "\" ";
            if (d->entriesIt.value().isEmpty()) {
                parameters += "NIL";
            } else {
                parameters += "\"" + d->entriesIt.value() + "\"";
            }
            parameters += " ";
        }
        parameters[parameters.length() - 1] = ')';
    } else {
        if (!d->entries.isEmpty()) {
            parameters += '\"' + d->entriesIt.key() + "\"";
            int size = d->entriesIt.value().size();
            parameters += " {" + QByteArray::number(size == 0 ? 3 : size) + '}';
        }
    }

    if (d->entries.isEmpty()) {
        parameters += ')';
    }

    d->tags << d->sessionInternal()->sendCommand(command, parameters);
    //   qCDebug(KIMAP_LOG) << "SENT: " << command << " " << parameters;
}

void SetMetaDataJob::handleResponse(const Response &response)
{
    Q_D(SetMetaDataJob);

    // TODO: Test if a server can really return more then one untagged NO response. If not, no need to OR the error codes
    if (!response.content.isEmpty() && d->tags.contains(response.content.first().toString())) {
        if (response.content[1].toString() == "NO") {
            setError(UserDefinedError);
            setErrorText(i18n("%1 failed, server replied: %2", d->m_name, QLatin1String(response.toString().constData())));
            const QByteArray responseBa = response.content[2].toString();
            if (responseBa == "[ANNOTATEMORE TOOMANY]" || responseBa == "[METADATA TOOMANY]") {
                d->metaDataErrors |= TooMany;
            } else if (responseBa == "[ANNOTATEMORE TOOBIG]" || responseBa.startsWith("[METADATA MAXSIZE")) { // krazy:exclude=strings
                d->metaDataErrors |= TooBig;
                d->maxAcceptedSize = -1;
                if (responseBa.startsWith("[METADATA MAXSIZE")) { // krazy:exclude=strings
                    QByteArray max = responseBa;
                    max.replace("[METADATA MAXSIZE", ""); // krazy:exclude=doublequote_chars
                    max.replace("]", ""); // krazy:exclude=doublequote_chars
                    d->maxAcceptedSize = max.toLongLong();
                }
            } else if (responseBa == "[METADATA NOPRIVATE]") {
                d->metaDataErrors |= NoPrivate;
            }
        } else if (response.content.size() < 2) {
            setErrorText(i18n("%1 failed, malformed reply from the server.", d->m_name));
        } else if (response.content[1].toString() != "OK") {
            setError(UserDefinedError);
            setErrorText(i18n("%1 failed, server replied: %2", d->m_name, QLatin1String(response.toString().constData())));
        }
        emitResult();
    } else if (d->serverCapability == Metadata && response.content[0].toString() == "+") {
        QByteArray content = "";
        if (d->entriesIt.value().isEmpty()) {
            content += "NIL";
        } else {
            content += d->entriesIt.value();
        }
        ++d->entriesIt;
        if (d->entriesIt == d->entries.constEnd()) {
            content += ')';
        } else {
            content += " \"" + d->entriesIt.key() + '\"';
            int size = d->entriesIt.value().size();
            content += " {" + QByteArray::number(size == 0 ? 3 : size) + '}';
        }
        //      qCDebug(KIMAP_LOG) << "SENT: " << content;
        d->sessionInternal()->sendData(content);
    }
}

void SetMetaDataJob::addMetaData(const QByteArray &name, const QByteArray &value)
{
    Q_D(SetMetaDataJob);
    if (d->serverCapability == Annotatemore && (name.startsWith("/shared") || name.startsWith("/private"))) {
        const QByteArray &attribute = d->getAttribute(name);
        d->entries[attribute] = value;
        d->entryName = d->removePrefix(name);
    } else {
        d->entries[name] = value;
    }
}

void SetMetaDataJob::setEntry(const QByteArray &entry)
{
    Q_D(SetMetaDataJob);
    d->entryName = entry;
}

SetMetaDataJob::MetaDataErrors SetMetaDataJob::metaDataErrors() const
{
    Q_D(const SetMetaDataJob);
    return d->metaDataErrors;
}
