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

#ifndef KIMAP_SETMETADATAJOB_H
#define KIMAP_SETMETADATAJOB_H

#include "kimap_export.h"

#include "metadatajobbase.h"

namespace KIMAP {

class Session;
struct Message;
class SetMetaDataJobPrivate;

class KIMAP_EXPORT SetMetaDataJob : public MetaDataJobBase
{
  Q_OBJECT
  Q_DECLARE_PRIVATE(SetMetaDataJob)

  friend class SessionPrivate;

  public:
    explicit SetMetaDataJob( Session *session );
    virtual ~SetMetaDataJob();

    /**
     * Add a metadata to the mailbox. Depending on the supported standard by the server (setServerCapability),
     * the @param name can have a different meaning.
     * @param name  the entry name if serverCapability() returns Metadata (RFC5464 mode), the attribute value name
     * if serverCapability() is Annotatemore (draft-daboo-imap-annotatemore-07 mode).
     * @param value the value of the entry/attribute
     */
    void addMetaData(const QByteArray &name, const QByteArray &value);
    /**
     * Set the entry name for the metada, if the job is operating in Annotatemore mode. In Metadata mode, this setting is
     * ignored.
     * @param entry the metadata entry name
     */
    void setEntry(const QByteArray &entry);

    enum MetaDataError {
      NoError = 0,
      TooMany = 1,
      TooBig = 2,
      NoPrivate = 4
    };

    // Q_DECLARE_WHATEVER_THAT_WAS missing
    Q_DECLARE_FLAGS(MetaDataErrors, MetaDataError)

    /**
     * The metadata errors received from the server.
     * @return OR connected error codes, see MetaDataError
     */
    MetaDataErrors metaDataErrors() const;
    /**
     * The maximum accepted metadata size.
     * @return the accepted metadata size, -1 means the limit is unknown.
     */
    qint64 maxAcceptedSize();

  protected:
    virtual void doStart();
    virtual void handleResponse( const Message &response );

};

}

Q_DECLARE_OPERATORS_FOR_FLAGS( KIMAP::SetMetaDataJob::MetaDataErrors )

#endif
