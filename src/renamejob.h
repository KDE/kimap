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

#ifndef KIMAP_RENAMEJOB_H
#define KIMAP_RENAMEJOB_H

#include "kimap_export.h"

#include "job.h"

namespace KIMAP
{

class Session;
class RenameJobPrivate;

class KIMAP_EXPORT RenameJob : public Job
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(RenameJob)

    friend class SessionPrivate;

public:
    explicit RenameJob(Session *session);
    virtual ~RenameJob();

    /**
     * Set the name of the mailbox that will be renamed.
     * @param mailBox the original name of the mailbox
     */
    void setSourceMailBox(const QString &mailBox);
    QString sourceMailBox() const;

    /**
     * The new name of the mailbox, see setMailBox.
     * @param mailBox the new mailbox name
     */
    void setDestinationMailBox(const QString &mailBox);
    QString destinationMailBox() const;

protected:
    virtual void doStart() Q_DECL_OVERRIDE;
};

}

#endif
