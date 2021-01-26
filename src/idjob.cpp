/*
    SPDX-FileCopyrightText: 2015 Christian Mollekopf <mollekopf@kolabsys.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "idjob.h"

#include <KLocalizedString>

#include "job_p.h"
#include "response_p.h"
#include "session_p.h"

namespace KIMAP
{
class IdJobPrivate : public JobPrivate
{
public:
    IdJobPrivate(Session *session, const QString &name)
        : JobPrivate(session, name)
    {
    }
    ~IdJobPrivate()
    {
    }

    QMap<QByteArray, QByteArray> fields;
};
}

using namespace KIMAP;

IdJob::IdJob(Session *session)
    : Job(*new IdJobPrivate(session, i18n("Id")))
{
}

IdJob::~IdJob()
{
}

void IdJob::setField(const QByteArray &name, const QByteArray &value)
{
    Q_D(IdJob);
    d->fields.insert(name, value);
}

void IdJob::doStart()
{
    Q_D(IdJob);
    QByteArray command = "ID";
    command += " (";

    QMapIterator<QByteArray, QByteArray> i(d->fields);
    while (i.hasNext()) {
        i.next();
        command += "\"" + i.key() + "\" \"" + i.value() + "\" ";
    }
    command.chop(1);
    command += ")";
    d->tags << d->sessionInternal()->sendCommand(command);
}

void IdJob::handleResponse(const Response &response)
{
    // Q_D(IdJob);
    if (handleErrorReplies(response) == NotHandled) {
        // Ignore the response
    }
}
