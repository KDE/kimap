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

#ifndef KIMAP_MESSAGE_P_H
#define KIMAP_MESSAGE_P_H

#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QMetaType>

namespace KIMAP
{

struct Message {
    class Part
    {
    public:
        enum Type { String = 0, List };

        explicit Part(const QByteArray &string)
            : m_type(String), m_string(string) { }
        explicit Part(const QList<QByteArray> &list)
            : m_type(List), m_list(list) { }

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

        foreach (const Part &part, content) {
            if (part.type() == Part::List) {
                result += '(';
                foreach (const QByteArray &item, part.toList()) {
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
            foreach (const Part &part, responseCode) {
                if (part.type() == Part::List) {
                    result += '(';
                    foreach (const QByteArray &item, part.toList()) {
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

Q_DECLARE_METATYPE(KIMAP::Message)
static const int _kimap_messageTypeId = qRegisterMetaType<KIMAP::Message>();

#endif
