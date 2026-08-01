#include "SettingsManager.h"

#include "ConfigManager.h"
#include "LocalizationManager.h"
#include "StyleManager.h"
#include "WindowsAutostartManager.h"

namespace ModeFlow::Core {

SettingsManager::SettingsManager(ConfigManager* cm, Services::WindowsAutostartManager* as, LocalizationManager* lm,
                                 StyleManager* sm)
    : m_config(cm), m_autostart(as), m_loc(lm), m_styleManager(sm) {}

QList<LanguageData> SettingsManager::availableLanguages() const {
    return m_loc->availableLanguages();
}

QString SettingsManager::currentLanguage() const {
    return m_config->language();
}

void SettingsManager::setLanguage(const QString& locale) {
    m_config->setLanguage(locale);
    m_loc->switchLanguage(locale);
}

void SettingsManager::setLanguagePreference(const QString& locale) {
    m_config->setLanguage(locale);
}

bool SettingsManager::autostartEnabled() const {
    return m_autostart->isAutostartEnabled();
}

QFuture<bool> SettingsManager::autostartEnabledAsync() const {
    return m_autostart->checkIsRegisteredAsync();
}

QFuture<bool> SettingsManager::requestAutostartToggleAsync(bool enabled, int delay) {
    return m_autostart->toggleAsync(enabled, delay);
}

int SettingsManager::autostartDelay() const {
    return m_config->autostartDelay();
}

void SettingsManager::setAutostartDelay(int seconds) {
    m_config->setAutostartDelay(seconds);
}

bool SettingsManager::audioConfirmation() const {
    return m_config->audioConfirmation();
}

void SettingsManager::setAudioConfirmation(bool enabled) {
    m_config->setAudioConfirmation(enabled);
}

bool SettingsManager::autoUpdateEnabled() const {
    return m_config->autoUpdateEnabled();
}

void SettingsManager::setAutoUpdateEnabled(bool enabled) {
    m_config->setAutoUpdateEnabled(enabled);
}

QKeySequence SettingsManager::nextProfileHotkey() const {
    return m_config->nextProfileHotkey();
}

void SettingsManager::setNextProfileHotkey(const QKeySequence& seq) {
    m_config->setNextProfileHotkey(seq);
}

StartupAction SettingsManager::startupAction() const {
    return m_config->startupAction();
}

QString SettingsManager::startupProfileId() const {
    return m_config->startupProfileId();
}

void SettingsManager::setStartupBehavior(StartupAction action, const QString& profileId) {
    m_config->setStartupAction(action);
    m_config->setStartupProfileId(profileId);
}

bool SettingsManager::saveSettings() {
    return m_config->saveConfig();
}

Theme SettingsManager::currentTheme() const {
    return m_config->currentTheme();
}

QString SettingsManager::currentQtStyleKey() const {
    return m_config->currentQtStyleKey();
}

void SettingsManager::setTheme(Theme theme, const QString& qtStyleKey) {
    m_styleManager->setTheme(theme, qtStyleKey);
    m_config->setTheme(theme);
    m_config->setQtStyleKey(m_styleManager->currentQtStyleKey());
}

void SettingsManager::setThemePreference(Theme theme, const QString& qtStyleKey) {
    m_config->setTheme(theme);
    m_config->setQtStyleKey(qtStyleKey);
}

QList<Core::ThemeData> SettingsManager::availableThemes() const {
    return m_styleManager->availableThemes();
}

bool SettingsManager::mainWindowMaximized() const {
    return m_config->isMainWindowMaximized();
}

void SettingsManager::setMainWindowMaximized(bool maximized) {
    m_config->setMainWindowMaximized(maximized);
}

QPoint SettingsManager::mainWindowPos() const {
    return m_config->mainWindowPos();
}

void SettingsManager::setMainWindowPos(const QPoint& pos) {
    m_config->setMainWindowPos(pos);
}

QSize SettingsManager::mainWindowSize() const {
    return m_config->mainWindowSize();
}

void SettingsManager::setMainWindowSize(const QSize& size) {
    m_config->setMainWindowSize(size);
}

bool SettingsManager::mainWindowVisible() const {
    return m_config->isMainWindowVisible();
}

void SettingsManager::setMainWindowVisible(bool visible) {
    m_config->setMainWindowVisible(visible);
}

bool SettingsManager::loggingEnabled() const {
    return m_config->loggingEnabled();
}

void SettingsManager::setLoggingEnabled(bool enabled) {
    m_config->setLoggingEnabled(enabled);
}

bool SettingsManager::askConfirmation() const {
    return m_config->askConfirmation();
}

void SettingsManager::setAskConfirmation(bool enabled) {
    m_config->setAskConfirmation(enabled);
}

} // namespace ModeFlow::Core