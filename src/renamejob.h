/*
    SPDX-FileCopyrightText: 2009 Andras Mantia <amantia@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

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
    ~RenameJob() override;

    /**
     * Set the name of the mailbox that will be renamed.
     * @param mailBox the original name of the mailbox
     */
    void setSourceMailBox(const QString &mailBox);
    Q_REQUIRED_RESULT QString sourceMailBox() const;

    /**
     * The new name of the mailbox, see setMailBox.
     * @param mailBox the new mailbox name
     */
    void setDestinationMailBox(const QString &mailBox);
    Q_REQUIRED_RESULT QString destinationMailBox() const;

protected:
    void doStart() override;
};

}

