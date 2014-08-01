/*
    Copyright (c) 2009 Andras Mantia <amantia@kde.org>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

#ifndef KIMAP_SESSIONUIPROXY_H
#define KIMAP_SESSIONUIPROXY_H

#include <boost/shared_ptr.hpp>

#include "kimap_export.h"

#include "job.h"

class KSslErrorUiData;

namespace KIMAP
{

/** @short Interface to display communication errors and wait for user feedback. */
class KIMAP_EXPORT SessionUiProxy
{
public:
    typedef boost::shared_ptr<SessionUiProxy> Ptr;

    virtual ~SessionUiProxy() {};
    /**
     * Show an SSL error and ask the user whether it should be ignored or not.
     * The recommended KDE UI is the following:
     * @code
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
     * @endcode
     * @param errorData contains details about the error.
     * @return true if the error can be ignored
     */
    virtual bool ignoreSslError(const KSslErrorUiData &errorData) = 0;
};

}

#endif
