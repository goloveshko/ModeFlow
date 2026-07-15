#include "SettingsManagerImpl.h"

#include "ConfigManager.h"
#include "LocalizationManager.h"
#include "StyleManager.h"
#include "WindowsAutostartManager.h"

namespace ModeFlow::Core {

SettingsManagerImpl::SettingsManagerImpl(ConfigManager* cm, Services::WindowsAutostartManager* as,
                                         LocalizationManager* lm, Services::StyleManager* sm)
    : m_config(cm), m_autostart(as), m_loc(lm), m_styleManager(sm) {}

QList<LanguageData> SettingsManagerImpl::availableLanguages() const {
    return m_loc->availableLanguages();
}

QString SettingsManagerImpl::currentLanguage() const {
    return m_config->language();
}

void SettingsManagerImpl::setLanguage(const QString& locale) {
    m_config->setLanguage(locale);
    m_loc->switchLanguage(locale);
}

void SettingsManagerImpl::setLanguagePreference(const QString& locale) {
    m_config->setLanguage(locale);
}

bool SettingsManagerImpl::isAutostartEnabled() const {
    return m_autostart->isAutostartEnabled();
}

QFuture<bool> SettingsManagerImpl::isAutostartEnabledAsync() const {
    return m_autostart->checkIsRegisteredAsync();
}

QFuture<bool> SettingsManagerImpl::requestAutostartToggleAsync(bool enabled, int delay) {
    return m_autostart->toggleAsync(enabled, delay);
}

int SettingsManagerImpl::autostartDelay() const {
    return m_config->autostartDelay();
}

void SettingsManagerImpl::setAutostartDelay(int seconds) {
    m_config->setAutostartDelay(seconds);
}

bool SettingsManagerImpl::audioConfirmation() const {
    return m_config->audioConfirmation();
}

void SettingsManagerImpl::setAudioConfirmation(bool enabled) {
    m_config->setAudioConfirmation(enabled);
}

bool SettingsManagerImpl::autoUpdateEnabled() const {
    return m_config->autoUpdateEnabled();
}

void SettingsManagerImpl::setAutoUpdateEnabled(bool enabled) {
    m_config->setAutoUpdateEnabled(enabled);
}

QKeySequence SettingsManagerImpl::nextProfileHotkey() const {
    return m_config->nextProfileHotkey();
}

void SettingsManagerImpl::setNextProfileHotkey(const QKeySequence& seq) {
    m_config->setNextProfileHotkey(seq);
}

StartupAction SettingsManagerImpl::startupAction() const {
    return m_config->startupAction();
}

QString SettingsManagerImpl::startupProfileId() const {
    return m_config->startupProfileId();
}

void SettingsManagerImpl::setStartupBehavior(StartupAction action, const QString& profileId) {
    m_config->setStartupAction(action);
    m_config->setStartupProfileId(profileId);
}

bool SettingsManagerImpl::saveSettings() {
    return m_config->saveConfig();
}

Theme SettingsManagerImpl::currentTheme() const {
    return m_config->currentTheme();
}

QString SettingsManagerImpl::currentQtStyleKey() const {
    return m_config->currentQtStyleKey();
}

void SettingsManagerImpl::setTheme(Theme theme, const QString& qtStyleKey) {
    m_styleManager->setTheme(theme, qtStyleKey);
    m_config->setTheme(theme);
    m_config->setQtStyleKey(m_styleManager->currentQtStyleKey());
}

void SettingsManagerImpl::setThemePreference(Theme theme, const QString& qtStyleKey) {
    m_config->setTheme(theme);
    m_config->setQtStyleKey(qtStyleKey);
}

QList<Core::ThemeData> SettingsManagerImpl::availableThemes() const {
    return m_styleManager->availableThemes();
}

bool SettingsManagerImpl::isMainWindowMaximized() const {
    return m_config->isMainWindowMaximized();
}

void SettingsManagerImpl::setMainWindowMaximized(bool maximized) {
    m_config->setMainWindowMaximized(maximized);
}

QPoint SettingsManagerImpl::mainWindowPos() const {
    return m_config->mainWindowPos();
}

void SettingsManagerImpl::setMainWindowPos(const QPoint& pos) {
    m_config->setMainWindowPos(pos);
}

QSize SettingsManagerImpl::mainWindowSize() const {
    return m_config->mainWindowSize();
}

void SettingsManagerImpl::setMainWindowSize(const QSize& size) {
    m_config->setMainWindowSize(size);
}

bool SettingsManagerImpl::isMainWindowVisible() const {
    return m_config->isMainWindowVisible();
}

void SettingsManagerImpl::setMainWindowVisible(bool visible) {
    m_config->setMainWindowVisible(visible);
}

bool SettingsManagerImpl::askConfirmation() const {
    return m_config->askConfirmation();
}

void SettingsManagerImpl::setAskConfirmation(bool enabled) {
    m_config->setAskConfirmation(enabled);
}

} // namespace ModeFlow::Core
