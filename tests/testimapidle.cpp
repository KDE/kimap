/**
  * This file is part of the KDE project
  * Copyright (C) 2009 Kevin Ottens <ervin@kde.org>
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
#include <klocale.h>
#include <klocalizedstring.h>
#include <qdebug.h>
#include <ktcpsocket.h> // declares KSslErrorUiData
#include <QCoreApplication>
#include <qsignalspy.h>

#include "session.h"
#include "capabilitiesjob.h"
#include "idlejob.h"
#include "loginjob.h"
#include "logoutjob.h"
#include "selectjob.h"
#include "closejob.h"
#include "sessionuiproxy.h"

using namespace KIMAP;

class UiProxy: public SessionUiProxy {
  public:
    bool ignoreSslError(const KSslErrorUiData& errorData) {
        Q_UNUSED( errorData );
        return true;
    }
};

int main( int argc, char **argv )
{
  K4AboutData about( "TestImapIdle", 0, ki18n( "TestImapIdle" ), "version" );
  KComponentData cData( &about );

  if ( argc < 4 ) {
    qCritical() << "Not enough parameters, expecting: <server> <user> <password>";
  }

  QString server = QString::fromLocal8Bit( argv[1] );
  int port = 143;
  if ( server.count( QLatin1Char(':') ) == 1 ) {
    port = server.split( QLatin1Char(':') ).last().toInt();
    server = server.split( QLatin1Char(':') ).first();
  }
  QString user = QString::fromLocal8Bit( argv[2] );
  QString password = QString::fromLocal8Bit( argv[3] );

  qDebug() << "Listening:" << server << port << user << password;
  qDebug();

  QCoreApplication app( argc, argv );
  Session session( server, port );
  UiProxy *proxy = new UiProxy();
  session.setUiProxy( UiProxy::Ptr( proxy ) );

  qDebug() << "Logging in...";
  LoginJob *login = new LoginJob( &session );
  login->setUserName( user );
  login->setPassword( password );
  login->exec();
  Q_ASSERT_X( login->error() == 0, "LoginJob", login->errorString().toLocal8Bit().constData() );
  Q_ASSERT( session.state() == Session::Authenticated );
  qDebug();

  qDebug() << "Asking for capabilities:";
  CapabilitiesJob *capabilities = new CapabilitiesJob( &session );
  capabilities->exec();
  Q_ASSERT_X( capabilities->error() == 0, "CapabilitiesJob", capabilities->errorString().toLocal8Bit().constData() );
  Q_ASSERT( session.state() == Session::Authenticated );
  qDebug() << capabilities->capabilities();
  qDebug();

  Q_ASSERT( capabilities->capabilities().contains( QLatin1String("IDLE") ) );

  qDebug() << "Selecting INBOX:";
  SelectJob *select = new SelectJob( &session );
  select->setMailBox( QLatin1String("INBOX") );
  select->exec();
  Q_ASSERT_X( select->error() == 0, "SelectJob", select->errorString().toLocal8Bit().constData() );
  Q_ASSERT( session.state() == Session::Selected );
  qDebug() << "Flags:" << select->flags();
  qDebug() << "Permanent flags:" << select->permanentFlags();
  qDebug() << "Total Number of Messages:" << select->messageCount();
  qDebug() << "Number of recent Messages:" << select->recentCount();
  qDebug() << "First Unseen Message Index:" << select->firstUnseenIndex();
  qDebug() << "UID validity:" << select->uidValidity();
  qDebug() << "Next UID:" << select->nextUid();
  qDebug();

  qDebug() << "Start idling...";
  IdleJob *idle = new IdleJob( &session );
  QObject::connect( idle, SIGNAL(mailBoxStats(KIMAP::IdleJob*,QString,int,int)),
                    idle, SLOT(stop()) );
  idle->exec();
  qDebug() << "Idling done for" << idle->lastMailBox()
           << "message count:" << idle->lastMessageCount()
           << "recent count:" << idle->lastRecentCount();

  qDebug() << "Closing INBOX:";
  CloseJob *close = new CloseJob( &session );
  close->exec();
  Q_ASSERT( session.state() == Session::Authenticated );
  qDebug();

  qDebug() << "Logging out...";
  LogoutJob *logout = new LogoutJob( &session );
  logout->exec();
  Q_ASSERT_X( logout->error() == 0, "LogoutJob", logout->errorString().toLocal8Bit().constData() );
  Q_ASSERT( session.state() == Session::Disconnected );

  return 0;
}
