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
[[nodiscard]] KIMAP_EXPORT QByteArray encodeImapFolderName(const QByteArray &src);

/*!
  Converts an UTF-7 encoded IMAP mailbox to a QByteArray

  \a inSrc is the QByteArray containing the Unicode path.

  \since 4.3
*/
[[nodiscard]] KIMAP_EXPORT QByteArray decodeImapFolderName(const QByteArray &inSrc);
/*!
  Converts an Unicode IMAP mailbox to a QString which can be used in
  IMAP communication.

  \a src is the QString containing the IMAP mailbox.
*/
[[nodiscard]] KIMAP_EXPORT QString encodeImapFolderName(const QString &src);

/*!
  Converts an UTF-7 encoded IMAP mailbox to a Unicode QString.

  \a inSrc is the QString containing the Unicode path.
*/
[[nodiscard]] KIMAP_EXPORT QString decodeImapFolderName(const QString &inSrc);

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
