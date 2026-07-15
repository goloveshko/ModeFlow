#pragma once

#include <QList>
#include <QMap>
#include <QObject>
#include <QTimer>

#include "ConfigTypes.h"

namespace ModeFlow::Services {

class AppLauncher : public QObject {
    Q_OBJECT
public:
    explicit AppLauncher(QObject* parent = nullptr);

    virtual bool launch(const QString& path, int delaySeconds);
    virtual bool launchSequence(const QString& profileId, const QList<Core::AppLaunchConfig>& apps);

    void terminateProfileProcesses(const QString& profileId);

    QMap<QString, QList<qint64>> activeProcesses() const { return m_activeProcesses; }

signals:
    void errorOccurred(const QString& message);

private:
    qint64 execute(const QString& path);
    void trackProcess(const QString& profileId, qint64 pid);
    void launchAndTrack(const QString& profileId, const Core::AppLaunchConfig& app);

    QMap<QString, QList<qint64>> m_activeProcesses;

    // Tracks active delayed launch timers for each profile ID
    QMap<QString, QList<QTimer*>> m_pendingTimers;
};
} // namespace ModeFlow::Services