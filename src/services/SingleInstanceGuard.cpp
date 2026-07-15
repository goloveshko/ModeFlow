#include "SingleInstanceGuard.h"

#include <QCryptographicHash>
#include <QDir>
#include <QLocalSocket>

#include "Constants.h"
#include "Logging.h"
#include "VersionInfo.h"

namespace ModeFlow::Services {

using namespace Qt::StringLiterals;

SingleInstanceGuard::SingleInstanceGuard(QObject* parent) : QObject(parent), m_server(nullptr) {}

SingleInstanceGuard::~SingleInstanceGuard() {
    if (m_server) {
        m_server->close();
        m_server->deleteLater();
    }
}

QString SingleInstanceGuard::generateUniqueServerName() {
    const QString appIdentifier = u"%1::%2"_s.arg(APP_COMPANY_NAME, APP_PRODUCT_NAME);
    QString serverName = QCryptographicHash::hash(appIdentifier.toUtf8(), QCryptographicHash::Sha256).toHex();
    serverName.prepend("ModeFlow_");
    return serverName;
}

QString SingleInstanceGuard::generateSharedSecret() {
    const QString salt = u"ModeFlow_IPC_Secret_2026_Salt_q89wery"_s;
    const QString userName = qEnvironmentVariable("USERNAME");
    return QCryptographicHash::hash((userName + salt).toUtf8(), QCryptographicHash::Sha256).toHex();
}

bool SingleInstanceGuard::tryToRun() {
    const QString serverName = generateUniqueServerName();

    QLocalSocket socket;
    socket.connectToServer(serverName);

    if (socket.waitForConnected(Utils::LocalSocketTimeoutMs)) {
        const QByteArray payload = "ACTIVATE:" + generateSharedSecret().toUtf8();
        socket.write(payload);
        socket.waitForBytesWritten(Utils::LocalSocketTimeoutMs);

        return false;
    }

    m_server = new QLocalServer(this);

    if (!m_server->listen(serverName)) {
        if (m_server->serverError() == QAbstractSocket::AddressInUseError) {
            QLocalServer::removeServer(serverName);
            if (!m_server->listen(serverName)) {
                qCCritical(lcService) << "Critical error: failed to listen after clearing dead socket:"
                                      << m_server->errorString();
                delete m_server;
                m_server = nullptr;
                return false;
            }
        } else {
            qCCritical(lcService) << "Local server listen failed:" << m_server->errorString();
            delete m_server;
            m_server = nullptr;
            return false;
        }
    }

    setupConnections();

    return true;
}

void SingleInstanceGuard::setupConnections() {
    connect(m_server, &QLocalServer::newConnection, this, [this]() {
        QLocalSocket* clientSocket = m_server->nextPendingConnection();
        if (clientSocket->waitForReadyRead(Utils::LocalSocketTimeoutMs)) {
            const QByteArray expectedPayload = "ACTIVATE:" + generateSharedSecret().toUtf8();
            if (clientSocket->readAll() == expectedPayload) {
                emit signalRaiseWindow();
            } else {
                qCWarning(lcService) << "Blocked unauthorized local socket connection!";
            }
        }
        clientSocket->close();
        clientSocket->deleteLater();
    });
}

void SingleInstanceGuard::shutdown() {
    if (m_server) {
        m_server->close();
        qCDebug(lcService) << "Server closed for restart.";
    }
}

} // namespace ModeFlow::Services