#include "ConfigManager.h"

#include <QMutexLocker>
#include <QSettings>

#include "Constants.h"
#include "FontAwesome.h"
#include "LocalizationManager.h"
#include "Logging.h"

namespace ModeFlow::Core {

using namespace Qt::StringLiterals;

ConfigManager::ConfigManager(QObject* parent)
    : QObject(parent), m_language(LocalizationManager::normalizedLocale(QLocale::system().name())) {}

QList<WorkspaceConfig> ConfigManager::getWorkspaces() const {
    QMutexLocker locker(&m_mutex);
    return m_workspaces;
}

void ConfigManager::setWorkspaces(const QList<WorkspaceConfig>& list) {
    QMutexLocker locker(&m_mutex);
    m_workspaces = list;
}

bool ConfigManager::loadConfig() {
    QSettings s;

    if (s.status() == QSettings::AccessError || s.status() == QSettings::FormatError) {
        emit errorOccurred(tr("Configuration file is corrupted or inaccessible."));
        return false;
    }

    QList<WorkspaceConfig> list;
    const int size = s.beginReadArray(u"Workspaces"_sv);
    for (int i = 0; i < size; ++i) {
        s.setArrayIndex(i);
        auto hkStr = s.value(u"hotkey"_sv).toString();
        WorkspaceConfig cfg;
        cfg.id = s.value(u"id"_sv).toString();
        cfg.name = s.value(u"name"_sv, tr("Workspace")).toString();
        cfg.iconSymbol = s.value(u"iconSymbol"_sv).toString();
        cfg.hotkey = QKeySequence(hkStr, QKeySequence::PortableText);
        cfg.displayId = s.value(u"displayId"_sv).toString();
        cfg.audioId = s.value(u"audioId"_sv).toString();
        cfg.skipInCycle = s.value(u"skipInCycle"_sv, false).toBool();

        const int appsSize = s.beginReadArray(u"Apps"_sv);
        for (int j = 0; j < appsSize; ++j) {
            s.setArrayIndex(j);
            AppLaunchConfig appCfg;
            appCfg.appPath = s.value(u"appPath"_sv).toString();
            appCfg.delaySeconds = s.value(u"delaySeconds"_sv, 0).toInt();
            appCfg.closeOnExit = s.value(u"closeOnExit"_sv, false).toBool();
            appCfg.delaySeconds = std::clamp(appCfg.delaySeconds, 0, Utils::MaxAppLaunchDelaySeconds);
            cfg.appsToLaunch.append(appCfg);
        }
        s.endArray();

        list.append(cfg);
    }
    s.endArray();

    QSet<QString> seenIds;
    for (auto& cfg : list) {
        if (cfg.id.isEmpty() || seenIds.contains(cfg.id)) {
            cfg.id = QUuid::createUuid().toString();
        }
        seenIds.insert(cfg.id);
        if (cfg.iconSymbol.isEmpty()) {
            cfg.iconSymbol = Gui::FontAwesome::Desktop;
        }
    }

    auto hkStr = s.value(u"nextProfileHotkey"_sv).toString();
    auto nextProfileHotkey = QKeySequence(hkStr, QKeySequence::PortableText);
    auto language = LocalizationManager::normalizedLocale(s.value(u"language"_sv, QLocale::system().name()).toString());
    auto theme = static_cast<Theme>(s.value(u"theme"_sv, static_cast<int>(Theme::Light)).toInt());
    auto qtStyleKey = s.value(u"qtStyleKey"_sv, Utils::DefaultQtStyleKey.toString()).toString();
    auto lastActiveProfileId = s.value(u"lastActiveProfileId"_sv).toString();
    auto selectedProfileId = s.value(u"selectedProfileId"_sv).toString();
    auto startupAction = static_cast<StartupAction>(s.value(u"startupAction"_sv, 0).toInt());
    auto startupProfileId = s.value(u"startupProfileId"_sv).toString();
    auto audioConfirmation = s.value(u"audioFeedback"_sv, true).toBool();
    auto autoUpdateEnabled = s.value(u"autoUpdate"_sv, true).toBool();
    auto autostartDelay = s.value(u"autostartDelay"_sv, 5).toInt();
    auto askConfirmation = s.value(u"askConfirmation"_sv, true).toBool();

    auto mainWindowVisible = s.value(u"window/visible"_sv, true).toBool();
    auto mainWindowMaximized = s.value(u"window/maximized"_sv, false).toBool();
    auto mainWindowPos = s.value(u"window/pos"_sv).toPoint();
    auto mainWindowSize = s.value(u"window/size"_sv, QSize(600, 450)).toSize();

    auto lastUpdateCheckTimestamp = s.value(u"update/lastCheckTimestamp"_sv, 0).toLongLong();
    
    auto autoLoggingEnabled = s.value(u"loggingEnabled"_sv, false).toBool();

    // Fast atomic update of member state using the double-buffered snapshot pattern
    {
        QMutexLocker locker(&m_mutex);
        m_workspaces = std::move(list);
        m_nextProfileHotkey = nextProfileHotkey;
        m_language = language;
        m_theme = theme;
        m_qtStyleKey = qtStyleKey;
        m_lastActiveProfileId = lastActiveProfileId;
        m_selectedProfileId = selectedProfileId;
        m_startupAction = startupAction;
        m_startupProfileId = startupProfileId;
        m_audioConfirmation = audioConfirmation;
        m_autoUpdateEnabled = autoUpdateEnabled;
        m_autostartDelay = autostartDelay;
        m_askConfirmation = askConfirmation;
        m_mainWindowVisible = mainWindowVisible;
        m_mainWindowMaximized = mainWindowMaximized;
        m_mainWindowPos = mainWindowPos;
        m_mainWindowSize = mainWindowSize;
        m_lastUpdateCheckTimestamp = lastUpdateCheckTimestamp;
        m_autoLoggingEnabled = autoLoggingEnabled;
    }

    return true;
}

bool ConfigManager::saveConfig() {
    // Take a thread-safe snapshot of member state under lock to prevent UI stalls during disk I/O
    QList<WorkspaceConfig> list;
    QKeySequence nextProfileHotkey;
    QString language;
    Theme theme;
    QString qtStyleKey;
    QString lastActiveProfileId;
    QString selectedProfileId;
    StartupAction startupAction;
    QString startupProfileId;
    bool audioConfirmation;
    bool autoUpdateEnabled;
    int autostartDelay;
    bool askConfirmation;
    bool mainWindowVisible;
    bool mainWindowMaximized;
    QPoint mainWindowPos;
    QSize mainWindowSize;
    qint64 lastUpdateCheckTimestamp;
    bool autoLoggingEnabled;

    {
        QMutexLocker locker(&m_mutex);
        list = m_workspaces;
        nextProfileHotkey = m_nextProfileHotkey;
        language = m_language;
        theme = m_theme;
        qtStyleKey = m_qtStyleKey;
        lastActiveProfileId = m_lastActiveProfileId;
        selectedProfileId = m_selectedProfileId;
        startupAction = m_startupAction;
        startupProfileId = m_startupProfileId;
        audioConfirmation = m_audioConfirmation;
        autoUpdateEnabled = m_autoUpdateEnabled;
        autostartDelay = m_autostartDelay;
        askConfirmation = m_askConfirmation;
        mainWindowVisible = m_mainWindowVisible;
        mainWindowMaximized = m_mainWindowMaximized;
        mainWindowPos = m_mainWindowPos;
        mainWindowSize = m_mainWindowSize;
        lastUpdateCheckTimestamp = m_lastUpdateCheckTimestamp;
        autoLoggingEnabled = m_autoLoggingEnabled;
    }

    QSettings s;

    s.beginWriteArray("Workspaces");
    for (int i = 0; i < list.size(); ++i) {
        s.setArrayIndex(i);
        s.setValue(u"id"_sv, list[i].id);
        s.setValue(u"name"_sv, list[i].name);
        s.setValue(u"iconSymbol"_sv, list[i].iconSymbol);
        s.setValue(u"hotkey"_sv, list[i].hotkey.toString(QKeySequence::PortableText));
        s.setValue(u"displayId"_sv, list[i].displayId);
        s.setValue(u"audioId"_sv, list[i].audioId);
        s.setValue(u"skipInCycle"_sv, list[i].skipInCycle);

        s.beginWriteArray(u"Apps"_sv, list[i].appsToLaunch.size());
        for (int j = 0; j < list[i].appsToLaunch.size(); ++j) {
            s.setArrayIndex(j);
            const auto& app = list[i].appsToLaunch[j];
            s.setValue(u"appPath"_sv, app.appPath);
            s.setValue(u"delaySeconds"_sv, app.delaySeconds);
            s.setValue(u"closeOnExit"_sv, app.closeOnExit);
        }
        s.endArray();
    }
    s.endArray();

    s.setValue(u"nextProfileHotkey"_sv, nextProfileHotkey.toString(QKeySequence::PortableText));
    s.setValue(u"language"_sv, language);
    s.setValue(u"theme"_sv, static_cast<int>(theme));
    s.setValue(u"qtStyleKey"_sv, qtStyleKey);
    s.setValue(u"lastActiveProfileId"_sv, lastActiveProfileId);
    s.setValue(u"selectedProfileId"_sv, selectedProfileId);
    s.setValue(u"startupAction"_sv, static_cast<int>(startupAction));
    s.setValue(u"startupProfileId"_sv, startupProfileId);
    s.setValue(u"audioFeedback"_sv, audioConfirmation);
    s.setValue(u"autoUpdate"_sv, autoUpdateEnabled);
    s.setValue(u"autostartDelay"_sv, autostartDelay);
    s.setValue(u"askConfirmation"_sv, askConfirmation);

    s.setValue(u"window/visible"_sv, mainWindowVisible);
    s.setValue(u"window/maximized"_sv, mainWindowMaximized);
    s.setValue(u"window/pos"_sv, mainWindowPos);
    s.setValue(u"window/size"_sv, mainWindowSize);

    s.setValue(u"update/lastCheckTimestamp"_sv, lastUpdateCheckTimestamp);

    s.setValue(u"loggingEnabled"_sv, autoLoggingEnabled);

    s.sync();

    if (s.status() != QSettings::NoError) {
        qCCritical(lcCore) << "ConfigManager::saveConfig - Failed to write settings:" << s.status();
        emit errorOccurred(tr("Failed to save settings. Check disk space or permissions."));
        return false;
    }

    return true;
}

QKeySequence ConfigManager::nextProfileHotkey() const {
    QMutexLocker locker(&m_mutex);
    return m_nextProfileHotkey;
}

void ConfigManager::setNextProfileHotkey(const QKeySequence& key) {
    QMutexLocker locker(&m_mutex);
    m_nextProfileHotkey = key;
}

QString ConfigManager::language() const {
    QMutexLocker locker(&m_mutex);
    return m_language;
}

void ConfigManager::setLanguage(const QString& locale) {
    QMutexLocker locker(&m_mutex);
    m_language = LocalizationManager::normalizedLocale(locale);
}

QString ConfigManager::lastActiveProfileId() const {
    QMutexLocker locker(&m_mutex);
    return m_lastActiveProfileId;
}

void ConfigManager::setLastActiveProfileId(const QString& id) {
    QMutexLocker locker(&m_mutex);
    m_lastActiveProfileId = id;
}

QString ConfigManager::selectedProfileId() const {
    QMutexLocker locker(&m_mutex);
    return m_selectedProfileId;
}

void ConfigManager::setSelectedProfileId(const QString& id) {
    QMutexLocker locker(&m_mutex);
    m_selectedProfileId = id;
}

StartupAction ConfigManager::startupAction() const {
    QMutexLocker locker(&m_mutex);
    return m_startupAction;
}

void ConfigManager::setStartupAction(StartupAction action) {
    QMutexLocker locker(&m_mutex);
    m_startupAction = action;
}

QString ConfigManager::startupProfileId() const {
    QMutexLocker locker(&m_mutex);
    return m_startupProfileId;
}

void ConfigManager::setStartupProfileId(const QString& id) {
    QMutexLocker locker(&m_mutex);
    m_startupProfileId = id;
}

bool ConfigManager::audioConfirmation() const {
    QMutexLocker locker(&m_mutex);
    return m_audioConfirmation;
}

void ConfigManager::setAudioConfirmation(bool enabled) {
    QMutexLocker locker(&m_mutex);
    m_audioConfirmation = enabled;
}

bool ConfigManager::autoUpdateEnabled() const {
    QMutexLocker locker(&m_mutex);
    return m_autoUpdateEnabled;
}

void ConfigManager::setAutoUpdateEnabled(bool enabled) {
    QMutexLocker locker(&m_mutex);
    m_autoUpdateEnabled = enabled;
}

int ConfigManager::autostartDelay() const {
    QMutexLocker locker(&m_mutex);
    return m_autostartDelay;
}

void ConfigManager::setAutostartDelay(int seconds) {
    QMutexLocker locker(&m_mutex);
    m_autostartDelay = seconds;
}

bool ConfigManager::askConfirmation() const {
    QMutexLocker locker(&m_mutex);
    return m_askConfirmation;
}

void ConfigManager::setAskConfirmation(bool enabled) {
    QMutexLocker locker(&m_mutex);
    m_askConfirmation = enabled;
}

bool ConfigManager::isMainWindowMaximized() const {
    QMutexLocker locker(&m_mutex);
    return m_mainWindowMaximized;
}

void ConfigManager::setMainWindowMaximized(bool maximized) {
    QMutexLocker locker(&m_mutex);
    m_mainWindowMaximized = maximized;
}

QPoint ConfigManager::mainWindowPos() const {
    QMutexLocker locker(&m_mutex);
    return m_mainWindowPos;
}

void ConfigManager::setMainWindowPos(const QPoint& pos) {
    QMutexLocker locker(&m_mutex);
    m_mainWindowPos = pos;
}

QSize ConfigManager::mainWindowSize() const {
    QMutexLocker locker(&m_mutex);
    return m_mainWindowSize;
}

void ConfigManager::setMainWindowSize(const QSize& size) {
    QMutexLocker locker(&m_mutex);
    m_mainWindowSize = size;
}

Theme ConfigManager::currentTheme() const {
    QMutexLocker locker(&m_mutex);
    return m_theme;
}

void ConfigManager::setTheme(Theme theme) {
    QMutexLocker locker(&m_mutex);
    m_theme = theme;
}

QString ConfigManager::currentQtStyleKey() const {
    QMutexLocker locker(&m_mutex);
    return m_qtStyleKey;
}

void ConfigManager::setQtStyleKey(const QString& styleKey) {
    if (!styleKey.isEmpty()) {
        QMutexLocker locker(&m_mutex);
        m_qtStyleKey = styleKey;
    }
}

bool ConfigManager::isMainWindowVisible() const {
    QMutexLocker locker(&m_mutex);
    return m_mainWindowVisible;
}

void ConfigManager::setMainWindowVisible(bool visible) {
    QMutexLocker locker(&m_mutex);
    m_mainWindowVisible = visible;
}

qint64 ConfigManager::lastUpdateCheckTimestamp() const {
    QMutexLocker locker(&m_mutex);
    return m_lastUpdateCheckTimestamp;
}

void ConfigManager::setLastUpdateCheckTimestamp(qint64 timestamp) {
    QMutexLocker locker(&m_mutex);
    m_lastUpdateCheckTimestamp = timestamp;
}

bool ConfigManager::autoLoggingEnabled() const {
    return m_autoLoggingEnabled;
}

void ConfigManager::setAutoLoggingEnabled(bool enabled) {
    m_autoLoggingEnabled = enabled;
}

} // namespace ModeFlow::Core