/**********************************************************************
 *
 *   rfccodecs  - handler for various rfc/mime encodings
 *   SPDX-FileCopyrightText: 2000 s .carstens@gmx.de
 *
 *   SPDX-License-Identifier: LGPL-2.0-or-later
 *
 *********************************************************************/

#pragma once

#include <QString>

#include "kimap_export.h"

/*!
 * \namespace KIMAP
 * \inmodule KIMAP
 * \brief Provides handlers for various RFC/MIME encodings.
 */
namespace KIMAP
{
/*!
  Converts an Unicode IMAP mailbox to a QByteArray which can be used in
  IMAP communication.

  \a src is the QByteArray containing the IMAP mailbox.

  \since 4.3
*/
[[nodiscard]] KIMAP_EXPORT QByteArray encodeImapFolderName(const QByteArray &src, bool utf8Enabled = false);

/*!
  Converts a wire-format IMAP mailbox to a UTF8-encoded QByteArray.

  \a inSrc is the QByteArray containing the Unicode path.

  \a utf8Enabled must reflect whether the active session is using
  UTF-8-encoded mailbox names — true for IMAP4rev2 (RFC 9051) and for
  IMAP4rev1 sessions that have negotiated UTF8=ACCEPT (RFC 9755) —
  pass the value returned by \c{SessionPrivate::isUtf8Enabled()}. When
  true, the input is already UTF-8 and is returned unchanged; otherwise
  it is decoded from modified UTF-7.

  \since 4.3
*/
[[nodiscard]] KIMAP_EXPORT QByteArray decodeImapFolderName(const QByteArray &inSrc, bool utf8Enabled);

/*!
  Converts an Unicode IMAP mailbox to a QString which can be used in
  IMAP communication.

  \a src is the QString containing the IMAP mailbox.
*/
[[nodiscard]] KIMAP_EXPORT QString encodeImapFolderName(const QString &src, bool utf8Enabled = false);

/*!
  Converts a wire-encoded IMAP mailbox to a Unicode QString.

  \a inSrc is the QString containing the Unicode path.
  \a utf8Enabled must reflect whether the active session is using
  UTF-8-encoded mailbox names — true for IMAP4rev2 (RFC 9051) and for
  IMAP4rev1 sessions that have negotiated UTF8=ACCEPT (RFC 9755) —
  pass the value returned by \c{SessionPrivate::isUtf8Enabled()}. When
  true, the input is already UTF-8 and is returned unchanged; otherwise
  it is decoded from modified UTF-7.
*/
[[nodiscard]] KIMAP_EXPORT QString decodeImapFolderName(const QString &inSrc, bool utf8Enabled);

/*!
  Replaces " with \" and \ with \\ " and \ characters.

  \a src is the QString to quote.
*/
[[nodiscard]] KIMAP_EXPORT QString quoteIMAP(QStringView src);

/*!
  Replaces " with \" and \ with \\ " and \ characters.

  \a src is the QString to quote.

  \since 4.3
*/
[[nodiscard]] KIMAP_EXPORT QByteArray quoteIMAP(QByteArrayView src);
}
