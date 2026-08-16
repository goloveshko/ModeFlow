#pragma once

#include <QObject>
#include <QTimer>

namespace ModeFlow::Core {

/**
 * @brief Self-contained asynchronous poller that waits for Windows OS services
 * and active user desktop to become fully ready on system logon.
 */
class StartupPreflight : public QObject {
    Q_OBJECT
public:
    explicit StartupPreflight(QObject* parent = nullptr);
    ~StartupPreflight() override = default;

    void startChecking();

signals:
    void finished();

private slots:
    void onPollTimeout();

private:
    QTimer* m_pollTimer = nullptr;
    int m_attempts = 0;
};

} // namespace ModeFlow::Core