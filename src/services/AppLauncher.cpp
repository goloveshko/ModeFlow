#include "AppLauncher.h"

#include <QDir>
#include <QProcess>

#include "Logging.h"
#include "SystemUtils.h"

namespace ModeFlow::Services {
using namespace Qt::StringLiterals;

namespace {
QStringList getBlockedSystemPaths() {
    const QString winDir = qEnvironmentVariable("windir");
    if (winDir.isEmpty()) {
        qCWarning(lcService) << "Failed to retrieve %windir% environment variable.";
        return {};
    }

    const QDir winQDir(winDir);

    return {QDir::toNativeSeparators(winQDir.filePath(u"System32"_s)),
            QDir::toNativeSeparators(winQDir.filePath(u"SysWOW64"_s)),
            QDir::toNativeSeparators(winQDir.filePath(u"Sysnative"_s)),
            QDir::toNativeSeparators(winQDir.filePath(u"Microsoft.NET"_s))};
}
} // namespace

AppLauncher::AppLauncher(QObject* parent) : QObject(parent) {}

bool AppLauncher::launch(const QString& path, int delaySeconds) {
    if (path.isEmpty()) {
        return true;
    }

    QFileInfo checkFile(path);

    if (checkFile.isSymLink()) {
        qCWarning(lcService) << "Symlinks are not allowed:" << path;
        emit errorOccurred(tr("Refusing to launch a symbolic link: %1").arg(path));
        return false;
    }

    if (!checkFile.exists() || !checkFile.isFile()) {
        qCWarning(lcService) << "File does not exist:" << path;
        emit errorOccurred(tr("Application file does not exist: %1").arg(path));
        return false;
    }

    const QString canonicalPath = QDir::toNativeSeparators(checkFile.canonicalFilePath());
    const QStringList blockedPaths = getBlockedSystemPaths();

    for (const auto& blockedPath : blockedPaths) {
        QDir blockedDir(blockedPath);
        const QString relative = blockedDir.relativeFilePath(canonicalPath);

        if (!relative.startsWith(u"../"_s) && relative != u".."_s) {
            qCWarning(lcService) << "System path blocked (containment detected):" << canonicalPath;
            emit errorOccurred(tr("Launching system executables is blocked: %1").arg(canonicalPath));
            return false;
        }
    }

    const QString suffix = checkFile.suffix().toLower();
    if (suffix != u"exe"_s) {
        qCWarning(lcService) << "Only .exe files are allowed:" << path << " (." << suffix << ")";
        emit errorOccurred(tr("Only .exe files can be launched: %1").arg(path));
        return false;
    }

    if (delaySeconds <= 0) {
        return execute(path) > 0;
    } else {
        qCDebug(lcService) << "Scheduled launch in" << delaySeconds << "sec for:" << path;
        QTimer::singleShot(std::chrono::seconds(delaySeconds), this, [this, path]() { execute(path); });
        return true;
    }
}

bool AppLauncher::launchSequence(const QString& profileId, const QList<Core::AppLaunchConfig>& apps) {
    if (apps.isEmpty())
        return true;

    for (const auto& app : apps) {
        if (app.appPath.isEmpty())
            continue;

        // If there's a startup delay, we instantiate a trackable QTimer instead of an anonymous singleShot.
        // This allows us to abort the delayed launch if the user switches away from this profile before the delay
        // expires.
        if (app.delaySeconds > 0) {
            auto* timer = new QTimer(this);
            timer->setSingleShot(true);
            timer->setInterval(std::chrono::seconds(app.delaySeconds));

            connect(timer, &QTimer::timeout, this, [this, timer, profileId, app]() {
                auto& list = m_pendingTimers[profileId];
                list.removeOne(timer);
                if (list.isEmpty()) {
                    m_pendingTimers.remove(profileId);
                }
                timer->deleteLater();

                launchAndTrack(profileId, app);
            });

            m_pendingTimers[profileId].append(timer);
            timer->start();
            qCDebug(lcService) << "Scheduled delayed launch for" << app.appPath << "in" << app.delaySeconds
                               << "seconds";
        } else {
            launchAndTrack(profileId, app);
        }
    }

    return true;
}

void AppLauncher::terminateProfileProcesses(const QString& profileId) {
    auto it = m_activeProcesses.find(profileId);
    if (it != m_activeProcesses.end()) {
        for (qint64 pid : *it) {
            if (pid <= 0)
                continue;

            if (Utils::SystemUtils::terminateProcess(pid)) {
                qCDebug(lcService) << "Terminated process PID" << pid << "for profile" << profileId;
            } else {
                qCWarning(lcService) << "Failed to terminate process PID" << pid << "for profile" << profileId;
            }
        }
        m_activeProcesses.erase(it);
    }

    // Abort and delete all scheduled delayed launch timers for this profile.
    // This prevents "runaway" processes from launching on top of a newly applied workspace.
    auto timerIt = m_pendingTimers.find(profileId);
    if (timerIt != m_pendingTimers.end()) {
        qCDebug(lcService) << "Cancelling" << timerIt.value().size() << "pending delayed launches for profile"
                           << profileId;
        for (QTimer* timer : timerIt.value()) {
            if (timer) {
                timer->stop();
                timer->deleteLater();
            }
        }
        m_pendingTimers.erase(timerIt);
    }
}

qint64 AppLauncher::execute(const QString& path) {
    qCDebug(lcService) << "Launching actual process:" << path;

    const QString workingDir = QFileInfo(path).absolutePath();
    qint64 pid = 0;
    const bool success = QProcess::startDetached(path, QStringList(), workingDir, &pid);

    if (!success) {
        qCWarning(lcService) << "Failed to start process:" << path;
        emit errorOccurred(tr("Failed to start application: %1").arg(path));
        return 0;
    }

    return pid;
}

void AppLauncher::trackProcess(const QString& profileId, qint64 pid) {
    m_activeProcesses[profileId].append(pid);
    qCDebug(lcService) << "Tracked process PID" << pid << "for profile" << profileId << "(closeOnExit is active)";
}

void AppLauncher::launchAndTrack(const QString& profileId, const Core::AppLaunchConfig& app) {
    qint64 pid = execute(app.appPath);
    if (pid > 0 && app.closeOnExit) {
        trackProcess(profileId, pid);
    }
}

} // namespace ModeFlow::Services
