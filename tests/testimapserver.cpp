#include <kcomponentdata.h>
#include <kaboutdata.h>
#include <kdebug.h>
#include <qtcpsocket.h>
#include <qcoreapplication.h>
#include <qsignalspy.h>

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

using namespace KIMAP;

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

void listFolders(Session *session, bool includeUnsubscribed = false, const QByteArray& nameFilter = "")
{
  ListJob *list = new ListJob(session);
  list->setIncludeUnsubscribed(includeUnsubscribed);
  list->exec();
  Q_ASSERT_X(list->error()==0, "ListJob", list->errorString().toLocal8Bit());
  int count = list->mailBoxes().size();
  for (int i=0; i<count; ++i) {
    QList<QByteArray> descriptor = list->mailBoxes()[i];
    QByteArray mailBox;
    for (int j=1; j<descriptor.size(); ++j) {
      if (j!=1) mailBox+=descriptor[0];
      mailBox+=descriptor[j];
    }
    if (mailBox.endsWith(nameFilter))
      kDebug() << mailBox;
  }
  
}

void testAppend(Session *session)
{
  kDebug() << "TESTING: APPEND";
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
  fetch->setSequenceSet("1");
  scope.parts.clear();
  scope.mode = FetchJob::FetchScope::Content;
  fetch->setScope(scope);
  fetch->exec();
  QSharedPointer<KMime::Message> message = fetch->messages()[1];
  Q_ASSERT_X(fetch->error()==0, "FetchJob", fetch->errorString().toLocal8Bit());
  Q_ASSERT_X(testMailContent==message->head()+message->body(),
             "Message differs from reference", message->head()+message->body());

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
  rename->setMailBox("INBOX/TestFolder");
  rename->setNewMailBox("INBOX/RenamedTestFolder");
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

  QCoreApplication app(argc, argv);
  Session session(server, port);
 
  kDebug() << "Logging in...";
  LoginJob *login = new LoginJob(&session);
  login->setEncryptionMode(LoginJob::TlsV1);
  login->setUserName(user);
  login->setPassword(password);
  login->exec();
  Q_ASSERT_X(login->error()==0, "LoginJob", login->errorString().toLocal8Bit());
  Q_ASSERT(session.state()==Session::Authenticated);
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
  fetch->setSequenceSet("1:3");
  scope.parts.clear();
  scope.mode = FetchJob::FetchScope::Headers;
  fetch->setScope(scope);
  fetch->exec();
  Q_ASSERT_X(fetch->error()==0, "FetchJob", fetch->errorString().toLocal8Bit());
  Q_ASSERT(session.state()==Session::Selected);
  QMap<int, QSharedPointer<KMime::Message> > messages = fetch->messages();
  foreach (int id, messages.keys()) {
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
  fetch->setSequenceSet("1:3");
  scope.parts.clear();
  scope.mode = FetchJob::FetchScope::Flags;
  fetch->setScope(scope);
  fetch->exec();
  Q_ASSERT_X(fetch->error()==0, "FetchJob", fetch->errorString().toLocal8Bit());
  Q_ASSERT(session.state()==Session::Selected);
  QMap<int, QList<QByteArray> > flags = fetch->flags();
  foreach (int id, flags.keys()) {
    kDebug() << "* Message" << id << "flags:" << flags[id];
  }
  qDebug();

  kDebug() << "Fetching first message structure:";
  fetch = new FetchJob(&session);
  fetch->setSequenceSet("1");
  scope.parts.clear();
  scope.mode = FetchJob::FetchScope::Structure;
  fetch->setScope(scope);
  fetch->exec();
  Q_ASSERT_X(fetch->error()==0, "FetchJob", fetch->errorString().toLocal8Bit());
  Q_ASSERT(session.state()==Session::Selected);
  QSharedPointer<KMime::Message> message = fetch->messages()[1];
  dumpContentHelper(message.data());
  qDebug();

  kDebug() << "Fetching first message second part headers:";
  fetch = new FetchJob(&session);
  fetch->setSequenceSet("1");
  scope.parts.clear();
  scope.parts << "2";
  scope.mode = FetchJob::FetchScope::Headers;
  fetch->setScope(scope);
  fetch->exec();
  Q_ASSERT_X(fetch->error()==0, "FetchJob", fetch->errorString().toLocal8Bit());
  Q_ASSERT(session.state()==Session::Selected);
  QMap<int, QMap<QByteArray, QSharedPointer<KMime::Content> > > allParts = fetch->parts();
  foreach (int id, allParts.keys()) {
    kDebug() << "* Message" << id << "parts headers";
    QMap<QByteArray, QSharedPointer<KMime::Content> > parts = allParts[id];
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
  fetch->setSequenceSet("1");
  scope.parts.clear();
  scope.parts << "2";
  scope.mode = FetchJob::FetchScope::Content;
  fetch->setScope(scope);
  fetch->exec();
  Q_ASSERT_X(fetch->error()==0, "FetchJob", fetch->errorString().toLocal8Bit());
  Q_ASSERT(session.state()==Session::Selected);
  allParts = fetch->parts();
  foreach (int id, allParts.keys()) {
    QMap<QByteArray, QSharedPointer<KMime::Content> > parts = allParts[id];
    foreach (const QByteArray &partId, parts.keys()) {
      kDebug() << "* Message" << id << "part" << partId << "content:";
      kDebug() << parts[partId]->body();
    }
  }
  qDebug();

  testDelete(&session);
  
  testSubscribe(&session);

  testRename(&session);

  testAppend(&session);
  
  kDebug() << "Expunge INBOX:";
  ExpungeJob *expunge = new ExpungeJob(&session);
  expunge->exec();
  kDebug() << "Deleted items: " << expunge->deletedItems();

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
