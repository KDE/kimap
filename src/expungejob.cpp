/*
    SPDX-FileCopyrightText: 2009 Andras Mantia <amantia@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "expungejob.h"

#include <KLocalizedString>
#include "kimap_debug.h"

#include "job_p.h"
#include "response_p.h"
#include "session_p.h"

namespace KIMAP
{
class ExpungeJobPrivate : public JobPrivate
{
public:
    ExpungeJobPrivate(Session *session, const QString &name) : JobPrivate(session, name) { }
    ~ExpungeJobPrivate() { }
#if 0
    QList< int > items;
#endif
};
}

using namespace KIMAP;

ExpungeJob::ExpungeJob(Session *session)
    : Job(*new ExpungeJobPrivate(session, i18n("Expunge")))
{
}

ExpungeJob::~ExpungeJob()
{
}

void ExpungeJob::doStart()
{
    Q_D(ExpungeJob);
    d->tags << d->sessionInternal()->sendCommand("EXPUNGE");
}

void ExpungeJob::handleResponse(const Response &response)
{
//  Q_D(ExpungeJob);

    if (handleErrorReplies(response) == NotHandled) {
        if (response.content.size() >= 3) {
            QByteArray code = response.content[2].toString();
            if (code == "EXPUNGE") {
#if 0
                QByteArray s = response.content[1].toString();
                bool ok = true;
                int id = s.toInt(&ok);
                if (ok) {
                    d->items.append(id);
                }
                //TODO error handling
#endif
                return;
            }
        }
        qCDebug(KIMAP_LOG) << "Unhandled response: " << response.toString().constData();

    }
}
