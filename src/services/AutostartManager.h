#pragma once

#include <QFuture>
#include <QObject>

namespace ModeFlow::Services {

class AutostartManager : public QObject {
    Q_OBJECT
public:
    explicit AutostartManager(QObject* parent = nullptr);

    static bool isAdmin();

    QFuture<bool> checkIsRegisteredAsync();
    QFuture<bool> toggleAsync(bool checked, int delaySeconds);

    bool isAutostartEnabled() const;
    static bool shouldEnableStartupLogging(Qt::KeyboardModifiers modifiers);
};
} // namespace ModeFlow::Services
