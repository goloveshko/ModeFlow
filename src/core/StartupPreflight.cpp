#include "StartupPreflight.h"

#include "Constants.h"
#include "Logging.h"
#include "SystemUtils.h"

namespace ModeFlow::Core {

StartupPreflight::StartupPreflight(QObject* parent) : QObject(parent) {}

void StartupPreflight::startChecking() {
    m_attempts = 0;
    m_pollTimer = new QTimer(this);

    connect(m_pollTimer, &QTimer::timeout, this, &StartupPreflight::onPollTimeout, Qt::QueuedConnection);
    m_pollTimer->start(Utils::SystemReadyPollIntervalMs);
}

void StartupPreflight::onPollTimeout() {
    m_attempts++;

    const bool systemReady = Utils::SystemUtils::isSystemReady() && Utils::SystemUtils::isDesktopActive();

    if (systemReady || m_attempts >= Utils::SystemReadyMaxAttempts) {
        if (systemReady) {
            qCDebug(lcCore) << "System preflight passed after" << (m_attempts * Utils::SystemReadyPollIntervalMs)
                            << "ms";
        } else {
            qCWarning(lcCore) << "Preflight timeout reached. Proceeding with caution...";
        }

        m_pollTimer->stop();
        m_pollTimer->deleteLater();
        m_pollTimer = nullptr;

        emit finished();
    }
}

} // namespace ModeFlow::Core