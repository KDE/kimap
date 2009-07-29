/*
    Copyright (c) 2009 Kevin Ottens <ervin@kde.org>

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

#ifndef KIMAP_IDLEJOB_H
#define KIMAP_IDLEJOB_H

#include "kimap_export.h"

#include "imapset.h"
#include "job.h"

#include "kmime/kmime_content.h"
#include "kmime/kmime_message.h"

#include <boost/shared_ptr.hpp>

namespace KIMAP {

class Session;
struct Message;
class IdleJobPrivate;

class KIMAP_EXPORT IdleJob : public Job
{
  Q_OBJECT
  Q_DECLARE_PRIVATE(IdleJob)

  public:
    explicit IdleJob( Session *session );
    virtual ~IdleJob();

    QString lastMailBox() const;
    int lastMessageCount() const;
    int lastRecentCount() const;

  public Q_SLOTS:
    void stop();

  Q_SIGNALS:
    void mailBoxStats(KIMAP::IdleJob *job, const QString &mailBox, int messageCount, int recentCount);

  protected:
    virtual void doStart();
    virtual void handleResponse(const Message &response);
};

}

#endif
