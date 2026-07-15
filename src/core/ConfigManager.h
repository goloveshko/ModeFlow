#pragma once

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

    QList<WorkspaceConfig> getWorkspaces() const { return m_workspaces; }
    void setWorkspaces(const QList<WorkspaceConfig>& list) { m_workspaces = list; }

    bool loadConfig();
    bool saveConfig();
    QKeySequence nextProfileHotkey() const { return m_nextProfileHotkey; }
    QString lastActiveProfileId() const { return m_lastActiveProfileId; }
    void setLastActiveProfileId(const QString& id);
    QString selectedProfileId() const { return m_selectedProfileId; }
    void setSelectedProfileId(const QString& id);

    const QString& language() const { return m_language; }
    StartupAction startupAction() const { return m_startupAction; }
    const QString& startupProfileId() const { return m_startupProfileId; }
    bool audioConfirmation() const { return m_audioConfirmation; }
    bool autoUpdateEnabled() const { return m_autoUpdateEnabled; }
    int autostartDelay() const { return m_autostartDelay; }
    bool askConfirmation() const { return m_askConfirmation; }

    bool isMainWindowMaximized() const { return m_mainWindowMaximized; }
    QPoint mainWindowPos() const { return m_mainWindowPos; }
    QSize mainWindowSize() const { return m_mainWindowSize; }

    void setNextProfileHotkey(const QKeySequence& key);
    void setLanguage(const QString& locale);
    void setStartupAction(StartupAction action);
    void setStartupProfileId(const QString& id);
    void setAudioConfirmation(bool enabled);
    void setAutoUpdateEnabled(bool enabled);
    void setAutostartDelay(int seconds);
    void setAskConfirmation(bool enabled);

    void setMainWindowMaximized(bool maximized) { m_mainWindowMaximized = maximized; }
    void setMainWindowPos(const QPoint& pos) { m_mainWindowPos = pos; }
    void setMainWindowSize(const QSize& size) { m_mainWindowSize = size; }

    Theme currentTheme() const;
    QString currentQtStyleKey() const;
    void setTheme(Theme theme);
    void setQtStyleKey(const QString& styleKey);

    bool isMainWindowVisible() const { return m_mainWindowVisible; }
    void setMainWindowVisible(bool visible);

    qint64 lastUpdateCheckTimestamp() const { return m_lastUpdateCheckTimestamp; }
    void setLastUpdateCheckTimestamp(qint64 timestamp);

signals:
    void errorOccurred(const QString& message);

private:
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
    bool m_mainWindowVisible = true; // Default to true for new users

    bool m_mainWindowMaximized = false;
    QPoint m_mainWindowPos;
    QSize m_mainWindowSize = QSize(600, 450); // Default Fluent size

    bool m_askConfirmation = true;

    qint64 m_lastUpdateCheckTimestamp = 0;
};
} // namespace ModeFlow::Core