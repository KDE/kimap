/**
 * This file is part of the KDE project
 * SPDX-FileCopyrightText: 2009 Kevin Ottens <ervin@kde.org>
 * SPDX-FileCopyrightText: 2009 Andras Mantia <amantia@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include <QCoreApplication>
#include <QDebug>

#include "acl.h"
#include "appendjob.h"
#include "capabilitiesjob.h"
#include "closejob.h"
#include "createjob.h"
#include "deleteacljob.h"
#include "deletejob.h"
#include "expungejob.h"
#include "fetchjob.h"
#include "getacljob.h"
#include "getmetadatajob.h"
#include "listjob.h"
#include "listrightsjob.h"
#include "loginjob.h"
#include "logoutjob.h"
#include "myrightsjob.h"
#include "namespacejob.h"
#include "renamejob.h"
#include "selectjob.h"
#include "session.h"
#include "sessionuiproxy.h"
#include "setacljob.h"
#include "setmetadatajob.h"
#include "storejob.h"
#include "subscribejob.h"
#include "unsubscribejob.h"

using namespace KIMAP;

using PartsReceivedSignal = void (FetchJob::*)(const QString &, const QMap<qint64, qint64> &, const QMap<qint64, MessageParts> &);

using HeadersReceivedSignal = void (FetchJob::*)(const QString &,
                                                 const QMap<qint64, qint64> &,
                                                 const QMap<qint64, qint64> &,
                                                 const QMap<qint64, MessageFlags> &,
                                                 const QMap<qint64, MessagePtr> &);

class UiProxy : public SessionUiProxy
{
public:
    bool ignoreSslError(const KSslErrorUiData &errorData) override
    {
        Q_UNUSED(errorData)
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
    auto list = new ListJob(session);
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
    auto create = new CreateJob(session);
    create->setMailBox(QStringLiteral("INBOX/TestFolder"));
    create->exec();

    auto setmetadata = new SetMetaDataJob(session);
    setmetadata->setMailBox(QStringLiteral("INBOX/TestFolder"));
    setmetadata->setServerCapability(SetMetaDataJob::Annotatemore);
    setmetadata->setEntry("/comment");
    setmetadata->addMetaData("value.priv", "My new comment");
    setmetadata->exec();

    setmetadata = new SetMetaDataJob(session);
    setmetadata->setMailBox(QStringLiteral("INBOX/TestFolder"));
    setmetadata->setServerCapability(SetMetaDataJob::Annotatemore);
    setmetadata->setEntry("/check");
    setmetadata->addMetaData("value.priv", "true");
    setmetadata->exec();

    auto getmetadata = new GetMetaDataJob(session);
    getmetadata->setMailBox(QStringLiteral("INBOX/TestFolder"));
    getmetadata->setServerCapability(SetMetaDataJob::Annotatemore);
    getmetadata->addEntry("/*", "value.priv");
    getmetadata->exec();
    Q_ASSERT_X(getmetadata->metaData(QLatin1String("INBOX/TestFolder"), "/check", "value.priv") == "true", "", "/check metadata should be true");
    Q_ASSERT_X(getmetadata->metaData(QLatin1String("INBOX/TestFolder"), "/comment", "value.priv") == "My new comment",
               "",
               "/check metadata should be My new comment");

    // cleanup
    auto deletejob = new DeleteJob(session);
    deletejob->setMailBox(QLatin1String("INBOX/TestFolder"));
    deletejob->exec();
}

void testAcl(Session *session, const QString &user)
{
    qDebug() << "TESTING: ACL commands";
    auto create = new CreateJob(session);
    create->setMailBox(QStringLiteral("INBOX/TestFolder"));
    create->exec();

    auto listRights = new ListRightsJob(session);
    listRights->setMailBox(QStringLiteral("INBOX/TestFolder"));
    listRights->setIdentifier(user.toLatin1());
    listRights->exec();
    qDebug() << "Default rights on INBOX/TestFolder: " << Acl::rightsToString(listRights->defaultRights());
    const QList<Acl::Rights> possible = listRights->possibleRights();
    QStringList strList;
    for (Acl::Rights r : std::as_const(possible)) {
        strList << QString::fromLatin1(Acl::rightsToString(r));
    }
    qDebug() << "Possible rights on INBOX/TestFolder: " << strList;

    auto myRights = new MyRightsJob(session);
    myRights->setMailBox(QStringLiteral("INBOX/TestFolder"));
    myRights->exec();

    Acl::Rights mine = myRights->rights();
    qDebug() << "My rights on INBOX/TestFolder: " << Acl::rightsToString(mine);
    qDebug() << "Reading INBOX/TestFolder is possible: " << myRights->hasRightEnabled(Acl::Read);
    Q_ASSERT_X(myRights->hasRightEnabled(Acl::Read), "Reading INBOX is NOT possible", "");

    auto getAcl = new GetAclJob(session);
    getAcl->setMailBox(QStringLiteral("INBOX/TestFolder"));
    getAcl->exec();
    qDebug() << "Anyone rights on INBOX/TestFolder: " << getAcl->rights("anyone");
    Acl::Rights users = getAcl->rights(user.toLatin1());
    qDebug() << user << " rights on INBOX/TestFolder: " << Acl::rightsToString(users);
    Q_ASSERT_X(mine == users, "GETACL returns different rights for the same user", "");

    qDebug() << "Removing Delete right ";
    mine = Acl::Delete;
    auto setAcl = new SetAclJob(session);
    setAcl->setMailBox(QStringLiteral("INBOX/TestFolder"));
    setAcl->setIdentifier(user.toLatin1());
    setAcl->setRights(AclJobBase::Remove, mine);
    setAcl->exec();

    getAcl = new GetAclJob(session);
    getAcl->setMailBox(QStringLiteral("INBOX/TestFolder"));
    getAcl->exec();
    users = getAcl->rights(user.toLatin1());
    qDebug() << user << " rights on INBOX/TestFolder: " << Acl::rightsToString(users);

    qDebug() << "Adding back Delete right ";
    mine = Acl::Delete;
    setAcl = new SetAclJob(session);
    setAcl->setMailBox(QStringLiteral("INBOX/TestFolder"));
    setAcl->setIdentifier(user.toLatin1());
    setAcl->setRights(AclJobBase::Add, mine);
    setAcl->exec();

    getAcl = new GetAclJob(session);
    getAcl->setMailBox(QStringLiteral("INBOX/TestFolder"));
    getAcl->exec();
    users = getAcl->rights(user.toLatin1());
    qDebug() << user << " rights on INBOX/TestFolder: " << Acl::rightsToString(users);

    // cleanup
    auto deletejob = new DeleteJob(session);
    deletejob->setMailBox(QStringLiteral("INBOX/TestFolder"));
    deletejob->exec();
}

void testAppendAndStore(Session *session)
{
    qDebug() << "TESTING: APPEND and STORE";
    // setup
    auto create = new CreateJob(session);
    create->setMailBox(QStringLiteral("INBOX/TestFolder"));
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
    auto append = new AppendJob(session);
    append->setMailBox(QStringLiteral("INBOX/TestFolder"));
    append->setContent(testMailContent);
    append->exec();
    Q_ASSERT_X(append->error() == 0, "AppendJob", append->errorString().toLocal8Bit().constData());

    qDebug() << "Read the message back and compare...";
    auto select = new SelectJob(session);
    select->setMailBox(QStringLiteral("INBOX/TestFolder"));
    select->exec();

    auto fetch = new FetchJob(session);
    FetchJob::FetchScope scope;
    fetch->setSequenceSet(ImapSet(1));
    scope.parts.clear();
    scope.mode = FetchJob::FetchScope::Content;
    fetch->setScope(scope);
    MessagePtr message;
    QObject::connect(fetch,
                     static_cast<HeadersReceivedSignal>(&FetchJob::headersReceived),
                     fetch,
                     [&](const QString &,
                         const QMap<qint64, qint64> &,
                         const QMap<qint64, qint64> &,
                         const QMap<qint64, MessageFlags> &,
                         const QMap<qint64, MessagePtr> &msgs) {
                         message = msgs[1];
                     });
    fetch->exec();
    Q_ASSERT_X(fetch->error() == 0, "FetchJob", fetch->errorString().toLocal8Bit().constData());
    testMailContent.replace("\r\n", "\n");
    Q_ASSERT_X(testMailContent == message->head() + "\n" + message->body(),
               "Message differs from reference",
               QByteArray(message->head() + "\n" + message->body()).constData());

    fetch = new FetchJob(session);
    fetch->setSequenceSet(ImapSet(1));
    scope.parts.clear();
    scope.mode = FetchJob::FetchScope::Flags;
    fetch->setScope(scope);
    MessageFlags expectedFlags;
    QObject::connect(fetch,
                     static_cast<HeadersReceivedSignal>(&FetchJob::headersReceived),
                     fetch,
                     [&](const QString &,
                         const QMap<qint64, qint64> &,
                         const QMap<qint64, qint64> &,
                         const QMap<qint64, MessageFlags> &flags,
                         const QMap<qint64, MessagePtr> &) {
                         expectedFlags = flags[1];
                     });
    fetch->exec();
    qDebug() << "Read the message flags:" << expectedFlags;

    qDebug() << "Add the \\Deleted flag...";
    expectedFlags << "\\Deleted";
    std::sort(expectedFlags.begin(), expectedFlags.end());
    auto store = new StoreJob(session);
    store->setSequenceSet(ImapSet(1));
    store->setMode(StoreJob::AppendFlags);
    store->setFlags(QList<QByteArray>() << "\\Deleted");
    store->exec();
    Q_ASSERT_X(store->error() == 0, "StoreJob", store->errorString().toLocal8Bit().constData());

    QList<QByteArray> resultingFlags = store->resultingFlags()[1];
    std::sort(resultingFlags.begin(), resultingFlags.end());
    if (expectedFlags != resultingFlags) {
        qDebug() << resultingFlags;
    }
    Q_ASSERT(expectedFlags == resultingFlags);

    select = new SelectJob(session);
    select->setMailBox(QStringLiteral("INBOX"));
    select->exec();

    // cleanup
    auto deletejob = new DeleteJob(session);
    deletejob->setMailBox(QStringLiteral("INBOX/TestFolder"));
    deletejob->exec();
    deletejob = new DeleteJob(session);
    deletejob->setMailBox(QStringLiteral("INBOX/RenamedTestFolder"));
    deletejob->exec();
}

void testRename(Session *session)
{
    qDebug() << "TESTING: RENAME";
    // setup
    auto create = new CreateJob(session);
    create->setMailBox(QStringLiteral("INBOX/TestFolder"));
    create->exec();

    qDebug() << "Listing mailboxes with name TestFolder:";
    listFolders(session, true, QStringLiteral("TestFolder"));

    // actual tests
    qDebug() << "Renaming to RenamedTestFolder";
    auto rename = new RenameJob(session);
    rename->setSourceMailBox(QStringLiteral("INBOX/TestFolder"));
    rename->setDestinationMailBox(QStringLiteral("INBOX/RenamedTestFolder"));
    rename->exec();

    qDebug() << "Listing mailboxes with name TestFolder:";
    listFolders(session, true, QStringLiteral("TestFolder"));
    qDebug() << "Listing mailboxes with name RenamedTestFolder:";
    listFolders(session, true, QStringLiteral("RenamedTestFolder"));

    // cleanup
    auto deletejob = new DeleteJob(session);
    deletejob->setMailBox(QStringLiteral("INBOX/TestFolder"));
    deletejob->exec();
    deletejob = new DeleteJob(session);
    deletejob->setMailBox(QStringLiteral("INBOX/RenamedTestFolder"));
    deletejob->exec();
}

void testSubscribe(Session *session)
{
    qDebug() << "TESTING: SUBSCRIBE/UNSUBSCRIBE";
    // setup
    auto create = new CreateJob(session);
    create->setMailBox(QStringLiteral("INBOX/TestFolder"));
    create->exec();

    qDebug() << "Listing  subscribed mailboxes with name TestFolder:";
    listFolders(session, false, QStringLiteral("TestFolder"));

    // actual tests
    qDebug() << "Subscribing to INBOX/TestFolder";
    auto subscribe = new SubscribeJob(session);
    subscribe->setMailBox(QStringLiteral("INBOX/TestFolder"));
    subscribe->exec();

    qDebug() << "Listing  subscribed mailboxes with name TestFolder:";
    listFolders(session, false, QStringLiteral("TestFolder"));

    qDebug() << "Unsubscribing from INBOX/TestFolder";
    auto unsubscribe = new UnsubscribeJob(session);
    unsubscribe->setMailBox(QStringLiteral("INBOX/TestFolder"));
    unsubscribe->exec();

    qDebug() << "Listing  subscribed mailboxes with name TestFolder:";
    listFolders(session, false, QStringLiteral("TestFolder"));

    // cleanup
    auto deletejob = new DeleteJob(session);
    deletejob->setMailBox(QStringLiteral("INBOX/TestFolder"));
    deletejob->exec();
}

void testDelete(Session *session)
{
    qDebug() << "TESTING: DELETE";
    qDebug() << "Creating INBOX/TestFolder:";
    auto create = new CreateJob(session);
    create->setMailBox(QStringLiteral("INBOX/TestFolder"));
    create->exec();

    qDebug() << "Listing  with name TestFolder  before DELETE:";
    listFolders(session, true, QStringLiteral("TestFolder"));

    qDebug() << "Deleting INBOX/TestFolder";
    auto deletejob = new DeleteJob(session);
    deletejob->setMailBox(QStringLiteral("INBOX/TestFolder"));
    deletejob->exec();

    qDebug() << "Listing with name TestFolder after DELETE:";
    listFolders(session, true, QStringLiteral("TestFolder"));
}

int main(int argc, char **argv)
{
    QCoreApplication::setApplicationName(QStringLiteral("TestImapServer"));

    if (argc < 4) {
        qCritical() << "Not enough parameters, expecting: <server> <user> <password>";
    }

    QString server = QString::fromLocal8Bit(argv[1]);
    int port = 143;
    if (server.count(QLatin1Char(':')) == 1) {
        const QStringList lstSplit = server.split(QLatin1Char(':'));
        port = lstSplit.last().toInt();
        server = lstSplit.first();
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
    auto login = new LoginJob(&session);
    // login->setEncryptionMode( LoginJob::TlsV1 );
    // login->setAuthenticationMode( LoginJob::Plain );
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
    auto capabilities = new CapabilitiesJob(&session);
    capabilities->exec();
    Q_ASSERT_X(capabilities->error() == 0, "CapabilitiesJob", capabilities->errorString().toLocal8Bit().constData());
    Q_ASSERT(session.state() == Session::Authenticated);
    qDebug() << capabilities->capabilities();
    qDebug();

    qDebug() << "Asking for namespaces:";
    auto namespaces = new NamespaceJob(&session);
    namespaces->exec();
    Q_ASSERT_X(namespaces->error() == 0, "CapabilitiesJob", namespaces->errorString().toLocal8Bit().constData());
    Q_ASSERT(session.state() == Session::Authenticated);

    qDebug() << "Contains empty namespace:" << namespaces->containsEmptyNamespace();

    qDebug() << "Personal:";
    const auto personalNamespaces = namespaces->personalNamespaces();
    for (MailBoxDescriptor ns : personalNamespaces) {
        qDebug() << ns.separator << ns.name;
    }

    qDebug() << "User:    ";
    const auto userNamespaces = namespaces->userNamespaces();
    for (MailBoxDescriptor ns : userNamespaces) {
        qDebug() << ns.separator << ns.name;
    }

    qDebug() << "Shared:  ";
    const auto sharedNamespaces = namespaces->sharedNamespaces();
    for (MailBoxDescriptor ns : sharedNamespaces) {
        qDebug() << ns.separator << ns.name;
    }
    qDebug();

    qDebug() << "Listing mailboxes:";
    listFolders(&session);
    Q_ASSERT(session.state() == Session::Authenticated);

    qDebug() << "Selecting INBOX:";
    auto select = new SelectJob(&session);
    select->setMailBox(QStringLiteral("INBOX"));
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
    auto fetch = new FetchJob(&session);
    FetchJob::FetchScope scope;
    fetch->setSequenceSet(ImapSet(1, 3));
    scope.parts.clear();
    scope.mode = FetchJob::FetchScope::Headers;
    fetch->setScope(scope);
    QMap<qint64, qint64> sizes;
    QMap<qint64, MessagePtr> messages;

    QObject::connect(fetch,
                     static_cast<HeadersReceivedSignal>(&FetchJob::headersReceived),
                     fetch,
                     [&](const QString &,
                         const QMap<qint64, qint64> &,
                         const QMap<qint64, qint64> &sizes_,
                         const QMap<qint64, MessageFlags> &,
                         const QMap<qint64, MessagePtr> &msgs_) {
                         sizes = sizes_;
                         messages = msgs_;
                     });
    fetch->exec();
    Q_ASSERT_X(fetch->error() == 0, "FetchJob", fetch->errorString().toLocal8Bit().constData());
    Q_ASSERT(session.state() == Session::Selected);
    const auto messagesKey = messages.keys();
    for (qint64 id : messagesKey) {
        qDebug() << "* Message" << id << "(" << sizes[id] << "bytes )";
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
    QMap<qint64, MessageFlags> flags;
    QObject::connect(fetch,
                     static_cast<HeadersReceivedSignal>(&FetchJob::headersReceived),
                     fetch,
                     [&](const QString &,
                         const QMap<qint64, qint64> &,
                         const QMap<qint64, qint64> &,
                         const QMap<qint64, MessageFlags> &flags_,
                         const QMap<qint64, MessagePtr> &) {
                         flags = flags_;
                     });
    fetch->exec();
    Q_ASSERT_X(fetch->error() == 0, "FetchJob", fetch->errorString().toLocal8Bit().constData());
    Q_ASSERT(session.state() == Session::Selected);
    const auto flagsKey = flags.keys();
    for (qint64 id : flagsKey) {
        qDebug() << "* Message" << id << "flags:" << flags[id];
    }
    qDebug();

    qDebug() << "Fetching first message structure:";
    fetch = new FetchJob(&session);
    fetch->setSequenceSet(ImapSet(1));
    scope.parts.clear();
    scope.mode = FetchJob::FetchScope::Structure;
    fetch->setScope(scope);
    QObject::connect(fetch,
                     static_cast<HeadersReceivedSignal>(&FetchJob::headersReceived),
                     fetch,
                     [&](const QString &,
                         const QMap<qint64, qint64> &,
                         const QMap<qint64, qint64> &,
                         const QMap<qint64, MessageFlags> &,
                         const QMap<qint64, MessagePtr> &msgs_) {
                         messages = msgs_;
                     });
    fetch->exec();
    Q_ASSERT_X(fetch->error() == 0, "FetchJob", fetch->errorString().toLocal8Bit().constData());
    Q_ASSERT(session.state() == Session::Selected);
    MessagePtr message = messages[1];
    dumpContentHelper(message.data());
    qDebug();

    qDebug() << "Fetching first message second part headers:";
    fetch = new FetchJob(&session);
    fetch->setSequenceSet(ImapSet(1));
    scope.parts.clear();
    scope.parts << "2";
    scope.mode = FetchJob::FetchScope::Headers;
    fetch->setScope(scope);
    QMap<qint64, MessageParts> allParts;
    QObject::connect(fetch,
                     static_cast<PartsReceivedSignal>(&FetchJob::partsReceived),
                     fetch,
                     [&](const QString &, const QMap<qint64, qint64> &, const QMap<qint64, MessageParts> &parts_) {
                         allParts = parts_;
                     });
    fetch->exec();
    Q_ASSERT_X(fetch->error() == 0, "FetchJob", fetch->errorString().toLocal8Bit().constData());
    Q_ASSERT(session.state() == Session::Selected);
    const auto allkeys = allParts.keys();
    for (qint64 id : allkeys) {
        qDebug() << "* Message" << id << "parts headers";
        MessageParts parts = allParts[id];
        const auto parsKeys = parts.keys();
        for (const QByteArray &partId : parsKeys) {
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
    QObject::connect(fetch,
                     static_cast<PartsReceivedSignal>(&FetchJob::partsReceived),
                     fetch,
                     [&](const QString &, const QMap<qint64, qint64> &, const QMap<qint64, MessageParts> &parts_) {
                         allParts = parts_;
                     });
    fetch->exec();
    Q_ASSERT_X(fetch->error() == 0, "FetchJob", fetch->errorString().toLocal8Bit().constData());
    Q_ASSERT(session.state() == Session::Selected);
    const auto allpartskeys = allParts.keys();
    for (int id : allpartskeys) {
        MessageParts parts = allParts[id];
        const auto partsKeys = parts.keys();
        for (const QByteArray &partId : partsKeys) {
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
    auto expunge = new ExpungeJob(&session);
    expunge->exec();

    qDebug() << "Closing INBOX:";
    auto close = new CloseJob(&session);
    close->exec();
    Q_ASSERT(session.state() == Session::Authenticated);
    qDebug();

    qDebug() << "Logging out...";
    auto logout = new LogoutJob(&session);
    logout->exec();
    Q_ASSERT_X(logout->error() == 0, "LogoutJob", logout->errorString().toLocal8Bit().constData());
    Q_ASSERT(session.state() == Session::Disconnected);

    return 0;
}
