#include <kcomponentdata.h>
#include <kaboutdata.h>
#include <kdebug.h>
#include <qtcpsocket.h>
#include <qapplication.h>
#include <qsignalspy.h>
#include <kmessagebox.h>
#include <kio/sslui.h>

#include "kimap/session.h"
#include "kimap/capabilitiesjob.h"
#include "kimap/idlejob.h"
#include "kimap/loginjob.h"
#include "kimap/logoutjob.h"
#include "kimap/selectjob.h"
#include "kimap/closejob.h"
#include "kimap/sessionuiproxy.h"

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

int main( int argc, char **argv )
{
  KAboutData about("TestImapIdle", 0, ki18n("TestImapIdle"), "version");
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

  kDebug() << "Listening:" << server << port << user << password;
  qDebug();

  QApplication app(argc, argv);
  Session session(server, port);
  UiProxy *proxy = new UiProxy();
  session.setUiProxy(proxy);

  kDebug() << "Logging in...";
  LoginJob *login = new LoginJob(&session);
  login->setUserName(user);
  login->setPassword(password);
  login->exec();
  Q_ASSERT_X(login->error()==0, "LoginJob", login->errorString().toLocal8Bit());
  Q_ASSERT(session.state()==Session::Authenticated);
  qDebug();

  kDebug() << "Asking for capabilities:";
  CapabilitiesJob *capabilities = new CapabilitiesJob(&session);
  capabilities->exec();
  Q_ASSERT_X(capabilities->error()==0, "CapabilitiesJob", capabilities->errorString().toLocal8Bit());
  Q_ASSERT(session.state()==Session::Authenticated);
  kDebug() << capabilities->capabilities();
  qDebug();

  Q_ASSERT(capabilities->capabilities().contains("IDLE"));

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

  kDebug() << "Start idling...";
  IdleJob *idle = new IdleJob(&session);
  QObject::connect(idle, SIGNAL(mailBoxStats(IdleJob*, QString, int, int)),
                   idle, SLOT(stop()));
  idle->exec();
  kDebug() << "Idling done for" << idle->lastMailBox()
           << "message count:" << idle->lastMessageCount()
           << "recent count:" << idle->lastRecentCount();

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
