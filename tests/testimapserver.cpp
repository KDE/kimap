/**
  * This file is part of the KDE project
  * Copyright (C) 2009 Kevin Ottens <ervin@kde.org>
  * Copyright (C) 2009 Andras Mantia <amantia@kde.org>
  *
  * This library is free software; you can redistribute it and/or
  * modify it under the terms of the GNU Library General Public
  * License as published by the Free Software Foundation; either
  * version 2 of the License, or (at your option) any later version.
  *
  * This library is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  * Library General Public License for more details.
  *
  * You should have received a copy of the GNU Library General Public License
  * along with this library; see the file COPYING.LIB.  If not, write to
  * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  * Boston, MA 02110-1301, USA.
  */

#include <kcomponentdata.h>
#include <k4aboutdata.h>
#include <qdebug.h>
#include <klocalizedstring.h>
#include <qtcpsocket.h>
#include <QCoreApplication>
#include <qsignalspy.h>

#include "acl.h"
#include "session.h"
#include "appendjob.h"
#include "capabilitiesjob.h"
#include "fetchjob.h"
#include "listjob.h"
#include "loginjob.h"
#include "logoutjob.h"
#include "selectjob.h"
#include "closejob.h"
#include "expungejob.h"
#include "createjob.h"
#include "deletejob.h"
#include "namespacejob.h"
#include "subscribejob.h"
#include "unsubscribejob.h"
#include "renamejob.h"
#include "storejob.h"
#include "sessionuiproxy.h"
#include "setacljob.h"
#include "getacljob.h"
#include "deleteacljob.h"
#include "myrightsjob.h"
#include "listrightsjob.h"
#include "setmetadatajob.h"
#include "getmetadatajob.h"

using namespace KIMAP;

class UiProxy: public SessionUiProxy
{
public:
    bool ignoreSslError(const KSslErrorUiData &errorData)
    {
        return true;
    }
};

void dumpContentHelper(KMime::Content *part, const QString &partId = QString())
{
    if (partId.isEmpty()) {
        qDebug() << "** Message root **";
    } else {
        qDebug() << "** Part" << partId << "**";
    }

    qDebug() << part->head();

    KMime::Content::List children = part->contents();
    for (int i = 0; i < children.size(); i++) {
        QString newId = partId;
        if (!newId.isEmpty()) {
            newId += QLatin1String(".");
        }
        newId += QString::number(i + 1);
        dumpContentHelper(children[i], newId);
    }
}

void listFolders(Session *session, bool includeUnsubscribed = false, const QString &nameFilter = QLatin1String(""))
{
    ListJob *list = new ListJob(session);
    list->setIncludeUnsubscribed(includeUnsubscribed);
    list->exec();
    Q_ASSERT_X(list->error() == 0, "ListJob", list->errorString().toLocal8Bit().constData());
    int count = list->mailBoxes().size();
    for (int i = 0; i < count; ++i) {
        MailBoxDescriptor descriptor = list->mailBoxes()[i];
        if (descriptor.name.endsWith(nameFilter)) {
            qDebug() << descriptor.separator << descriptor.name;
        }
    }

}

void testMetaData(Session *session)
{
    qDebug() << "TESTING: METADATA commands";
    CreateJob *create = new CreateJob(session);
    create->setMailBox(QLatin1String("INBOX/TestFolder"));
    create->exec();

    SetMetaDataJob *setmetadata = new SetMetaDataJob(session);
    setmetadata->setMailBox(QLatin1String("INBOX/TestFolder"));
    setmetadata->setServerCapability(SetMetaDataJob::Annotatemore);
    setmetadata->setEntry("/comment");
    setmetadata->addMetaData("value.priv", "My new comment");
    setmetadata->exec();

    setmetadata = new SetMetaDataJob(session);
    setmetadata->setMailBox(QLatin1String("INBOX/TestFolder"));
    setmetadata->setServerCapability(SetMetaDataJob::Annotatemore);
    setmetadata->setEntry("/check");
    setmetadata->addMetaData("value.priv", "true");
    setmetadata->exec();

    GetMetaDataJob *getmetadata = new GetMetaDataJob(session);
    getmetadata->setMailBox(QLatin1String("INBOX/TestFolder"));
    getmetadata->setServerCapability(SetMetaDataJob::Annotatemore);
    getmetadata->addEntry("/*", "value.priv");
    getmetadata->exec();
    Q_ASSERT_X(getmetadata->metaData(QLatin1String("INBOX/TestFolder"), "/check", "value.priv") == "true", "",  "/check metadata should be true");
    Q_ASSERT_X(getmetadata->metaData(QLatin1String("INBOX/TestFolder"), "/comment", "value.priv") == "My new comment", "",  "/check metadata should be My new comment");

    //cleanup
    DeleteJob *deletejob = new DeleteJob(session);
    deletejob->setMailBox(QLatin1String("INBOX/TestFolder"));
    deletejob->exec();
}

void testAcl(Session *session, const QString &user)
{
    qDebug() << "TESTING: ACL commands";
    CreateJob *create = new CreateJob(session);
    create->setMailBox(QLatin1String("INBOX/TestFolder"));
    create->exec();

    ListRightsJob *listRights = new ListRightsJob(session);
    listRights->setMailBox(QLatin1String("INBOX/TestFolder"));
    listRights->setIdentifier(user.toLatin1());
    listRights->exec();
    qDebug() << "Default rights on INBOX/TestFolder: " << Acl::rightsToString(listRights->defaultRights());
    QList<Acl::Rights> possible = listRights->possibleRights();
    QStringList strList;
    Q_FOREACH (Acl::Rights r, possible) {
        strList << QString::fromLatin1(Acl::rightsToString(r));
    }
    qDebug() << "Possible rights on INBOX/TestFolder: " << strList;

    MyRightsJob *myRights = new MyRightsJob(session);
    myRights->setMailBox(QLatin1String("INBOX/TestFolder"));
    myRights->exec();

    Acl::Rights mine = myRights->rights();
    qDebug() << "My rights on INBOX/TestFolder: " << Acl::rightsToString(mine);
    qDebug() << "Reading INBOX/TestFolder is possible: " << myRights->hasRightEnabled(Acl::Read);
    Q_ASSERT_X(myRights->hasRightEnabled(Acl::Read), "Reading INBOX is NOT possible", "");

    GetAclJob *getAcl = new GetAclJob(session);
    getAcl->setMailBox(QLatin1String("INBOX/TestFolder"));
    getAcl->exec();
    qDebug() << "Anyone rights on INBOX/TestFolder: " << getAcl->rights("anyone");
    Acl::Rights users = getAcl->rights(user.toLatin1());
    qDebug() << user << " rights on INBOX/TestFolder: " << Acl::rightsToString(users);
    Q_ASSERT_X(mine == users, "GETACL returns different rights for the same user", "");

    qDebug() << "Removing Delete right ";
    mine = Acl::Delete;
    SetAclJob *setAcl = new SetAclJob(session);
    setAcl->setMailBox(QLatin1String("INBOX/TestFolder"));
    setAcl->setIdentifier(user.toLatin1());
    setAcl->setRights(AclJobBase::Remove, mine);
    setAcl->exec();

    getAcl = new GetAclJob(session);
    getAcl->setMailBox(QLatin1String("INBOX/TestFolder"));
    getAcl->exec();
    users = getAcl->rights(user.toLatin1());
    qDebug() << user << " rights on INBOX/TestFolder: " << Acl::rightsToString(users);

    qDebug() << "Adding back Delete right ";
    mine = Acl::Delete;
    setAcl = new SetAclJob(session);
    setAcl->setMailBox(QLatin1String("INBOX/TestFolder"));
    setAcl->setIdentifier(user.toLatin1());
    setAcl->setRights(AclJobBase::Add, mine);
    setAcl->exec();

    getAcl = new GetAclJob(session);
    getAcl->setMailBox(QLatin1String("INBOX/TestFolder"));
    getAcl->exec();
    users = getAcl->rights(user.toLatin1());
    qDebug() << user << " rights on INBOX/TestFolder: " << Acl::rightsToString(users);

    //cleanup
    DeleteJob *deletejob = new DeleteJob(session);
    deletejob->setMailBox(QLatin1String("INBOX/TestFolder"));
    deletejob->exec();
}

void testAppendAndStore(Session *session)
{
    qDebug() << "TESTING: APPEND and STORE";
    //setup
    CreateJob *create = new CreateJob(session);
    create->setMailBox(QLatin1String("INBOX/TestFolder"));
    create->exec();

    QByteArray testMailContent =
        "Date: Mon, 7 Feb 1994 21:52:25 -0800 (PST)\r\n"
        "From: Fred Foobar <foobar@Blurdybloop.COM>\r\n"
        "Subject: afternoon meeting\r\n"
        "To: mooch@owatagu.siam.edu\r\n"
        "Message-Id: <B27397-0100000@Blurdybloop.COM>\r\n"
        "MIME-Version: 1.0\r\n"
        "Content-Type: TEXT/PLAIN; CHARSET=US-ASCII\r\n"
        "\r\n"
        "Hello Joe, do you think we can meet at 3:30 tomorrow?\r\n";

    qDebug() << "Append a message in INBOX/TestFolder...";
    AppendJob *append = new AppendJob(session);
    append->setMailBox(QLatin1String("INBOX/TestFolder"));
    append->setContent(testMailContent);
    append->exec();
    Q_ASSERT_X(append->error() == 0, "AppendJob", append->errorString().toLocal8Bit().constData());

    qDebug() << "Read the message back and compare...";
    SelectJob *select = new SelectJob(session);
    select->setMailBox(QLatin1String("INBOX/TestFolder"));
    select->exec();

    FetchJob *fetch = new FetchJob(session);
    FetchJob::FetchScope scope;
    fetch->setSequenceSet(ImapSet(1));
    scope.parts.clear();
    scope.mode = FetchJob::FetchScope::Content;
    fetch->setScope(scope);
    fetch->exec();
    MessagePtr message = fetch->messages()[1];
    Q_ASSERT_X(fetch->error() == 0, "FetchJob", fetch->errorString().toLocal8Bit().constData());
    testMailContent.replace("\r\n", "\n");
    Q_ASSERT_X(testMailContent == message->head() + "\n" + message->body(),
               "Message differs from reference", QByteArray(message->head() + "\n" + message->body()).constData());

    fetch = new FetchJob(session);
    fetch->setSequenceSet(ImapSet(1));
    scope.parts.clear();
    scope.mode = FetchJob::FetchScope::Flags;
    fetch->setScope(scope);
    fetch->exec();
    MessageFlags expectedFlags = fetch->flags()[1];
    qDebug() << "Read the message flags:" << expectedFlags;

    qDebug() << "Add the \\Deleted flag...";
    expectedFlags << "\\Deleted";
    qSort(expectedFlags);
    StoreJob *store = new StoreJob(session);
    store->setSequenceSet(ImapSet(1));
    store->setMode(StoreJob::AppendFlags);
    store->setFlags(QList<QByteArray>() << "\\Deleted");
    store->exec();
    Q_ASSERT_X(store->error() == 0, "StoreJob", store->errorString().toLocal8Bit().constData());

    QList<QByteArray> resultingFlags = store->resultingFlags()[1];
    qSort(resultingFlags);
    if (expectedFlags != resultingFlags) {
        qDebug() << resultingFlags;
    }
    Q_ASSERT(expectedFlags == resultingFlags);

    select = new SelectJob(session);
    select->setMailBox(QLatin1String("INBOX"));
    select->exec();

    //cleanup
    DeleteJob *deletejob = new DeleteJob(session);
    deletejob->setMailBox(QLatin1String("INBOX/TestFolder"));
    deletejob->exec();
    deletejob = new DeleteJob(session);
    deletejob->setMailBox(QLatin1String("INBOX/RenamedTestFolder"));
    deletejob->exec();
}

void testRename(Session *session)
{
    qDebug() << "TESTING: RENAME";
    //setup
    CreateJob *create = new CreateJob(session);
    create->setMailBox(QLatin1String("INBOX/TestFolder"));
    create->exec();

    qDebug() << "Listing mailboxes with name TestFolder:";
    listFolders(session, true, QLatin1String("TestFolder"));

    //actual tests
    qDebug() << "Renaming to RenamedTestFolder";
    RenameJob *rename = new RenameJob(session);
    rename->setSourceMailBox(QLatin1String("INBOX/TestFolder"));
    rename->setDestinationMailBox(QLatin1String("INBOX/RenamedTestFolder"));
    rename->exec();

    qDebug() << "Listing mailboxes with name TestFolder:";
    listFolders(session, true, QLatin1String("TestFolder"));
    qDebug() << "Listing mailboxes with name RenamedTestFolder:";
    listFolders(session, true, QLatin1String("RenamedTestFolder"));

    //cleanup
    DeleteJob *deletejob = new DeleteJob(session);
    deletejob->setMailBox(QLatin1String("INBOX/TestFolder"));
    deletejob->exec();
    deletejob = new DeleteJob(session);
    deletejob->setMailBox(QLatin1String("INBOX/RenamedTestFolder"));
    deletejob->exec();
}

void testSubscribe(Session *session)
{
    qDebug() << "TESTING: SUBSCRIBE/UNSUBSCRIBE";
    //setup
    CreateJob *create = new CreateJob(session);
    create->setMailBox(QLatin1String("INBOX/TestFolder"));
    create->exec();

    qDebug() << "Listing  subscribed mailboxes with name TestFolder:";
    listFolders(session, false, QLatin1String("TestFolder"));

    //actual tests
    qDebug() << "Subscribing to INBOX/TestFolder";
    SubscribeJob *subscribe = new SubscribeJob(session);
    subscribe->setMailBox(QLatin1String("INBOX/TestFolder"));
    subscribe->exec();

    qDebug() << "Listing  subscribed mailboxes with name TestFolder:";
    listFolders(session, false, QLatin1String("TestFolder"));

    qDebug() << "Unsubscribing from INBOX/TestFolder";
    UnsubscribeJob *unsubscribe = new UnsubscribeJob(session);
    unsubscribe->setMailBox(QLatin1String("INBOX/TestFolder"));
    unsubscribe->exec();

    qDebug() << "Listing  subscribed mailboxes with name TestFolder:";
    listFolders(session, false, QLatin1String("TestFolder"));

    //cleanup
    DeleteJob *deletejob = new DeleteJob(session);
    deletejob->setMailBox(QLatin1String("INBOX/TestFolder"));
    deletejob->exec();
}

void testDelete(Session *session)
{
    qDebug() << "TESTING: DELETE";
    qDebug() << "Creating INBOX/TestFolder:";
    CreateJob *create = new CreateJob(session);
    create->setMailBox(QLatin1String("INBOX/TestFolder"));
    create->exec();

    qDebug() << "Listing  with name TestFolder  before DELETE:";
    listFolders(session, true, QLatin1String("TestFolder"));

    qDebug() << "Deleting INBOX/TestFolder";
    DeleteJob *deletejob = new DeleteJob(session);
    deletejob->setMailBox(QLatin1String("INBOX/TestFolder"));
    deletejob->exec();

    qDebug() << "Listing with name TestFolder after DELETE:";
    listFolders(session, true, QLatin1String("TestFolder"));
}

int main(int argc, char **argv)
{
    K4AboutData about("TestImapServer", 0, ki18n("TestImapServer"), "version");
    KComponentData cData(&about);

    if (argc < 4) {
        qCritical() << "Not enough parameters, expecting: <server> <user> <password>";
    }

    QString server = QString::fromLocal8Bit(argv[1]);
    int port = 143;
    if (server.count(QLatin1Char(':')) == 1) {
        port = server.split(QLatin1Char(':')).last().toInt();
        server = server.split(QLatin1Char(':')).first();
    }
    QString user = QString::fromLocal8Bit(argv[2]);
    QString password = QString::fromLocal8Bit(argv[3]);

    qDebug() << "Querying:" << server << port << user << password;
    qDebug();

    QCoreApplication app(argc, argv);
    Session session(server, port);
    UiProxy::Ptr proxy(new UiProxy());
    session.setUiProxy(proxy);

    qDebug() << "Logging in...";
    LoginJob *login = new LoginJob(&session);
    //login->setEncryptionMode( LoginJob::TlsV1 );
    //login->setAuthenticationMode( LoginJob::Plain );
    login->setUserName(user);
    login->setPassword(password);
    login->exec();
    qDebug();

    /*if (login->encryptionMode() == LoginJob::Unencrypted)
    {
      qDebug() << "Encrypted login not possible, try to log in without encryption";
      login = new LoginJob( &session );
      login->setUserName( user );
      login->setPassword( password );
      login->exec();
      Q_ASSERT_X( login->error() == 0, "LoginJob", login->errorString().toLocal8Bit().constData() );
      Q_ASSERT( session.state() == Session::Authenticated );
      qDebug();

    }*/

    qDebug() << "Server greeting:" << session.serverGreeting();

    qDebug() << "Asking for capabilities:";
    CapabilitiesJob *capabilities = new CapabilitiesJob(&session);
    capabilities->exec();
    Q_ASSERT_X(capabilities->error() == 0, "CapabilitiesJob", capabilities->errorString().toLocal8Bit().constData());
    Q_ASSERT(session.state() == Session::Authenticated);
    qDebug() << capabilities->capabilities();
    qDebug();

    qDebug() << "Asking for namespaces:";
    NamespaceJob *namespaces = new NamespaceJob(&session);
    namespaces->exec();
    Q_ASSERT_X(namespaces->error() == 0, "CapabilitiesJob", namespaces->errorString().toLocal8Bit().constData());
    Q_ASSERT(session.state() == Session::Authenticated);

    qDebug() << "Contains empty namespace:" << namespaces->containsEmptyNamespace();

    qDebug() << "Personal:";
    foreach (MailBoxDescriptor ns, namespaces->personalNamespaces()) {
        qDebug() << ns.separator << ns.name;
    }

    qDebug() << "User:    ";
    foreach (MailBoxDescriptor ns, namespaces->userNamespaces()) {
        qDebug() << ns.separator << ns.name;
    }

    qDebug() << "Shared:  ";
    foreach (MailBoxDescriptor ns, namespaces->sharedNamespaces()) {
        qDebug() << ns.separator << ns.name;
    }
    qDebug();

    qDebug() << "Listing mailboxes:";
    listFolders(&session);
    Q_ASSERT(session.state() == Session::Authenticated);

    qDebug() << "Selecting INBOX:";
    SelectJob *select = new SelectJob(&session);
    select->setMailBox(QLatin1String("INBOX"));
    select->exec();
    Q_ASSERT_X(select->error() == 0, "SelectJob", select->errorString().toLocal8Bit().constData());
    Q_ASSERT(session.state() == Session::Selected);
    qDebug() << "Flags:" << select->flags();
    qDebug() << "Permanent flags:" << select->permanentFlags();
    qDebug() << "Total Number of Messages:" << select->messageCount();
    qDebug() << "Number of recent Messages:" << select->recentCount();
    qDebug() << "First Unseen Message Index:" << select->firstUnseenIndex();
    qDebug() << "UID validity:" << select->uidValidity();
    qDebug() << "Next UID:" << select->nextUid();
    qDebug();

    qDebug() << "Fetching first 3 messages headers:";
    FetchJob *fetch = new FetchJob(&session);
    FetchJob::FetchScope scope;
    fetch->setSequenceSet(ImapSet(1, 3));
    scope.parts.clear();
    scope.mode = FetchJob::FetchScope::Headers;
    fetch->setScope(scope);
    fetch->exec();
    Q_ASSERT_X(fetch->error() == 0, "FetchJob", fetch->errorString().toLocal8Bit().constData());
    Q_ASSERT(session.state() == Session::Selected);
    QMap<qint64, MessagePtr> messages = fetch->messages();
    foreach (qint64 id, messages.keys()) {
        qDebug() << "* Message" << id << "(" << fetch->sizes()[id] << "bytes )";
        qDebug() << "  From      :" << messages[id]->from()->asUnicodeString();
        qDebug() << "  To        :" << messages[id]->to()->asUnicodeString();
        qDebug() << "  Date      :" << messages[id]->date()->asUnicodeString();
        qDebug() << "  Subject   :" << messages[id]->subject()->asUnicodeString();
        qDebug() << "  Message-ID:" << messages[id]->messageID()->asUnicodeString();
    }
    qDebug();

    qDebug() << "Fetching first 3 messages flags:";
    fetch = new FetchJob(&session);
    fetch->setSequenceSet(ImapSet(1, 3));
    scope.parts.clear();
    scope.mode = FetchJob::FetchScope::Flags;
    fetch->setScope(scope);
    fetch->exec();
    Q_ASSERT_X(fetch->error() == 0, "FetchJob", fetch->errorString().toLocal8Bit().constData());
    Q_ASSERT(session.state() == Session::Selected);
    QMap<qint64, MessageFlags> flags = fetch->flags();
    foreach (qint64 id, flags.keys()) {
        qDebug() << "* Message" << id << "flags:" << flags[id];
    }
    qDebug();

    qDebug() << "Fetching first message structure:";
    fetch = new FetchJob(&session);
    fetch->setSequenceSet(ImapSet(1));
    scope.parts.clear();
    scope.mode = FetchJob::FetchScope::Structure;
    fetch->setScope(scope);
    fetch->exec();
    Q_ASSERT_X(fetch->error() == 0, "FetchJob", fetch->errorString().toLocal8Bit().constData());
    Q_ASSERT(session.state() == Session::Selected);
    MessagePtr message = fetch->messages()[1];
    dumpContentHelper(message.get());
    qDebug();

    qDebug() << "Fetching first message second part headers:";
    fetch = new FetchJob(&session);
    fetch->setSequenceSet(ImapSet(1));
    scope.parts.clear();
    scope.parts << "2";
    scope.mode = FetchJob::FetchScope::Headers;
    fetch->setScope(scope);
    fetch->exec();
    Q_ASSERT_X(fetch->error() == 0, "FetchJob", fetch->errorString().toLocal8Bit().constData());
    Q_ASSERT(session.state() == Session::Selected);
    QMap<qint64, MessageParts> allParts = fetch->parts();
    foreach (qint64 id, allParts.keys()) {
        qDebug() << "* Message" << id << "parts headers";
        MessageParts parts = allParts[id];
        foreach (const QByteArray &partId, parts.keys()) {
            qDebug() << "  ** Part" << partId;
            qDebug() << "     Name       :" << parts[partId]->contentType()->name();
            qDebug() << "     Mimetype   :" << parts[partId]->contentType()->mimeType();
            qDebug() << "     Description:" << parts[partId]->contentDescription()->asUnicodeString().simplified();
        }
    }
    qDebug();

    qDebug() << "Fetching first message second part content:";
    fetch = new FetchJob(&session);
    fetch->setSequenceSet(ImapSet(1));
    scope.parts.clear();
    scope.parts << "2";
    scope.mode = FetchJob::FetchScope::Content;
    fetch->setScope(scope);
    fetch->exec();
    Q_ASSERT_X(fetch->error() == 0, "FetchJob", fetch->errorString().toLocal8Bit().constData());
    Q_ASSERT(session.state() == Session::Selected);
    allParts = fetch->parts();
    foreach (int id, allParts.keys()) {
        MessageParts parts = allParts[id];
        foreach (const QByteArray &partId, parts.keys()) {
            qDebug() << "* Message" << id << "part" << partId << "content:";
            qDebug() << parts[partId]->body();
        }
    }
    qDebug();

    testDelete(&session);

    testSubscribe(&session);

    testRename(&session);

    testAppendAndStore(&session);

    testAcl(&session, user);

    testMetaData(&session);

    qDebug() << "Expunge INBOX:";
    ExpungeJob *expunge = new ExpungeJob(&session);
    expunge->exec();

    qDebug() << "Closing INBOX:";
    CloseJob *close = new CloseJob(&session);
    close->exec();
    Q_ASSERT(session.state() == Session::Authenticated);
    qDebug();

    qDebug() << "Logging out...";
    LogoutJob *logout = new LogoutJob(&session);
    logout->exec();
    Q_ASSERT_X(logout->error() == 0, "LogoutJob", logout->errorString().toLocal8Bit().constData());
    Q_ASSERT(session.state() == Session::Disconnected);

    return 0;
}
