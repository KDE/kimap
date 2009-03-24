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

#ifndef KIMAP_JOB_H
#define KIMAP_JOB_H

#include "kimap_export.h"

#include <KDE/KJob>

namespace KIMAP {

class Session;
class SessionPrivate;
class JobPrivate;
class Message;

class KIMAP_EXPORT Job : public KJob
{
  Q_OBJECT
  Q_DECLARE_PRIVATE(Job)

  friend class SessionPrivate;

  public:
    virtual ~Job();

    virtual void start();

  protected:
    virtual void doStart() = 0;
    virtual void doHandleResponse(const Message &response);

    Job( Session *session );
    Job( JobPrivate &dd );

    JobPrivate *const d_ptr;
};

}

#endif
