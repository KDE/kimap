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

#ifndef KIMAP_LISTJOB_H
#define KIMAP_LISTJOB_H

#include "kimap_export.h"

#include "job.h"

namespace KIMAP {

class Session;
struct Message;
class ListJobPrivate;

struct KIMAP_EXPORT MailBoxDescriptor
{
  QString name;
  QChar separator;

  inline bool operator==(const MailBoxDescriptor &other) const
  {
    return other.name==name && other.separator==separator;
  }

  inline bool operator<(const MailBoxDescriptor &other) const
  {
    return other.name<name || (other.name==name && other.separator<separator);
  }
};

class KIMAP_EXPORT ListJob : public Job
{
  Q_OBJECT
  Q_DECLARE_PRIVATE(ListJob)

  friend class SessionPrivate;

  public:
    explicit ListJob( Session *session );
    virtual ~ListJob();

    void setIncludeUnsubscribed( bool include );
    bool isIncludeUnsubscribed() const;

    void setQueriedNamespaces( const QList<MailBoxDescriptor> &namespaces );
    QList<MailBoxDescriptor> queriedNamespaces() const;

    QList<MailBoxDescriptor> mailBoxes() const;
    QMap< MailBoxDescriptor, QList<QByteArray> > flags() const;

  Q_SIGNALS:
    void mailBoxesReceived( const QList<KIMAP::MailBoxDescriptor> &descriptors,
                            const QList< QList<QByteArray> > &flags );

  protected:
    virtual void doStart();
    virtual void handleResponse(const Message &response);

  private:
    Q_PRIVATE_SLOT( d_func(), void emitPendings() )
};

}

#endif
