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

#ifndef KIMAP_FETCHJOB_H
#define KIMAP_FETCHJOB_H

#include "kimap_export.h"

#include "imapset.h"
#include "job.h"

#include "kmime/kmime_content.h"
#include "kmime/kmime_message.h"

#include <boost/shared_ptr.hpp>

namespace KIMAP {

class Session;
struct Message;
class FetchJobPrivate;

typedef boost::shared_ptr<KMime::Content> ContentPtr;
typedef QMap<QByteArray, ContentPtr> MessageParts;

typedef boost::shared_ptr<KMime::Message> MessagePtr;
typedef QList<QByteArray> MessageFlags;

class KIMAP_EXPORT FetchJob : public Job
{
  Q_OBJECT
  Q_DECLARE_PRIVATE(FetchJob)

  friend class SessionPrivate;

  public:
    struct FetchScope
    {
      QList<QByteArray> parts; // used only if mode == Headers or Content
      enum {Headers, Flags, Structure, Content, Full} mode;
    };

    explicit FetchJob( Session *session );
    virtual ~FetchJob();

    void setSequenceSet( const ImapSet &set );
    ImapSet sequenceSet() const;

    void setUidBased(bool uidBased);
    bool isUidBased() const;

    void setScope( const FetchScope &scope );
    FetchScope scope() const;

    QMap<qint64, MessagePtr> messages() const;
    QMap<qint64, MessageParts> parts() const;
    QMap<qint64, MessageFlags> flags() const;
    QMap<qint64, qint64> sizes() const;
    QMap<qint64, qint64> uids() const;

  Q_SIGNALS:
    void headersReceived( const QString &mailBox,
                          const QMap<qint64, qint64> &uids,
                          const QMap<qint64, qint64> &sizes,
                          const QMap<qint64, KIMAP::MessageFlags> &flags,
                          const QMap<qint64, KIMAP::MessagePtr> &messages );

    void messagesReceived( const QString &mailBox,
                           const QMap<qint64, qint64> &uids,
                           const QMap<qint64, KIMAP::MessagePtr> &messages );

    void partsReceived( const QString &mailBox,
                        const QMap<qint64, qint64> &uids,
                        const QMap<qint64, KIMAP::MessageParts> &parts );

  protected:
    virtual void doStart();
    virtual void handleResponse(const Message &response);

  private:
    Q_PRIVATE_SLOT( d_func(), void emitPendings() )
};

}

#endif
