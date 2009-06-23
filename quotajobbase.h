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

#ifndef KIMAP_QUOTAJOBBASE_H
#define KIMAP_QUOTAJOBBASE_H

#include "kimap_export.h"

#include "job.h"

namespace KIMAP {

class Session;
struct Message;
class QuotaJobBasePrivate;

/** @short Base class of Quota related jobs. It cannot be used directly, you must subclass it and reimplement at least the
doStart() method.
*/
class KIMAP_EXPORT QuotaJobBase : public Job
{
  Q_OBJECT
  Q_DECLARE_PRIVATE(QuotaJobBase)

  friend class SessionPrivate;

  public:
    explicit QuotaJobBase( Session *session );
    virtual ~QuotaJobBase();

    qint64 usage(const QByteArray& resource);
    qint64 limit(const QByteArray& resource);

  protected:
    QuotaJobBase( JobPrivate &dd );

};

}

#endif
