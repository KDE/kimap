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

#ifndef KIMAP_SETQUOTAJOB_H
#define KIMAP_SETQUOTAJOB_H

#include "quotajobbase.h"

namespace KIMAP {

class Session;
struct Message;
class SetQuotaJobPrivate;

class KIMAP_EXPORT SetQuotaJob : public QuotaJobBase
{
  Q_OBJECT
  Q_DECLARE_PRIVATE(SetQuotaJob)

  friend class SessionPrivate;

  public:
    explicit SetQuotaJob( Session *session );
    virtual ~SetQuotaJob();

    /**
     * Set a limit for a quota resource
     * @param resource quota resouce name
     * @param limit limit for the resouce
     */
    // use qint64 for all the quota stuff
    void setQuota(const QByteArray& resource, qint64 limit);

    void setRoot(const QByteArray &root);
    QByteArray root() const;

  protected:
    virtual void doStart();
    virtual void handleResponse(const Message &response);

};

}

#endif
