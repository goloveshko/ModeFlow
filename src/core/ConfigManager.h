#pragma once

#include <QMutex>
#include <QObject>
#include <QPoint>
#include <QSize>

#include "Constants.h"
#include "WorkspaceModel.h"

namespace ModeFlow::Core {

class ConfigManager : public QObject {
    Q_OBJECT
public:
    Q_ENUM(StartupAction)

    explicit ConfigManager(QObject* parent = nullptr);

    QList<WorkspaceConfig> getWorkspaces() const;
    void setWorkspaces(const QList<WorkspaceConfig>& list);

    bool loadConfig();
    bool saveConfig();

    QKeySequence nextProfileHotkey() const;
    void setNextProfileHotkey(const QKeySequence& key);

    QString lastActiveProfileId() const;
    void setLastActiveProfileId(const QString& id);

    QString selectedProfileId() const;
    void setSelectedProfileId(const QString& id);

    QString language() const;
    void setLanguage(const QString& locale);

    StartupAction startupAction() const;
    void setStartupAction(StartupAction action);

    QString startupProfileId() const;
    void setStartupProfileId(const QString& id);

    bool audioConfirmation() const;
    void setAudioConfirmation(bool enabled);

    bool autoUpdateEnabled() const;
    void setAutoUpdateEnabled(bool enabled);

    int autostartDelay() const;
    void setAutostartDelay(int seconds);

    bool askConfirmation() const;
    void setAskConfirmation(bool enabled);

    bool isMainWindowMaximized() const;
    void setMainWindowMaximized(bool maximized);

    QPoint mainWindowPos() const;
    void setMainWindowPos(const QPoint& pos);

    QSize mainWindowSize() const;
    void setMainWindowSize(const QSize& size);

    Theme currentTheme() const;
    void setTheme(Theme theme);

    QString currentQtStyleKey() const;
    void setQtStyleKey(const QString& styleKey);

    bool isMainWindowVisible() const;
    void setMainWindowVisible(bool visible);

    qint64 lastUpdateCheckTimestamp() const;
    void setLastUpdateCheckTimestamp(qint64 timestamp);

    bool autoLoggingEnabled() const;
    void setAutoLoggingEnabled(bool enabled);

signals:
    void errorOccurred(const QString& message);

private:
    mutable QMutex m_mutex;

    QList<WorkspaceConfig> m_workspaces;
    QKeySequence m_nextProfileHotkey;
    QString m_language;
    QString m_lastActiveProfileId;
    QString m_selectedProfileId;
    StartupAction m_startupAction = StartupAction::None;
    QString m_startupProfileId;
    bool m_audioConfirmation = true;
    bool m_autoUpdateEnabled = true;
    int m_autostartDelay = 0;
    Theme m_theme = Theme::Light;
    QString m_qtStyleKey{Utils::DefaultQtStyleKey};
    bool m_mainWindowVisible = true;

    bool m_mainWindowMaximized = false;
    QPoint m_mainWindowPos;
    QSize m_mainWindowSize = QSize(600, 450);

    bool m_askConfirmation = true;
    qint64 m_lastUpdateCheckTimestamp = 0;

    bool m_autoLoggingEnabled = false;
};

} // namespace ModeFlow::Core
