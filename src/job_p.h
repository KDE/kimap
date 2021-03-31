/*
    SPDX-FileCopyrightText: 2009 Kevin Ottens <ervin@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include "session.h"
#include <QAbstractSocket>

namespace KIMAP
{
class SessionPrivate;

class JobPrivate
{
public:
    JobPrivate(Session *session, const QString &name)
        : m_session(session)
    {
        m_name = name;
    }
    virtual ~JobPrivate()
    {
    }

    inline SessionPrivate *sessionInternal()
    {
        return m_session->d;
    }

    inline const SessionPrivate *sessionInternal() const
    {
        return m_session->d;
    }

    void setSocketError(QAbstractSocket::SocketError error)
    {
        m_socketError = error;
    }

    QList<QByteArray> tags;
    Session *m_session = nullptr;
    QString m_name;
    QAbstractSocket::SocketError m_socketError = QAbstractSocket::UnknownSocketError;
};

}

