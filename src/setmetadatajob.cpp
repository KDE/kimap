/*
    Copyright (c) 2009 Andras Mantia <amantia@kde.org>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

#include "setmetadatajob.h"

#include <KLocalizedString>
#include <QDebug>

#include "metadatajobbase_p.h"
#include "message_p.h"
#include "session_p.h"
#include "rfccodecs.h"

namespace KIMAP
{
class SetMetaDataJobPrivate : public MetaDataJobBasePrivate
{
public:
    SetMetaDataJobPrivate(Session *session, const QString &name) : MetaDataJobBasePrivate(session, name), metaDataErrors(0), maxAcceptedSize(-1) { }
    ~SetMetaDataJobPrivate() { }

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
    QByteArray parameters;
    parameters = '\"' + KIMAP::encodeImapFolderName(d->mailBox.toUtf8()) + "\" ";
    d->entriesIt = d->entries.constBegin();

    QByteArray command = "SETMETADATA";
    if (d->serverCapability == Annotatemore) {
        command = "SETANNOTATION";
        parameters += '\"' + d->entryName + "\" (";
        d->m_name = i18n("SetAnnotation");
        if (!d->entries.isEmpty()) {
            for (; d->entriesIt != d->entries.constEnd(); ++d->entriesIt) {
                parameters += '\"' + d->entriesIt.key() + "\" \"" + d->entriesIt.value() + "\" ";
            }
            parameters[parameters.length() - 1] = ')';
        }
    } else {
        parameters += '(';
        if (!d->entries.isEmpty()) {
            parameters += '\"' + d->entriesIt.key() + '\"';
            parameters += ' ';
            parameters += " {" + QByteArray::number(d->entriesIt.value().size()) + '}';
        }
    }

    if (d->entries.isEmpty()) {
        parameters += ')';
    }

    d->tags << d->sessionInternal()->sendCommand(command, parameters);
//   qDebug() << "SENT: " << command << " " << parameters;
}

void SetMetaDataJob::handleResponse(const Message &response)
{
    Q_D(SetMetaDataJob);

    //TODO: Test if a server can really return more then one untagged NO response. If not, no need to OR the error codes
    if (!response.content.isEmpty() &&
            d->tags.contains(response.content.first().toString())) {
        if (response.content[1].toString() == "NO") {
            setError(UserDefinedError);
            setErrorText(i18n("%1 failed, server replied: %2", d->m_name, QLatin1String(response.toString().constData())));
            if (response.content[2].toString() == "[ANNOTATEMORE TOOMANY]" ||
                    response.content[2].toString() == "[METADATA TOOMANY]") {
                d->metaDataErrors |= TooMany;
            } else if (response.content[2].toString() == "[ANNOTATEMORE TOOBIG]" ||
                       response.content[2].toString().startsWith("[METADATA MAXSIZE")) {    //krazy:exclude=strings
                d->metaDataErrors |= TooBig;
                d->maxAcceptedSize = -1;
                if (response.content[2].toString().startsWith("[METADATA MAXSIZE")) {     //krazy:exclude=strings
                    QByteArray max = response.content[2].toString();
                    max.replace("[METADATA MAXSIZE", "");   //krazy:exclude=doublequote_chars
                    max.replace("]", "");                   //krazy:exclude=doublequote_chars
                    d->maxAcceptedSize = max.toLongLong();
                }
            } else if (response.content[2].toString() == "[METADATA NOPRIVATE]") {
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
        QByteArray content = d->entriesIt.value();
        ++d->entriesIt;
        if (d->entriesIt == d->entries.constEnd()) {
            content += ')';
        } else {
            content += " {" + QByteArray::number(d->entriesIt.value().size()) + '}';
        }
//      qDebug() << "SENT: " << content;
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
