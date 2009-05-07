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

#ifndef KIMAP_GETQUOTAROOTJOB_H
#define KIMAP_GETQUOTAROOTJOB_H

#include "quotajobbase.h"

namespace KIMAP {

class Session;
class Message;
class GetQuotaRootJobPrivate;

class KIMAP_EXPORT GetQuotaRootJob : public QuotaJobBase
{
  Q_OBJECT
  Q_DECLARE_PRIVATE(GetQuotaRootJob)

  friend class SessionPrivate;

  public:
    GetQuotaRootJob( Session *session );
    virtual ~GetQuotaRootJob();

    void setMailBox(const QString &mailBox);
    QString mailBox() const;
    QList<QByteArray> roots() const;
    qint32 usage(const QByteArray& root, const QByteArray& resource);
    qint32 limit(const QByteArray& root, const QByteArray& resource);

  protected:
    virtual void doStart();
    virtual void doHandleResponse(const Message &response);

};

}

#endif
