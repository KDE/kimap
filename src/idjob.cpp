/*
    SPDX-FileCopyrightText: 2015 Christian Mollekopf <mollekopf@kolabsys.com>

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
        IdJobPrivate(Session *session,  const QString& name) : JobPrivate(session, name) { }
        ~IdJobPrivate() { }
  
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
    //Q_D(IdJob);
    if (handleErrorReplies(response) == NotHandled) {
        // Ignore the response
    }
}
