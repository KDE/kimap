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

#ifndef KIMAP_METADATAJOBBASE_H
#define KIMAP_METADATAJOBBASE_H

#include "kimap_export.h"

#include "job.h"

namespace KIMAP {

class Session;
struct Message;
class MetaDataJobBasePrivate;

/** @short Base class of Metadata/Annotatemore related jobs. It cannot be used directly, you must subclass it and reimplement at least the
doStart() method.
*/
class KIMAP_EXPORT MetaDataJobBase : public Job
{
  Q_OBJECT
  Q_DECLARE_PRIVATE(MetaDataJobBase)

  friend class SessionPrivate;

  public:
    explicit MetaDataJobBase( Session *session );
    virtual ~MetaDataJobBase();

    enum ServerCapability {
      Metadata = 0, //rfc5464
      Annotatemore //compatibility with draft-daboo-imap-annotatemore-07
    };

    void setMailBox( const QString &mailBox );
    QString mailBox() const;

    /**
     * Set what kind of annotation does the server support. The commands send out depend on the mode set here.
     * @param capability Metadata (RFC5464 mode) or Annotatemore (draft-daboo-imap-annotatemore-07 mode)
     */
    void setServerCapability(const ServerCapability& capability);

    /**
     * Check the operating mode.
     * @return the annotation capability of the server, see ServerCapability
     */
    ServerCapability serverCapability() const;

  protected:
    MetaDataJobBase( JobPrivate &dd );

};

}

#endif
