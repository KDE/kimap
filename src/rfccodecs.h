/**********************************************************************
 *
 *   rfccodecs  - handler for various rfc/mime encodings
 *   SPDX-FileCopyrightText: 2000 s .carstens@gmx.de
 *
 *   SPDX-License-Identifier: LGPL-2.0-or-later
 *
 *********************************************************************/
/**
 * @file
 * This file is part of the IMAP support library and defines the
 * RfcCodecs class.
 *
 * @brief
 * Provides handlers for various RFC/MIME encodings.
 *
 * @author Sven Carstens
 */

#pragma once

#include <QString>

#include "kimap_export.h"

class QTextCodec;

namespace KIMAP
{
/**
  Converts an Unicode IMAP mailbox to a QByteArray which can be used in
  IMAP communication.
  @param src is the QByteArray containing the IMAP mailbox.
  @since 4.3
*/
Q_REQUIRED_RESULT KIMAP_EXPORT QByteArray encodeImapFolderName(const QByteArray &src);

/**
  Converts an UTF-7 encoded IMAP mailbox to a QByteArray
  @param inSrc is the QByteArray containing the Unicode path.
  @since 4.3
*/
Q_REQUIRED_RESULT KIMAP_EXPORT QByteArray decodeImapFolderName(const QByteArray &inSrc);
/**
  Converts an Unicode IMAP mailbox to a QString which can be used in
  IMAP communication.
  @param src is the QString containing the IMAP mailbox.
*/
Q_REQUIRED_RESULT KIMAP_EXPORT QString encodeImapFolderName(const QString &src);

/**
  Converts an UTF-7 encoded IMAP mailbox to a Unicode QString.
  @param inSrc is the QString containing the Unicode path.
*/
Q_REQUIRED_RESULT KIMAP_EXPORT QString decodeImapFolderName(const QString &inSrc);

/**
  Replaces " with \" and \ with \\ " and \ characters.
  @param src is the QString to quote.
*/
Q_REQUIRED_RESULT KIMAP_EXPORT QString quoteIMAP(const QString &src);

/**
  Replaces " with \" and \ with \\ " and \ characters.
  @param src is the QString to quote.
  @since 4.3
*/
Q_REQUIRED_RESULT KIMAP_EXPORT QByteArray quoteIMAP(const QByteArray &src);

/**
  Fetches a Codec by @p name.
  @param name is the QString version of the Codec name.
  @return Text Codec object
*/
KIMAP_EXPORT QTextCodec *codecForName(const QString &name);

/**
  Decodes a RFC2047 string @p str.
  @param str is the QString to decode.
  @param charset is the character set to use when decoding.
  @param language is the language found in the charset.
*/
Q_REQUIRED_RESULT KIMAP_EXPORT const QString decodeRFC2047String(const QString &str, QString &charset, QString &language);
/**
  Decodes a RFC2047 string @p str.
  @param str is the QString to decode.
  @param charset is the character set to use when decoding.
*/
Q_REQUIRED_RESULT KIMAP_EXPORT const QString decodeRFC2047String(const QString &str, QString &charset);

/**
  Decodes a RFC2047 string @p str.
  @param str is the QString to decode.
*/
Q_REQUIRED_RESULT KIMAP_EXPORT const QString decodeRFC2047String(const QString &str);

/**
  Encodes a RFC2047 string @p str.
  @param str is the QString to encode.
*/
Q_REQUIRED_RESULT KIMAP_EXPORT const QString encodeRFC2047String(const QString &str);

/**
  Encodes a RFC2047 string @p str.
  @param str is the QString to encode.
*/
Q_REQUIRED_RESULT KIMAP_EXPORT const QByteArray encodeRFC2047String(const QByteArray &str);

/**
  Encodes a RFC2231 string @p str.
  @param str is the QString to encode.
*/
Q_REQUIRED_RESULT KIMAP_EXPORT const QString encodeRFC2231String(const QString &str);

/**
  Decodes a RFC2231 string @p str.
  @param str is the QString to decode.
*/
Q_REQUIRED_RESULT KIMAP_EXPORT const QString decodeRFC2231String(const QString &str);
}

