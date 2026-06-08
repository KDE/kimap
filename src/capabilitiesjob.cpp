/*
    SPDX-FileCopyrightText: 2009 Kevin Ottens <ervin@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "capabilitiesjob.h"
#include <iostream>

#include <KLocalizedString>

#include "job_p.h"
#include "response_p.h"
#include "session_p.h"

namespace KIMAP
{
class CapabilitiesJobPrivate : public JobPrivate
{
public:
    CapabilitiesJobPrivate(Session *session, const QString &name)
        : JobPrivate(session, name)
    {
    }

    QStringList capabilities;
};
}

using namespace KIMAP;

CapabilitiesJob::CapabilitiesJob(Session *session)
    : Job(*new CapabilitiesJobPrivate(session, i18n("Capabilities")))
{
}

CapabilitiesJob::~CapabilitiesJob() = default;

QStringList CapabilitiesJob::capabilities() const
{
    Q_D(const CapabilitiesJob);
    return d->capabilities;
}

void CapabilitiesJob::doStart()
{
    Q_D(CapabilitiesJob);
    d->tags << d->sessionInternal()->sendCommand("CAPABILITY");
}

void CapabilitiesJob::handleResponse(const Response &response)
{
    Q_D(CapabilitiesJob);
    if (handleErrorReplies(response) == NotHandled) {
        const auto responseSize(response.content.size());
        if (responseSize >= 2 && response.content[1].toString() == "CAPABILITY") {
            bool supportsRev2 = false;
            bool supportsLiteral = false;
            bool supportsCondstore = false;
            bool supportsQresync = false;
            for (int i = 2; i < responseSize; ++i) {
                d->capabilities << QLatin1StringView(response.content[i].toString().toUpper());
                if (d->capabilities[d->capabilities.length() - 1] == QLatin1StringView("IMAP4REV2")) {
                    supportsRev2 = true;
                }
                if (d->capabilities[d->capabilities.length() - 1] == QLatin1StringView("LITERAL+")
                    || d->capabilities[d->capabilities.length() - 1] == QLatin1StringView("LITERAL-")) {
                    supportsLiteral = true;
                }
                if (d->capabilities[d->capabilities.length() - 1] == QLatin1StringView("CONDSTORE")) {
                    supportsCondstore = true;
                }
                if (d->capabilities[d->capabilities.length() - 1] == QLatin1StringView("QRESYNC")) {
                    supportsQresync = true;
                }
            }
            if (supportsRev2) {
                constexpr auto foldedInCapabilities = std::to_array({
                    QLatin1StringView("NAMESPACE"),
                    QLatin1StringView("UNSELECT"),
                    QLatin1StringView("UIDPLUS"),
                    QLatin1StringView("ESEARCH"),
                    QLatin1StringView("SEARCHRES"),
                    QLatin1StringView("ENABLE"),
                    QLatin1StringView("IDLE"),
                    QLatin1StringView("SASL-IR"),
                    QLatin1StringView("LIST-EXTENDED"),
                    QLatin1StringView("LIST-STATUS"),
                    QLatin1StringView("MOVE"),
                });
                for (const auto capability : std::as_const(foldedInCapabilities)) {
                    if (!d->capabilities.contains(capability)) {
                        d->capabilities << capability;
                    }
                }
                if (!supportsLiteral) {
                    d->capabilities << QLatin1StringView("LITERAL-");
                }
            }
            if (supportsQresync && !supportsCondstore) {
                d->capabilities << QLatin1StringView("CONDSTORE");
            }
            Q_EMIT capabilitiesReceived(d->capabilities);
        }
    }
}

#include "moc_capabilitiesjob.cpp"
