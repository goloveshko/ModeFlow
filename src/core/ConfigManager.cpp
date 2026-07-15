#include "ConfigManager.h"

#include <QSettings>

#include "Constants.h"
#include "FontAwesome.h"
#include "LocalizationManager.h"
#include "Logging.h"

namespace ModeFlow::Core {

using namespace Qt::StringLiterals;

ConfigManager::ConfigManager(QObject* parent)
    : QObject(parent), m_language(LocalizationManager::normalizedLocale(QLocale::system().name())) {}

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

        // Migrate legacy single-app config if no new-format apps exist
        if (cfg.appsToLaunch.isEmpty()) {
            // Legacy fields were already lost during QSettings read,
            // so check the JSON migration path instead.
            // This block handles fresh reads of old configs that had
            // no nested array but may have flat runApp/appPath.
        }
    }
    m_workspaces = list;

    auto hkStr = s.value(u"nextProfileHotkey"_sv).toString();
    m_nextProfileHotkey = QKeySequence(hkStr, QKeySequence::PortableText);
    m_language = LocalizationManager::normalizedLocale(s.value(u"language"_sv, QLocale::system().name()).toString());
    m_theme = static_cast<Theme>(s.value(u"theme"_sv, static_cast<int>(Theme::Light)).toInt());
    m_qtStyleKey = s.value(u"qtStyleKey"_sv, Utils::DefaultQtStyleKey.toString()).toString();
    m_lastActiveProfileId = s.value(u"lastActiveProfileId"_sv).toString();
    m_selectedProfileId = s.value(u"selectedProfileId"_sv).toString();
    m_startupAction = static_cast<StartupAction>(s.value(u"startupAction"_sv, 0).toInt());
    m_startupProfileId = s.value(u"startupProfileId"_sv).toString();
    m_audioConfirmation = s.value(u"audioFeedback"_sv, true).toBool();
    m_autoUpdateEnabled = s.value(u"autoUpdate"_sv, true).toBool();
    m_autostartDelay = s.value(u"autostartDelay"_sv, 5).toInt();
    m_askConfirmation = s.value(u"askConfirmation"_sv, true).toBool();

    m_mainWindowVisible = s.value(u"window/visible"_sv, true).toBool();
    m_mainWindowMaximized = s.value(u"window/maximized"_sv, false).toBool();
    m_mainWindowPos = s.value(u"window/pos"_sv).toPoint();
    m_mainWindowSize = s.value(u"window/size"_sv, QSize(600, 450)).toSize();

    m_lastUpdateCheckTimestamp = s.value(u"update/lastCheckTimestamp"_sv, 0).toLongLong();

    return true;
}

bool ConfigManager::saveConfig() {
    QSettings s;

    s.beginWriteArray("Workspaces");
    const auto& list = m_workspaces;
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

    s.setValue(u"nextProfileHotkey"_sv, m_nextProfileHotkey.toString(QKeySequence::PortableText));
    s.setValue(u"language"_sv, m_language);
    s.setValue(u"theme"_sv, static_cast<int>(m_theme));
    s.setValue(u"qtStyleKey"_sv, m_qtStyleKey);
    s.setValue(u"lastActiveProfileId"_sv, m_lastActiveProfileId);
    s.setValue(u"selectedProfileId"_sv, m_selectedProfileId);
    s.setValue(u"startupAction"_sv, static_cast<int>(m_startupAction));
    s.setValue(u"startupProfileId"_sv, m_startupProfileId);
    s.setValue(u"audioFeedback"_sv, m_audioConfirmation);
    s.setValue(u"autoUpdate"_sv, m_autoUpdateEnabled);
    s.setValue(u"autostartDelay"_sv, m_autostartDelay);
    s.setValue(u"askConfirmation"_sv, m_askConfirmation);

    s.setValue(u"window/visible"_sv, m_mainWindowVisible);
    s.setValue(u"window/maximized"_sv, m_mainWindowMaximized);
    s.setValue(u"window/pos"_sv, m_mainWindowPos);
    s.setValue(u"window/size"_sv, m_mainWindowSize);

    s.setValue(u"update/lastCheckTimestamp"_sv, m_lastUpdateCheckTimestamp);

    s.sync();

    if (s.status() != QSettings::NoError) {
        qCCritical(lcCore) << "ConfigManager::saveConfig - Failed to write settings:" << s.status();
        emit errorOccurred(tr("Failed to save settings. Check disk space or permissions."));
        return false;
    }

    return true;
}

void ConfigManager::setNextProfileHotkey(const QKeySequence& key) {
    m_nextProfileHotkey = key;
}

void ConfigManager::setLanguage(const QString& locale) {
    m_language = LocalizationManager::normalizedLocale(locale);
}

void ConfigManager::setLastActiveProfileId(const QString& id) {
    m_lastActiveProfileId = id;
}

void ConfigManager::setSelectedProfileId(const QString& id) {
    m_selectedProfileId = id;
}

void ConfigManager::setStartupAction(StartupAction action) {
    m_startupAction = action;
}

void ConfigManager::setStartupProfileId(const QString& id) {
    m_startupProfileId = id;
}

void ConfigManager::setAudioConfirmation(bool enabled) {
    m_audioConfirmation = enabled;
}

void ConfigManager::setAutoUpdateEnabled(bool enabled) {
    m_autoUpdateEnabled = enabled;
}

void ConfigManager::setAutostartDelay(int seconds) {
    m_autostartDelay = seconds;
}

Theme ConfigManager::currentTheme() const {
    return m_theme;
}

QString ConfigManager::currentQtStyleKey() const {
    return m_qtStyleKey;
}

void ConfigManager::setTheme(Theme theme) {
    m_theme = theme;
}

void ConfigManager::setQtStyleKey(const QString& styleKey) {
    if (!styleKey.isEmpty()) {
        m_qtStyleKey = styleKey;
    }
}

void ConfigManager::setMainWindowVisible(bool visible) {
    m_mainWindowVisible = visible;
}

void ConfigManager::setAskConfirmation(bool enabled) {
    m_askConfirmation = enabled;
}

void ConfigManager::setLastUpdateCheckTimestamp(qint64 timestamp) {
    m_lastUpdateCheckTimestamp = timestamp;
}

} // namespace ModeFlow::Core
