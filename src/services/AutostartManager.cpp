#include "AutostartManager.h"

#include <QGuiApplication>
#include <QtConcurrent>

#include "CommandLineBuilder.h"
#include "TaskScheduler.h"

namespace ModeFlow::Services {

AutostartManager::AutostartManager(QObject* parent) : QObject(parent) {}

bool AutostartManager::isAdmin() {
    return Utils::TaskScheduler::isAdmin();
}

QFuture<bool> AutostartManager::checkIsRegisteredAsync() {
    return QtConcurrent::run([]() { return Utils::TaskScheduler::isTaskRegistered(); });
}

QFuture<bool> AutostartManager::toggleAsync(bool checked, int delaySeconds) {
    return QtConcurrent::run([=]() {
        using namespace ModeFlow::Core;

        bool success = false;

        const bool withLogs = shouldEnableStartupLogging(QGuiApplication::keyboardModifiers());

        if (Utils::TaskScheduler::isAdmin()) {
            // --- Admin Mode: Direct Task Manipulation ---

            CommandLineBuilder taskArgsBuilder;

            taskArgsBuilder.withLogon().withLog(withLogs);

            if (checked) {
                success = Utils::TaskScheduler::createTaskAtLogon(taskArgsBuilder.toString(), delaySeconds, false);
            } else {
                success = Utils::TaskScheduler::removeTask();
            }
        } else {
            // --- User Mode: Elevation Required ---

            CommandLineBuilder elevationBuilder;

            if (checked) {
                elevationBuilder.withRegister().withLogon().withDelay(delaySeconds).withLog(withLogs);
            } else {
                elevationBuilder.withUnregister();
            }

            success = Utils::TaskScheduler::runAsAdmin(elevationBuilder.toString());
        }

        return success;
    });
}

bool AutostartManager::isAutostartEnabled() const {
    return Utils::TaskScheduler::isTaskRegistered();
}

bool AutostartManager::shouldEnableStartupLogging(Qt::KeyboardModifiers modifiers) {
    return modifiers.testFlag(Qt::ControlModifier);
}

} // namespace ModeFlow::Services
