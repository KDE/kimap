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

#include "myrightsjob.h"

#include <KLocalizedString>

#include "acljobbase_p.h"
#include "message_p.h"
#include "session_p.h"
#include "rfccodecs.h"

namespace KIMAP
{
class MyRightsJobPrivate : public AclJobBasePrivate
{
public:
    MyRightsJobPrivate(Session *session, const QString &name) : AclJobBasePrivate(session, name), myRights(Acl::None) {}
    ~MyRightsJobPrivate() { }

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

void MyRightsJob::handleResponse(const Message &response)
{
    Q_D(MyRightsJob);

    if (handleErrorReplies(response) == NotHandled) {
        if (response.content.size() == 4 &&
                response.content[1].toString() == "MYRIGHTS") {
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
