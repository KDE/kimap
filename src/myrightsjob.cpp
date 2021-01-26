/*
    SPDX-FileCopyrightText: 2009 Andras Mantia <amantia@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "myrightsjob.h"

#include <KLocalizedString>

#include "acljobbase_p.h"
#include "response_p.h"
#include "rfccodecs.h"
#include "session_p.h"

namespace KIMAP
{
class MyRightsJobPrivate : public AclJobBasePrivate
{
public:
    MyRightsJobPrivate(Session *session, const QString &name)
        : AclJobBasePrivate(session, name)
        , myRights(Acl::None)
    {
    }
    ~MyRightsJobPrivate()
    {
    }

    Acl::Rights myRights;
};
}

using namespace KIMAP;

MyRightsJob::MyRightsJob(Session *session)
    : AclJobBase(*new MyRightsJobPrivate(session, i18n("MyRights")))
{
}

MyRightsJob::~MyRightsJob()
{
}

void MyRightsJob::doStart()
{
    Q_D(MyRightsJob);

    d->tags << d->sessionInternal()->sendCommand("MYRIGHTS", '\"' + KIMAP::encodeImapFolderName(d->mailBox.toUtf8()) + '\"');
}

void MyRightsJob::handleResponse(const Response &response)
{
    Q_D(MyRightsJob);

    if (handleErrorReplies(response) == NotHandled) {
        if (response.content.size() == 4 && response.content[1].toString() == "MYRIGHTS") {
            d->myRights = Acl::rightsFromString(response.content[3].toString());
        }
    }
}

bool MyRightsJob::hasRightEnabled(Acl::Right right)
{
    Q_D(MyRightsJob);
    return d->myRights & right;
}

Acl::Rights MyRightsJob::rights()
{
    Q_D(MyRightsJob);
    return d->myRights;
}
