/*
    Copyright (c) 2009 Kevin Ottens <ervin@kde.org>

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

#include "capabilitiesjob.h"

#include <KLocalizedString>

#include "job_p.h"
#include "message_p.h"
#include "session_p.h"

namespace KIMAP
{
class CapabilitiesJobPrivate : public JobPrivate
{
public:
    CapabilitiesJobPrivate(Session *session,  const QString &name) : JobPrivate(session, name) { }
    ~CapabilitiesJobPrivate() { }

    QStringList capabilities;
};
}

using namespace KIMAP;

CapabilitiesJob::CapabilitiesJob(Session *session)
    : Job(*new CapabilitiesJobPrivate(session, i18n("Capabilities")))
{
}

CapabilitiesJob::~CapabilitiesJob()
{
}

QStringList CapabilitiesJob::capabilities() const
{
    Q_D(const CapabilitiesJob);
    return d->capabilities;
}

void CapabilitiesJob::doStart()
{
    Q_D(CapabilitiesJob);
    d->tags << d->sessionInternal()->sendCommand("CAPABILITY");
}

void CapabilitiesJob::handleResponse(const Message &response)
{

    Q_D(CapabilitiesJob);
    if (handleErrorReplies(response) == NotHandled) {
        if (response.content.size() >= 2 &&
                response.content[1].toString() == "CAPABILITY") {
            for (int i = 2; i < response.content.size(); ++i) {
                d->capabilities << QLatin1String(response.content[i].toString().toUpper());
            }
            emit capabilitiesReceived(d->capabilities);
        }
    }
}
