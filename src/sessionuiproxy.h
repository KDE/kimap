/*
    SPDX-FileCopyrightText: 2009 Andras Mantia <amantia@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "kimap_export.h"

#include "job.h"

#include <QSharedPointer>

class KSslErrorUiData;

namespace KIMAP
{
/*!
 * \class KIMAP::SessionUiProxy
 * \inmodule KIMAP
 * \inheaderfile KIMAP/SessionUiProxy
 *
 * \brief Interface to display communication errors and wait for user feedback.
 */
class KIMAP_EXPORT SessionUiProxy
{
public:
    /*!
     * \typealias KIMAP::SessionUiProxy::Ptr
     */
    using Ptr = QSharedPointer<SessionUiProxy>;

    virtual ~SessionUiProxy();
    /*!
     * Show an SSL error and ask the user whether it should be ignored or not.
     * The recommended KDE UI is the following:
     * \code
     * #include <kio/ksslui.h>
     * class UiProxy: public SessionUiProxy {
     *   public:
     *     bool ignoreSslError(const KSslErrorUiData& errorData) {
     *       if (KIO::SslUi::askIgnoreSslErrors(errorData)) {
     *         return true;
     *       } else {
     *        return false;
     *       }
     *     }
     * };
     * [...]
     * Session session(server, port);
     * UiProxy *proxy = new UiProxy();
     * session.setUiProxy(proxy);
     * \endcode
     *
     * \a errorData contains details about the error.
     *
     * Returns \c true if the error can be ignored
     */
    virtual bool ignoreSslError(const KSslErrorUiData &errorData) = 0;
};

}
