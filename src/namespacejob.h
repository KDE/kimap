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

#ifndef KIMAP_NAMESPACEJOB_H
#define KIMAP_NAMESPACEJOB_H

#include "kimap_export.h"

#include "job.h"

namespace KIMAP
{

class Session;
struct Message;
struct MailBoxDescriptor;
class NamespaceJobPrivate;

class KIMAP_EXPORT NamespaceJob : public Job
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(NamespaceJob)

    friend class SessionPrivate;

public:
    NamespaceJob(Session *session);
    virtual ~NamespaceJob();

    QList<MailBoxDescriptor> personalNamespaces() const;
    QList<MailBoxDescriptor> userNamespaces() const;
    QList<MailBoxDescriptor> sharedNamespaces() const;

    bool containsEmptyNamespace() const;

protected:
    virtual void doStart() Q_DECL_OVERRIDE;
    virtual void handleResponse(const Message &response) Q_DECL_OVERRIDE;
};

}

#endif
