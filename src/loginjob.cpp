/*
    SPDX-FileCopyrightText: 2009 Kevin Ottens <ervin@kde.org>
    SPDX-FileCopyrightText: 2009 Andras Mantia <amantia@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "loginjob.h"

#include <KLocalizedString>

#include "kimap_debug.h"

#include "capabilitiesjob.h"
#include "job_p.h"
#include "response_p.h"
#include "rfccodecs.h"
#include "session_p.h"

#include "common.h"

extern "C" {
#include <sasl/sasl.h>
}

static const sasl_callback_t callbacks[] = {{SASL_CB_ECHOPROMPT, nullptr, nullptr},
                                            {SASL_CB_NOECHOPROMPT, nullptr, nullptr},
                                            {SASL_CB_GETREALM, nullptr, nullptr},
                                            {SASL_CB_USER, nullptr, nullptr},
                                            {SASL_CB_AUTHNAME, nullptr, nullptr},
                                            {SASL_CB_PASS, nullptr, nullptr},
                                            {SASL_CB_CANON_USER, nullptr, nullptr},
                                            {SASL_CB_LIST_END, nullptr, nullptr}};

namespace KIMAP
{
class LoginJobPrivate : public JobPrivate
{
public:
    enum AuthState { PreStartTlsCapability = 0, StartTls, Capability, Login, Authenticate };

    LoginJobPrivate(LoginJob *job, Session *session, const QString &name)
        : JobPrivate(session, name)
        , q(job)
        , encryptionMode(LoginJob::Unencrypted)
        , authState(Login)
        , plainLoginDisabled(false)
    {
        conn = nullptr;
        client_interact = nullptr;
    }
    ~LoginJobPrivate()
    {
    }
    bool sasl_interact();

    bool startAuthentication();
    bool answerChallenge(const QByteArray &data);
    void sslResponse(bool response);
    void saveServerGreeting(const Response &response);

    LoginJob *const q;

    QString userName;
    QString authorizationName;
    QString password;
    QString serverGreeting;

    LoginJob::EncryptionMode encryptionMode;
    QString authMode;
    AuthState authState;
    QStringList capabilities;
    bool plainLoginDisabled;

    sasl_conn_t *conn;
    sasl_interact_t *client_interact;
};
}

using namespace KIMAP;

bool LoginJobPrivate::sasl_interact()
{
    qCDebug(KIMAP_LOG) << "sasl_interact";
    sasl_interact_t *interact = client_interact;

    // some mechanisms do not require username && pass, so it doesn't need a popup
    // window for getting this info
    for (; interact->id != SASL_CB_LIST_END; interact++) {
        if (interact->id == SASL_CB_AUTHNAME || interact->id == SASL_CB_PASS) {
            // TODO: dialog for use name??
            break;
        }
    }

    interact = client_interact;
    while (interact->id != SASL_CB_LIST_END) {
        qCDebug(KIMAP_LOG) << "SASL_INTERACT id:" << interact->id;
        switch (interact->id) {
        case SASL_CB_AUTHNAME:
            if (!authorizationName.isEmpty()) {
                qCDebug(KIMAP_LOG) << "SASL_CB_[AUTHNAME]: '" << authorizationName << "'";
                interact->result = strdup(authorizationName.toUtf8().constData());
                interact->len = strlen((const char *)interact->result);
                break;
            }
            Q_FALLTHROUGH();
        case SASL_CB_USER:
            qCDebug(KIMAP_LOG) << "SASL_CB_[USER|AUTHNAME]: '" << userName << "'";
            interact->result = strdup(userName.toUtf8().constData());
            interact->len = strlen((const char *)interact->result);
            break;
        case SASL_CB_PASS:
            qCDebug(KIMAP_LOG) << "SASL_CB_PASS: [hidden]";
            interact->result = strdup(password.toUtf8().constData());
            interact->len = strlen((const char *)interact->result);
            break;
        default:
            interact->result = nullptr;
            interact->len = 0;
            break;
        }
        interact++;
    }
    return true;
}

LoginJob::LoginJob(Session *session)
    : Job(*new LoginJobPrivate(this, session, i18n("Login")))
{
    Q_D(LoginJob);
    connect(d->sessionInternal(), SIGNAL(encryptionNegotiationResult(bool)), this, SLOT(sslResponse(bool)));
    qCDebug(KIMAP_LOG) << this;
}

LoginJob::~LoginJob()
{
    qCDebug(KIMAP_LOG) << this;
}

QString LoginJob::userName() const
{
    Q_D(const LoginJob);
    return d->userName;
}

void LoginJob::setUserName(const QString &userName)
{
    Q_D(LoginJob);
    d->userName = userName;
}

QString LoginJob::authorizationName() const
{
    Q_D(const LoginJob);
    return d->authorizationName;
}

void LoginJob::setAuthorizationName(const QString &authorizationName)
{
    Q_D(LoginJob);
    d->authorizationName = authorizationName;
}

QString LoginJob::password() const
{
    Q_D(const LoginJob);
    return d->password;
}

void LoginJob::setPassword(const QString &password)
{
    Q_D(LoginJob);
    d->password = password;
}

void LoginJob::doStart()
{
    Q_D(LoginJob);

    qCDebug(KIMAP_LOG) << this;
    // Don't authenticate on a session in the authenticated state
    if (session()->state() == Session::Authenticated || session()->state() == Session::Selected) {
        setError(UserDefinedError);
        setErrorText(i18n("IMAP session in the wrong state for authentication"));
        emitResult();
        return;
    }

    // Trigger encryption negotiation only if needed
    EncryptionMode encryptionMode = d->encryptionMode;

    const auto negotiatedEncryption = d->sessionInternal()->negotiatedEncryption();
    if (negotiatedEncryption != QSsl::UnknownProtocol) {
        // If the socket is already encrypted, pretend we did not want any
        // encryption
        encryptionMode = Unencrypted;
    }

    if (encryptionMode == SSLorTLS) {
        d->sessionInternal()->startSsl(QSsl::SecureProtocols);
    } else if (encryptionMode == STARTTLS) {
        // Check if STARTTLS is supported
        d->authState = LoginJobPrivate::PreStartTlsCapability;
        d->tags << d->sessionInternal()->sendCommand("CAPABILITY");
    } else {
        if (d->authMode.isEmpty()) {
            d->authState = LoginJobPrivate::Login;
            qCDebug(KIMAP_LOG) << "sending LOGIN";
            d->tags << d->sessionInternal()->sendCommand("LOGIN",
                                                         '"' + quoteIMAP(d->userName).toUtf8() + '"' + ' ' + '"' + quoteIMAP(d->password).toUtf8() + '"');
        } else {
            if (!d->startAuthentication()) {
                emitResult();
            }
        }
    }
}

void LoginJob::handleResponse(const Response &response)
{
    Q_D(LoginJob);

    if (response.content.isEmpty()) {
        return;
    }

    // set the actual command name for standard responses
    QString commandName = i18n("Login");
    if (d->authState == LoginJobPrivate::Capability) {
        commandName = i18n("Capability");
    } else if (d->authState == LoginJobPrivate::StartTls) {
        commandName = i18n("StartTls");
    }

    enum ResponseCode { OK, ERR, UNTAGGED, CONTINUATION, MALFORMED };

    QByteArray tag = response.content.first().toString();
    ResponseCode code = OK;

    qCDebug(KIMAP_LOG) << commandName << tag;

    if (tag == "+") {
        code = CONTINUATION;
    } else if (tag == "*") {
        if (response.content.size() < 2) {
            code = MALFORMED; // Received empty untagged response
        } else {
            code = UNTAGGED;
        }
    } else if (d->tags.contains(tag)) {
        if (response.content.size() < 2) {
            code = MALFORMED;
        } else if (response.content[1].toString() == "OK") {
            code = OK;
        } else {
            code = ERR;
        }
    }

    switch (code) {
    case MALFORMED:
        // We'll handle it later
        break;

    case ERR:
        // server replied with NO or BAD for SASL authentication
        if (d->authState == LoginJobPrivate::Authenticate) {
            sasl_dispose(&d->conn);
        }

        setError(UserDefinedError);
        setErrorText(i18n("%1 failed, server replied: %2", commandName, QLatin1String(response.toString().constData())));
        emitResult();
        return;

    case UNTAGGED:
        // The only untagged response interesting for us here is CAPABILITY
        if (response.content[1].toString() == "CAPABILITY") {
            d->capabilities.clear();
            QList<Response::Part>::const_iterator p = response.content.begin() + 2;
            while (p != response.content.end()) {
                QString capability = QLatin1String(p->toString());
                d->capabilities << capability;
                if (capability == QLatin1String("LOGINDISABLED")) {
                    d->plainLoginDisabled = true;
                }
                ++p;
            }
            qCDebug(KIMAP_LOG) << "Capabilities updated: " << d->capabilities;
        }
        break;

    case CONTINUATION:
        if (d->authState != LoginJobPrivate::Authenticate) {
            // Received unexpected continuation response for something
            // other than AUTHENTICATE command
            code = MALFORMED;
            break;
        }

        if (d->authMode == QLatin1String("PLAIN")) {
            if (response.content.size() > 1 && response.content.at(1).toString() == "OK") {
                return;
            }
            QByteArray challengeResponse;
            if (!d->authorizationName.isEmpty()) {
                challengeResponse += d->authorizationName.toUtf8();
            }
            challengeResponse += '\0';
            challengeResponse += d->userName.toUtf8();
            challengeResponse += '\0';
            challengeResponse += d->password.toUtf8();
            challengeResponse = challengeResponse.toBase64();
            d->sessionInternal()->sendData(challengeResponse);
        } else if (response.content.size() >= 2) {
            if (!d->answerChallenge(QByteArray::fromBase64(response.content[1].toString()))) {
                emitResult(); // error, we're done
            }
        } else {
            // Received empty continuation for authMode other than PLAIN
            code = MALFORMED;
        }
        break;

    case OK:

        switch (d->authState) {
        case LoginJobPrivate::PreStartTlsCapability:
            if (d->capabilities.contains(QLatin1String("STARTTLS"))) {
                d->authState = LoginJobPrivate::StartTls;
                d->tags << d->sessionInternal()->sendCommand("STARTTLS");
            } else {
                qCWarning(KIMAP_LOG) << "STARTTLS not supported by server!";
                setError(UserDefinedError);
                setErrorText(i18n("STARTTLS is not supported by the server, try using SSL/TLS instead."));
                emitResult();
            }
            break;

        case LoginJobPrivate::StartTls:
            d->sessionInternal()->startSsl(QSsl::SecureProtocols);
            break;

        case LoginJobPrivate::Capability:
            // cleartext login, if enabled
            if (d->authMode.isEmpty()) {
                if (d->plainLoginDisabled) {
                    setError(UserDefinedError);
                    setErrorText(i18n("Login failed, plain login is disabled by the server."));
                    emitResult();
                } else {
                    d->authState = LoginJobPrivate::Login;
                    d->tags << d->sessionInternal()->sendCommand("LOGIN",
                                                                 '"' + quoteIMAP(d->userName).toUtf8() + '"' + ' ' + '"' + quoteIMAP(d->password).toUtf8()
                                                                     + '"');
                }
            } else {
                bool authModeSupported = false;
                // find the selected SASL authentication method
                for (const QString &capability : std::as_const(d->capabilities)) {
                    if (capability.startsWith(QLatin1String("AUTH="))) {
                        if (QStringView(capability).mid(5) == d->authMode) {
                            authModeSupported = true;
                            break;
                        }
                    }
                }
                if (!authModeSupported) {
                    setError(UserDefinedError);
                    setErrorText(i18n("Login failed, authentication mode %1 is not supported by the server.", d->authMode));
                    emitResult();
                } else if (!d->startAuthentication()) {
                    emitResult(); // problem, we're done
                }
            }
            break;

        case LoginJobPrivate::Authenticate:
            sasl_dispose(&d->conn); // SASL authentication done
            // Fall through
            Q_FALLTHROUGH();
        case LoginJobPrivate::Login:
            d->saveServerGreeting(response);
            emitResult(); // got an OK, command done
            break;
        }
    }

    if (code == MALFORMED) {
        setErrorText(i18n("%1 failed, malformed reply from the server.", commandName));
        emitResult();
    }
}

bool LoginJobPrivate::startAuthentication()
{
    // SASL authentication
    if (!initSASL()) {
        q->setError(LoginJob::UserDefinedError);
        q->setErrorText(i18n("Login failed, client cannot initialize the SASL library."));
        return false;
    }

    authState = LoginJobPrivate::Authenticate;
    const char *out = nullptr;
    uint outlen = 0;
    const char *mechusing = nullptr;

    int result = sasl_client_new("imap", m_session->hostName().toLatin1().constData(), nullptr, nullptr, callbacks, 0, &conn);
    if (result != SASL_OK) {
        const QString saslError = QString::fromUtf8(sasl_errdetail(conn));
        qCWarning(KIMAP_LOG) << "sasl_client_new failed with:" << result << saslError;
        q->setError(LoginJob::UserDefinedError);
        q->setErrorText(saslError);
        return false;
    }

    do {
        qCDebug(KIMAP_LOG) << "Trying authmod" << authMode.toLatin1();
        result = sasl_client_start(conn,
                                   authMode.toLatin1().constData(),
                                   &client_interact,
                                   capabilities.contains(QLatin1String("SASL-IR")) ? &out : nullptr,
                                   &outlen,
                                   &mechusing);

        if (result == SASL_INTERACT) {
            if (!sasl_interact()) {
                sasl_dispose(&conn);
                q->setError(LoginJob::UserDefinedError); // TODO: check up the actual error
                return false;
            }
        }
    } while (result == SASL_INTERACT);

    if (result != SASL_CONTINUE && result != SASL_OK) {
        const QString saslError = QString::fromUtf8(sasl_errdetail(conn));
        qCWarning(KIMAP_LOG) << "sasl_client_start failed with:" << result << saslError;
        q->setError(LoginJob::UserDefinedError);
        q->setErrorText(saslError);
        sasl_dispose(&conn);
        return false;
    }

    QByteArray tmp = QByteArray::fromRawData(out, outlen);
    QByteArray challenge = tmp.toBase64();

    if (challenge.isEmpty()) {
        tags << sessionInternal()->sendCommand("AUTHENTICATE", authMode.toLatin1());
    } else {
        tags << sessionInternal()->sendCommand("AUTHENTICATE", authMode.toLatin1() + ' ' + challenge);
    }

    return true;
}

bool LoginJobPrivate::answerChallenge(const QByteArray &data)
{
    QByteArray challenge = data;
    int result = -1;
    const char *out = nullptr;
    uint outlen = 0;
    do {
        result = sasl_client_step(conn, challenge.isEmpty() ? nullptr : challenge.data(), challenge.size(), &client_interact, &out, &outlen);

        if (result == SASL_INTERACT) {
            if (!sasl_interact()) {
                q->setError(LoginJob::UserDefinedError); // TODO: check up the actual error
                sasl_dispose(&conn);
                return false;
            }
        }
    } while (result == SASL_INTERACT);

    if (result != SASL_CONTINUE && result != SASL_OK) {
        const QString saslError = QString::fromUtf8(sasl_errdetail(conn));
        qCWarning(KIMAP_LOG) << "sasl_client_step failed with:" << result << saslError;
        q->setError(LoginJob::UserDefinedError); // TODO: check up the actual error
        q->setErrorText(saslError);
        sasl_dispose(&conn);
        return false;
    }

    QByteArray tmp = QByteArray::fromRawData(out, outlen);
    challenge = tmp.toBase64();

    sessionInternal()->sendData(challenge);

    return true;
}

void LoginJobPrivate::sslResponse(bool response)
{
    if (response) {
        authState = LoginJobPrivate::Capability;
        tags << sessionInternal()->sendCommand("CAPABILITY");
    } else {
        q->setError(LoginJob::UserDefinedError);
        q->setErrorText(i18n("Login failed, TLS negotiation failed."));
        encryptionMode = LoginJob::Unencrypted;
        q->emitResult();
    }
}

void LoginJob::setEncryptionMode(EncryptionMode mode)
{
    Q_D(LoginJob);
    d->encryptionMode = mode;
}

LoginJob::EncryptionMode LoginJob::encryptionMode()
{
    Q_D(LoginJob);
    return d->encryptionMode;
}

void LoginJob::setAuthenticationMode(AuthenticationMode mode)
{
    Q_D(LoginJob);
    switch (mode) {
    case ClearText:
        d->authMode = QLatin1String("");
        break;
    case Login:
        d->authMode = QStringLiteral("LOGIN");
        break;
    case Plain:
        d->authMode = QStringLiteral("PLAIN");
        break;
    case CramMD5:
        d->authMode = QStringLiteral("CRAM-MD5");
        break;
    case DigestMD5:
        d->authMode = QStringLiteral("DIGEST-MD5");
        break;
    case GSSAPI:
        d->authMode = QStringLiteral("GSSAPI");
        break;
    case Anonymous:
        d->authMode = QStringLiteral("ANONYMOUS");
        break;
    case XOAuth2:
        d->authMode = QStringLiteral("XOAUTH2");
        break;
    default:
        d->authMode = QString();
    }
}

void LoginJob::connectionLost()
{
    Q_D(LoginJob);

    // don't emit the result if the connection was lost before getting the tls result, as it can mean
    // the TLS handshake failed and the socket was reconnected in normal mode
    if (d->authState != LoginJobPrivate::StartTls) {
        qCWarning(KIMAP_LOG) << "Connection to server lost " << d->m_socketError;
        if (d->m_socketError == QAbstractSocket::SslHandshakeFailedError) {
            setError(KJob::UserDefinedError);
            setErrorText(i18n("SSL handshake failed."));
            emitResult();
        } else {
            setError(ERR_COULD_NOT_CONNECT);
            setErrorText(i18n("Connection to server lost."));
            emitResult();
        }
    }
}

void LoginJobPrivate::saveServerGreeting(const Response &response)
{
    // Concatenate the parts of the server response into a string, while dropping the first two parts
    // (the response tag and the "OK" code), and being careful not to add useless extra whitespace.

    for (int i = 2; i < response.content.size(); i++) {
        if (response.content.at(i).type() == Response::Part::List) {
            serverGreeting += QLatin1Char('(');
            const QList<QByteArray> itemLst = response.content.at(i).toList();
            for (const QByteArray &item : itemLst) {
                serverGreeting += QLatin1String(item) + QLatin1Char(' ');
            }
            serverGreeting.chop(1);
            serverGreeting += QStringLiteral(") ");
        } else {
            serverGreeting += QLatin1String(response.content.at(i).toString()) + QLatin1Char(' ');
        }
    }
    serverGreeting.chop(1);
}

QString LoginJob::serverGreeting() const
{
    Q_D(const LoginJob);
    return d->serverGreeting;
}

#include "moc_loginjob.cpp"
