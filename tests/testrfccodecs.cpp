/* This file is part of the KDE project
   Copyright (C) 2004 David Faure <faure@kde.org>
   Copyright (C) 2007 Tom Albers <tomalbers@kde.nl>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "kimap/rfccodecs.h"

#include <kcmdlineargs.h>
#include <kdebug.h>

#include <QCoreApplication>

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

using namespace KIMAP;

static bool check(const QString& txt, const QString& a, const QString& b)
{
  if (a == b) {
    kDebug() << txt << " : checking '" << a << "' against expected value '" << b << "'... " << "ok" << endl;
  }
  else {
    kDebug() << txt << " : checking '" << a << "' against expected value '" << b << "'... " << "KO !" << endl;
    exit(1);
  }
  return true;
}

static bool checkIMAPEncoding( const QString& decodedIn, const QString& encodedIn )
{
  QString encoded = RfcCodecs::encodeImapFolderName( decodedIn );
  check( "encode test: " + decodedIn + " encoded ", encoded, encodedIn );
  
  QString decoded = RfcCodecs::decodeImapFolderName( encodedIn );
  check( "decode test: " + encodedIn + " decoded ", decoded, decodedIn );
  return true;
}


int main(int argc, char *argv[])
{
  QCoreApplication app( argc, argv );

  // Check encoding:
  checkIMAPEncoding("Test.Frode Rønning", "Test.Frode R&APg-nning");
  checkIMAPEncoding("Test.tom & jerry", "Test.tom &- jerry");

  printf("\nTest OK !\n");

  return 0;
}

