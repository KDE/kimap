/*
    SPDX-FileCopyrightText: 2009 Kevin Ottens <ervin@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <QByteArray>
#include <QList>
#include <QMetaType>

namespace KIMAP
{
struct Response {
    class Part
    {
    public:
        enum Type { String = 0, List };

        explicit Part(const QByteArray &string)
            : m_type(String)
            , m_string(string)
        {
        }
        explicit Part(const QList<QByteArray> &list)
            : m_type(List)
            , m_list(list)
        {
        }

        inline Type type() const
        {
            return m_type;
        }
        inline QByteArray toString() const
        {
            return m_string;
        }
        inline QList<QByteArray> toList() const
        {
            return m_list;
        }

    private:
        Type m_type;
        QByteArray m_string;
        QList<QByteArray> m_list;
    };

    inline QByteArray toString() const
    {
        QByteArray result;

        for (const Part &part : std::as_const(content)) {
            if (part.type() == Part::List) {
                result += '(';
                const QList<QByteArray> lstBa = part.toList();
                for (const QByteArray &item : lstBa) {
                    result += ' ';
                    result += item;
                }
                result += " ) ";
            } else {
                result += part.toString() + ' ';
            }
        }

        if (!responseCode.isEmpty()) {
            result += "[ ";
            for (const Part &part : std::as_const(responseCode)) {
                if (part.type() == Part::List) {
                    result += '(';
                    const QList<QByteArray> lstBa = part.toList();
                    for (const QByteArray &item : lstBa) {
                        result += ' ';
                        result += item;
                    }
                    result += " ) ";
                } else {
                    result += part.toString() + ' ';
                }
            }
            result += " ]";
        }

        return result;
    }

    QList<Part> content;
    QList<Part> responseCode;
};

}

Q_DECLARE_METATYPE(KIMAP::Response)
static const int _kimap_messageTypeId = qRegisterMetaType<KIMAP::Response>();

