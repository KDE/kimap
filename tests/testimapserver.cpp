#include <kcomponentdata.h>
#include <kaboutdata.h>
#include <kdebug.h>
#include <qtcpsocket.h>
#include <qapplication.h>
#include <qsignalspy.h>
#include <kmessagebox.h>
#include <kio/sslui.h>

#include "kimap/acl.h"
#include "kimap/session.h"
#include "kimap/appendjob.h"
#include "kimap/capabilitiesjob.h"
#include "kimap/fetchjob.h"
#include "kimap/listjob.h"
#include "kimap/loginjob.h"
#include "kimap/logoutjob.h"
#include "kimap/selectjob.h"
#include "kimap/closejob.h"
#include "kimap/expungejob.h"
#include "kimap/createjob.h"
#include "kimap/deletejob.h"
#include "kimap/subscribejob.h"
#include "kimap/unsubscribejob.h"
#include "kimap/renamejob.h"
#include "kimap/storejob.h"
#include "kimap/sessionuiproxy.h"
#include "kimap/setacljob.h"
#include "kimap/getacljob.h"
#include "kimap/deleteacljob.h"
#include "kimap/myrightsjob.h"
#include "kimap/listrightsjob.h"
#include "kimap/setmetadatajob.h"
#include "kimap/getmetadatajob.h"

using namespace KIMAP;

class UiProxy: public SessionUiProxy {
  public:
    bool ignoreSslError(const KSslErrorUiData& errorData) {
      if (KIO::SslUi::askIgnoreSslErrors(errorData, KIO::SslUi::StoreRules)) {
        return true;
      } else {
        return false;
      }
    }
};

void dumpContentHelper(KMime::Content *part, const QString &partId = QString())
{
  if (partId.isEmpty()) {
    kDebug() << "** Message root **";
  } else {
    kDebug() << "** Part" << partId << "**";
  }

  kDebug() << part->head();

  KMime::Content::List children = part->contents();
  for (int i=0; i<children.size(); i++) {
    QString newId = partId;
    if (!newId.isEmpty()) {
      newId+=".";
    }
    newId+=QString::number(i+1);
    dumpContentHelper(children[i], newId);
  }
}

void listFolders(Session *session, bool includeUnsubscribed = false, const QString &nameFilter = "")
{
  ListJob *list = new ListJob(session);
  list->setIncludeUnsubscribed(includeUnsubscribed);
  list->exec();
  Q_ASSERT_X(list->error()==0, "ListJob", list->errorString().toLocal8Bit());
  int count = list->mailBoxes().size();
  for (int i=0; i<count; ++i) {
    MailBoxDescriptor descriptor = list->mailBoxes()[i];
    if (descriptor.name.endsWith(nameFilter))
      kDebug() << descriptor.separator << descriptor.name;
  }

}

void testMetaData(Session *session)
{
  kDebug() << "TESTING: METADATA commands";
  CreateJob *create = new CreateJob(session);
  create->setMailBox("INBOX/TestFolder");
  create->exec();

  SetMetaDataJob *setmetadata = new SetMetaDataJob(session);
  setmetadata->setMailBox("INBOX/TestFolder");
  setmetadata->setServerCapability(SetMetaDataJob::Annotatemore);
  setmetadata->setEntry("/comment");
  setmetadata->addMetaData("value.priv", "My new comment");
  setmetadata->exec();

  setmetadata = new SetMetaDataJob(session);
  setmetadata->setMailBox("INBOX/TestFolder");
  setmetadata->setServerCapability(SetMetaDataJob::Annotatemore);
  setmetadata->setEntry("/check");
  setmetadata->addMetaData("value.priv", "true");
  setmetadata->exec();

  GetMetaDataJob *getmetadata = new GetMetaDataJob(session);
  getmetadata->setMailBox("INBOX/TestFolder");
  getmetadata->setServerCapability(SetMetaDataJob::Annotatemore);
  getmetadata->addEntry("/*","value.priv");
  getmetadata->exec();
  Q_ASSERT_X(getmetadata->metaData("INBOX/TestFolder", "/check", "value.priv") == "true", "",  "/check metadata should be true");
  Q_ASSERT_X(getmetadata->metaData("INBOX/TestFolder", "/comment", "value.priv") == "My new comment", "",  "/check metadata should be My new comment");

  //cleanup
  DeleteJob *deletejob = new DeleteJob(session);
  deletejob->setMailBox("INBOX/TestFolder");
  deletejob->exec();
}

void testAcl(Session *session, const QString &user)
{
  kDebug() << "TESTING: ACL commands";
  CreateJob *create = new CreateJob(session);
  create->setMailBox("INBOX/TestFolder");
  create->exec();

  ListRightsJob *listRights = new ListRightsJob(session);
  listRights->setMailBox("INBOX/TestFolder");
  listRights->setIdentifier(user.toLatin1());
  listRights->exec();
  kDebug() << "Default rights on INBOX/TestFolder: " << Acl::rightsToString(listRights->defaultRights());
  QList<Acl::Rights> possible = listRights->possibleRights();
  QStringList strList;
  Q_FOREACH(Acl::Rights r, possible) {
    strList << Acl::rightsToString(r);
  }
  kDebug() << "Possible rights on INBOX/TestFolder: " << strList;

  MyRightsJob *myRights = new MyRightsJob(session);
  myRights->setMailBox("INBOX/TestFolder");
  myRights->exec();

  Acl::Rights mine = myRights->rights();
  kDebug() << "My rights on INBOX/TestFolder: " << Acl::rightsToString(mine);
  kDebug() << "Reading INBOX/TestFolder is possible: " << myRights->hasRightEnabled(Acl::Read);
  Q_ASSERT_X(myRights->hasRightEnabled(Acl::Read), "Reading INBOX is NOT possible", "");

  GetAclJob *getAcl= new GetAclJob(session);
  getAcl->setMailBox("INBOX/TestFolder");
  getAcl->exec();
  kDebug() << "Anyone rights on INBOX/TestFolder: " << getAcl->rights("anyone");
  Acl::Rights users = getAcl->rights(user.toLatin1());
  kDebug() << user << " rights on INBOX/TestFolder: " << Acl::rightsToString(users);
  Q_ASSERT_X(mine == users, "GETACL returns different rights for the same user", "");


  kDebug() << "Removing Delete right ";
  mine = Acl::Delete;
  SetAclJob *setAcl= new SetAclJob(session);
  setAcl->setMailBox("INBOX/TestFolder");
  setAcl->setIdentifier(user.toLatin1());
  setAcl->setRights(AclJobBase::Remove, mine);
  setAcl->exec();

  getAcl= new GetAclJob(session);
  getAcl->setMailBox("INBOX/TestFolder");
  getAcl->exec();
  users = getAcl->rights(user.toLatin1());
  kDebug() << user << " rights on INBOX/TestFolder: " << Acl::rightsToString(users);

  kDebug() << "Adding back Delete right ";
  mine = Acl::Delete;
  setAcl= new SetAclJob(session);
  setAcl->setMailBox("INBOX/TestFolder");
  setAcl->setIdentifier(user.toLatin1());
  setAcl->setRights(AclJobBase::Add, mine);
  setAcl->exec();

  getAcl= new GetAclJob(session);
  getAcl->setMailBox("INBOX/TestFolder");
  getAcl->exec();
  users = getAcl->rights(user.toLatin1());
  kDebug() << user << " rights on INBOX/TestFolder: " << Acl::rightsToString(users);

  //cleanup
  DeleteJob *deletejob = new DeleteJob(session);
  deletejob->setMailBox("INBOX/TestFolder");
  deletejob->exec();
}

void testAppendAndStore(Session *session)
{
  kDebug() << "TESTING: APPEND and STORE";
  //setup
  CreateJob *create = new CreateJob(session);
  create->setMailBox("INBOX/TestFolder");
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

  kDebug() << "Append a message in INBOX/TestFolder...";
  AppendJob *append = new AppendJob(session);
  append->setMailBox("INBOX/TestFolder");
  append->setContent(testMailContent);
  append->exec();
  Q_ASSERT_X(append->error()==0, "AppendJob", append->errorString().toLocal8Bit());

  kDebug() << "Read the message back and compare...";
  SelectJob *select = new SelectJob(session);
  select->setMailBox("INBOX/TestFolder");
  select->exec();

  FetchJob *fetch = new FetchJob(session);
  FetchJob::FetchScope scope;
  fetch->setSequenceSet(ImapSet(1));
  scope.parts.clear();
  scope.mode = FetchJob::FetchScope::Content;
  fetch->setScope(scope);
  fetch->exec();
  MessagePtr message = fetch->messages()[1];
  Q_ASSERT_X(fetch->error()==0, "FetchJob", fetch->errorString().toLocal8Bit());
  testMailContent.replace( "\r\n", "\n" );
  Q_ASSERT_X(testMailContent==message->head()+"\n"+message->body(),
             "Message differs from reference", message->head()+"\n"+message->body());

  fetch = new FetchJob(session);
  fetch->setSequenceSet(ImapSet(1));
  scope.parts.clear();
  scope.mode = FetchJob::FetchScope::Flags;
  fetch->setScope(scope);
  fetch->exec();
  MessageFlags expectedFlags = fetch->flags()[1];
  kDebug() << "Read the message flags:" << expectedFlags;

  kDebug() << "Add the \\Deleted flag...";
  expectedFlags << "\\Deleted";
  qSort(expectedFlags);
  StoreJob *store = new StoreJob(session);
  store->setSequenceSet(ImapSet(1));
  store->setMode(StoreJob::AppendFlags);
  store->setFlags(QList<QByteArray>() << "\\Deleted");
  store->exec();
  Q_ASSERT_X(store->error()==0, "StoreJob", store->errorString().toLocal8Bit());

  QList<QByteArray> resultingFlags = store->resultingFlags()[1];
  qSort(resultingFlags);
  if (expectedFlags!=resultingFlags) kDebug() << resultingFlags;
  Q_ASSERT(expectedFlags==resultingFlags);


  select = new SelectJob(session);
  select->setMailBox("INBOX");
  select->exec();

  //cleanup
  DeleteJob *deletejob = new DeleteJob(session);
  deletejob->setMailBox("INBOX/TestFolder");
  deletejob->exec();
  deletejob = new DeleteJob(session);
  deletejob->setMailBox("INBOX/RenamedTestFolder");
  deletejob->exec();
}

void testRename(Session *session)
{
  kDebug() << "TESTING: RENAME";
  //setup
  CreateJob *create = new CreateJob(session);
  create->setMailBox("INBOX/TestFolder");
  create->exec();

  kDebug() << "Listing mailboxes with name TestFolder:";
  listFolders(session, true, "TestFolder");

  //actual tests
  kDebug() << "Renaming to RenamedTestFolder";
  RenameJob *rename = new RenameJob(session);
  rename->setSourceMailBox("INBOX/TestFolder");
  rename->setDestinationMailBox("INBOX/RenamedTestFolder");
  rename->exec();

  kDebug() << "Listing mailboxes with name TestFolder:";
  listFolders(session, true, "TestFolder");
  kDebug() << "Listing mailboxes with name RenamedTestFolder:";
  listFolders(session, true, "RenamedTestFolder");

  //cleanup
  DeleteJob *deletejob = new DeleteJob(session);
  deletejob->setMailBox("INBOX/TestFolder");
  deletejob->exec();
  deletejob = new DeleteJob(session);
  deletejob->setMailBox("INBOX/RenamedTestFolder");
  deletejob->exec();
}


void testSubscribe(Session *session)
{
  kDebug() << "TESTING: SUBSCRIBE/UNSUBSCRIBE";
  //setup
  CreateJob *create = new CreateJob(session);
  create->setMailBox("INBOX/TestFolder");
  create->exec();

  kDebug() << "Listing  subscribed mailboxes with name TestFolder:";
  listFolders(session, false, "TestFolder");

  //actual tests
  kDebug() << "Subscribing to INBOX/TestFolder";
  SubscribeJob *subscribe = new SubscribeJob(session);
  subscribe->setMailBox("INBOX/TestFolder");
  subscribe->exec();

  kDebug() << "Listing  subscribed mailboxes with name TestFolder:";
  listFolders(session, false, "TestFolder");

  kDebug() << "Unsubscribing from INBOX/TestFolder";
  UnsubscribeJob *unsubscribe = new UnsubscribeJob(session);
  unsubscribe->setMailBox("INBOX/TestFolder");
  unsubscribe->exec();

  kDebug() << "Listing  subscribed mailboxes with name TestFolder:";
  listFolders(session, false, "TestFolder");

  //cleanup
  DeleteJob *deletejob = new DeleteJob(session);
  deletejob->setMailBox("INBOX/TestFolder");
  deletejob->exec();
}

void testDelete(Session *session)
{
  kDebug() << "TESTING: DELETE";
  kDebug() << "Creating INBOX/TestFolder:";
  CreateJob *create = new CreateJob(session);
  create->setMailBox("INBOX/TestFolder");
  create->exec();


  kDebug() << "Listing  with name TestFolder  before DELETE:";
  listFolders(session, true, "TestFolder");

  kDebug() << "Deleting INBOX/TestFolder";
  DeleteJob *deletejob = new DeleteJob(session);
  deletejob->setMailBox("INBOX/TestFolder");
  deletejob->exec();

  kDebug() << "Listing with name TestFolder after DELETE:";
  listFolders(session, true, "TestFolder");
}

int main( int argc, char **argv )
{
  KAboutData about("TestImapServer", 0, ki18n("TestImapServer"), "version");
  KComponentData cData(&about);

  if (argc < 4) {
    kError() << "Not enough parameters, expecting: <server> <user> <password>";
  }

  QString server = QString::fromLocal8Bit(argv[1]);
  int port = 143;
  if ( server.count( ':' ) == 1 ) {
    port = server.split( ':' ).last().toInt();
    server = server.split( ':' ).first();
  }
  QString user = QString::fromLocal8Bit(argv[2]);
  QString password = QString::fromLocal8Bit(argv[3]);

  kDebug() << "Querying:" << server << port << user << password;
  qDebug();

  QApplication app(argc, argv);
  Session session(server, port);
  UiProxy *proxy = new UiProxy();
  session.setUiProxy(proxy);

  kDebug() << "Logging in...";
  LoginJob *login = new LoginJob(&session);
  login->setEncryptionMode(LoginJob::TlsV1);
  login->setAuthenticationMode(LoginJob::Plain);
  login->setUserName(user);
  login->setPassword(password);
  login->exec();
  qDebug();

  if (login->encryptionMode() == LoginJob::Unencrypted)
  {
    kDebug() << "Encrypted login not possible, try to log in without encryption";
    login = new LoginJob(&session);
    login->setUserName(user);
    login->setPassword(password);
    login->exec();
    Q_ASSERT_X(login->error()==0, "LoginJob", login->errorString().toLocal8Bit());
    Q_ASSERT(session.state()==Session::Authenticated);
    qDebug();

  }

  kDebug() << "Asking for capabilities:";
  CapabilitiesJob *capabilities = new CapabilitiesJob(&session);
  capabilities->exec();
  Q_ASSERT_X(capabilities->error()==0, "CapabilitiesJob", capabilities->errorString().toLocal8Bit());
  Q_ASSERT(session.state()==Session::Authenticated);
  kDebug() << capabilities->capabilities();
  qDebug();

  kDebug() << "Listing mailboxes:";
  listFolders(&session);
  Q_ASSERT(session.state()==Session::Authenticated);

  kDebug() << "Selecting INBOX:";
  SelectJob *select = new SelectJob(&session);
  select->setMailBox("INBOX");
  select->exec();
  Q_ASSERT_X(select->error()==0, "SelectJob", select->errorString().toLocal8Bit());
  Q_ASSERT(session.state()==Session::Selected);
  kDebug() << "Flags:" << select->flags();
  kDebug() << "Permanent flags:" << select->permanentFlags();
  kDebug() << "Total Number of Messages:" << select->messageCount();
  kDebug() << "Number of recent Messages:" << select->recentCount();
  kDebug() << "First Unseen Message Index:" << select->firstUnseenIndex();
  kDebug() << "UID validity:" << select->uidValidity();
  kDebug() << "Next UID:" << select->nextUid();
  qDebug();

  kDebug() << "Fetching first 3 messages headers:";
  FetchJob *fetch = new FetchJob(&session);
  FetchJob::FetchScope scope;
  fetch->setSequenceSet(ImapSet(1, 3));
  scope.parts.clear();
  scope.mode = FetchJob::FetchScope::Headers;
  fetch->setScope(scope);
  fetch->exec();
  Q_ASSERT_X(fetch->error()==0, "FetchJob", fetch->errorString().toLocal8Bit());
  Q_ASSERT(session.state()==Session::Selected);
  QMap<qint64, MessagePtr> messages = fetch->messages();
  foreach (qint64 id, messages.keys()) {
    kDebug() << "* Message" << id << "(" << fetch->sizes()[id] << "bytes )";
    kDebug() << "  From      :" << messages[id]->from()->asUnicodeString();
    kDebug() << "  To        :" << messages[id]->to()->asUnicodeString();
    kDebug() << "  Date      :" << messages[id]->date()->asUnicodeString();
    kDebug() << "  Subject   :" << messages[id]->subject()->asUnicodeString();
    kDebug() << "  Message-ID:" << messages[id]->messageID()->asUnicodeString();
  }
  qDebug();


  kDebug() << "Fetching first 3 messages flags:";
  fetch = new FetchJob(&session);
  fetch->setSequenceSet(ImapSet(1, 3));
  scope.parts.clear();
  scope.mode = FetchJob::FetchScope::Flags;
  fetch->setScope(scope);
  fetch->exec();
  Q_ASSERT_X(fetch->error()==0, "FetchJob", fetch->errorString().toLocal8Bit());
  Q_ASSERT(session.state()==Session::Selected);
  QMap<qint64, MessageFlags> flags = fetch->flags();
  foreach (qint64 id, flags.keys()) {
    kDebug() << "* Message" << id << "flags:" << flags[id];
  }
  qDebug();

  kDebug() << "Fetching first message structure:";
  fetch = new FetchJob(&session);
  fetch->setSequenceSet(ImapSet(1));
  scope.parts.clear();
  scope.mode = FetchJob::FetchScope::Structure;
  fetch->setScope(scope);
  fetch->exec();
  Q_ASSERT_X(fetch->error()==0, "FetchJob", fetch->errorString().toLocal8Bit());
  Q_ASSERT(session.state()==Session::Selected);
  MessagePtr message = fetch->messages()[1];
  dumpContentHelper(message.get());
  qDebug();

  kDebug() << "Fetching first message second part headers:";
  fetch = new FetchJob(&session);
  fetch->setSequenceSet(ImapSet(1));
  scope.parts.clear();
  scope.parts << "2";
  scope.mode = FetchJob::FetchScope::Headers;
  fetch->setScope(scope);
  fetch->exec();
  Q_ASSERT_X(fetch->error()==0, "FetchJob", fetch->errorString().toLocal8Bit());
  Q_ASSERT(session.state()==Session::Selected);
  QMap<qint64, MessageParts> allParts = fetch->parts();
  foreach (qint64 id, allParts.keys()) {
    kDebug() << "* Message" << id << "parts headers";
    MessageParts parts = allParts[id];
    foreach (const QByteArray &partId, parts.keys()) {
      kDebug() << "  ** Part" << partId;
      kDebug() << "     Name       :" << parts[partId]->contentType()->name();
      kDebug() << "     Mimetype   :" << parts[partId]->contentType()->mimeType();
      kDebug() << "     Description:" << parts[partId]->contentDescription()->asUnicodeString().simplified();
    }
  }
  qDebug();

  kDebug() << "Fetching first message second part content:";
  fetch = new FetchJob(&session);
  fetch->setSequenceSet(ImapSet(1));
  scope.parts.clear();
  scope.parts << "2";
  scope.mode = FetchJob::FetchScope::Content;
  fetch->setScope(scope);
  fetch->exec();
  Q_ASSERT_X(fetch->error()==0, "FetchJob", fetch->errorString().toLocal8Bit());
  Q_ASSERT(session.state()==Session::Selected);
  allParts = fetch->parts();
  foreach (int id, allParts.keys()) {
    MessageParts parts = allParts[id];
    foreach (const QByteArray &partId, parts.keys()) {
      kDebug() << "* Message" << id << "part" << partId << "content:";
      kDebug() << parts[partId]->body();
    }
  }
  qDebug();

  testDelete(&session);

  testSubscribe(&session);

  testRename(&session);

  testAppendAndStore(&session);

  testAcl(&session, user);

  testMetaData(&session);

  kDebug() << "Expunge INBOX:";
  ExpungeJob *expunge = new ExpungeJob(&session);
  expunge->exec();

  kDebug() << "Closing INBOX:";
  CloseJob *close = new CloseJob(&session);
  close->exec();
  Q_ASSERT(session.state()==Session::Authenticated);
  qDebug();

  kDebug() << "Logging out...";
  LogoutJob *logout = new LogoutJob(&session);
  logout->exec();
  Q_ASSERT_X(logout->error()==0, "LogoutJob", logout->errorString().toLocal8Bit());
  Q_ASSERT(session.state()==Session::Disconnected);

  return 0;
}
