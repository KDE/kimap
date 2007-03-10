/**********************************************************************
 *
 *   rfccodecs  - handler for various rfc/mime encodings
 *   Copyright (C) 2000 s.carstens@gmx.de
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Library General Public
 *   License as published by the Free Software Foundation; either
 *   version 2 of the License, or (at your option) any later version.
 *
 *   This library is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *   Library General Public License for more details.
 *
 *   You should have received a copy of the GNU Library General Public License
 *   along with this library; see the file COPYING.LIB.  If not, write to
 *   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *   Boston, MA 02110-1301, USA.
 *
 *********************************************************************/
/**
 * @file
 * This file is part of the IMAP support library and defines the
 * RfcCodecs class.
 *
 * @author Sven Carstens
 */

#ifndef KIMAP_RFCCODECS_H
#define KIMAP_RFCCODECS_H

#include <QString>

#include "kimap.h"

class QTextCodec;

namespace KIMAP {

/**
 * @brief
 * Provides handlers for various RFC/MIME encodings.
 */
class KIMAP_EXPORT RfcCodecs
{
  public:

    /**
      Convert an IMAP mailbox to a Unicode path
    */
    static QString fromIMAP( const QString &src );

    /**
      Convert Unicode path to modified UTF-7 IMAP mailbox.
    */
    static QString toIMAP( const QString &inSrc );

    /**
      Replaces " with \" and \ with \\ " and \ characters.
    */
    static QString quoteIMAP( const QString &src );

    /**
      Fetches a codec by @p name.
      @return Text Codec object
    */
    static QTextCodec *codecForName( const QString &name );

    // decoder for RFC2047 and RFC1522

    /**
      Decodes a RFC2047 string @p str.
    */
    static const QString decodeRFC2047String( const QString &str,
                                              QString &charset,
                                              QString &language );
    /**
      Decodes a RFC2047 string @p str.
    */
    static const QString decodeRFC2047String( const QString &str,
                                              QString &charset );

    /**
      Decode a RFC2047 string @p str.
    */
    static const QString decodeRFC2047String( const QString &str );

    // encoder for RFC2047 and RFC1522

    /**
      Encodes a RFC2047 string @p str.
    */
    static const QString encodeRFC2047String( const QString &str,
                                              QString &charset,
                                              QString &language );

    /**
      Encodes a RFC2047 string @p str.
    */
    static const QString encodeRFC2047String( const QString &str,
                                              QString &charset );

    /**
      Encodes a RFC2047 string @p str.
    */
    static const QString encodeRFC2047String( const QString &str );

    /**
      Encodes a RFC2231 string @p str.
    */
    static const QString encodeRFC2231String( const QString &str );

    /**
      Decodes a RFC2231 string @p str.
    */
    static const QString decodeRFC2231String( const QString &str );
};

}

#endif
