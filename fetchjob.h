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

#include "job.h"

#include "kmime/kmime_content.h"
#include "kmime/kmime_message.h"

#include <QtCore/QSharedPointer>

namespace KIMAP {

class Session;
class Message;
class FetchJobPrivate;

class KIMAP_EXPORT FetchJob : public Job
{
  Q_OBJECT
  Q_DECLARE_PRIVATE(FetchJob)

  friend class SessionPrivate;

  public:
    struct FetchScope
    {
      QList<QByteArray> parts; // used only if mode == Headers or Content
      enum {Headers, Flags, Structure, Content} mode;
    };


    FetchJob( Session *session );
    virtual ~FetchJob();

    // TODO: Make a proper class (actually there's one in akonadi server)
    void setSequenceSet( const QByteArray &set );
    QByteArray sequenceSet() const;

    void setUidBased(bool uidBased);
    bool isUidBased() const;

    void setScope( const FetchScope &scope );
    FetchScope scope() const;

    QMap<int, QSharedPointer<KMime::Message> > messages() const;
    QMap<int, QMap<QByteArray, QSharedPointer<KMime::Content> > > parts() const;
    QMap<int, QList<QByteArray> > flags() const;
    QMap<int, qint64> sizes() const;

  Q_SIGNALS:
    void headersReceived( const QByteArray &mailBox, int messageNumber, qint64 size, KMime::Message *message );
    void messageReceived( const QByteArray &mailBox, int messageNumber, KMime::Message *message );
    void partReceived( const QByteArray &mailBox, int messageNumber, const QByteArray &partIndex, KMime::Content *part );

  protected:
    virtual void doStart();
    virtual void doHandleResponse(const Message &response);
};

}

#endif
