#include "DialogManager.h"

#include <QTimer>

#include "AboutDialog.h"
#include "AppServices.h"
#include "ConfigManager.h"
#include "HotkeyManager.h"
#include "LogViewerDialog.h"
#include "MainWindow.h"
#include "SettingsDialog.h"
#include "SettingsManager.h"
#include "StyleManager.h"
#include "UpdateDialog.h"
#include "UpdateService.h"
#include "VersionInfo.h"
#include "WorkspaceManager.h"

namespace ModeFlow::Gui {

DialogManager::DialogManager(Core::AppServices& services, QObject* parent) : QObject(parent), m_services(services) {}

void DialogManager::setActiveDialog(Core::ActiveDialog dialog) {
    if (m_activeDialog == dialog)
        return;
    m_activeDialog = dialog;
    emit activeDialogChanged(dialog);
}

QWidget* DialogManager::parentWindow() const {
    return m_services.mainWindow ? m_services.mainWindow.get() : nullptr;
}

bool DialogManager::confirmApplyProfile(const Core::WorkspaceConfig& config) {
    if (!m_services.configManager->askConfirmation()) {
        return true;
    }

    m_services.styleManager->forceUnhover();
    const QString title = tr("Apply Profile");
    const QString text = tr("Apply profile '%1'?").arg(config.name);

    return m_services.styleManager->confirmAction(parentWindow(), title, text);
}

bool DialogManager::confirmAction(const QString& title, const QString& text) {
    m_services.styleManager->forceUnhover();
    return m_services.styleManager->confirmAction(parentWindow(), title, text);
}

void DialogManager::showInfo(const QString& title, const QString& text) {
    m_services.styleManager->showInfo(parentWindow(), title, text);
}

void DialogManager::showWarning(const QString& title, const QString& text) {
    m_services.styleManager->showWarning(parentWindow(), title, text);
}

void DialogManager::showError(const QString& title, const QString& text) {
    m_services.styleManager->showError(parentWindow(), title, text);
}

void DialogManager::showAboutDialog() {
    if (m_activeDialog != Core::ActiveDialog::None)
        return;

    int result = 0;
    {
        m_services.styleManager->forceUnhover();
        DialogGuard guard(this, Core::ActiveDialog::About);

        const bool updateAvailable = m_services.updateService && m_services.updateService->isUpdateAvailable();
        const QString latestVersion = m_services.updateService ? m_services.updateService->latestVersion() : QString();

        AboutDialog dlg(m_services.styleManager.get(), updateAvailable, latestVersion, parentWindow());
        result = dlg.exec();
    }

    if (result == 2) {
        showUpdateDialog();
        QTimer::singleShot(0, this, &DialogManager::showAboutDialog);
    }
}

void DialogManager::showLogViewerDialog() {
    if (m_activeDialog != Core::ActiveDialog::None)
        return;

    m_services.styleManager->forceUnhover();
    DialogGuard guard(this, Core::ActiveDialog::LogViewer);

    LogViewerDialog dlg(m_services.settingsManager.get(), m_services.styleManager.get(), parentWindow());
    dlg.exec();
}

void DialogManager::showSettingsDialog() {
    if (m_activeDialog != Core::ActiveDialog::None)
        return;

    m_services.styleManager->forceUnhover();

    const QString oldLang = m_services.settingsManager->currentLanguage();
    const Core::Theme oldTheme = m_services.settingsManager->currentTheme();

    bool accepted = false;
    {
        DialogGuard guard(this, Core::ActiveDialog::Settings);

        SettingsDialog dlg(m_services.settingsManager.get(), m_services.workspaceManager.get(),
                           m_services.styleManager.get(), parentWindow());
        if (m_services.hotkeyManager) {
            connect(&dlg, &SettingsDialog::hotkeyCaptureChanged, m_services.hotkeyManager.get(),
                    &Services::HotkeyManager::setCaptureMode, Qt::UniqueConnection);
        }
        accepted = (dlg.exec() == QDialog::Accepted);
    }

    if (accepted) {
        emit settingsAccepted(oldLang, oldTheme);
    }
}

void DialogManager::showUpdateDialog() {
    if (m_activeDialog != Core::ActiveDialog::None)
        return;

    if (!m_services.updateService || !m_services.updateService->isUpdateAvailable())
        return;

    m_services.styleManager->forceUnhover();

    const QString version = m_services.updateService->latestVersion();
    const QString changelog = m_services.updateService->changelog();
    const QUrl downloadUrl = m_services.updateService->downloadUrl();

    {
        DialogGuard guard(this, Core::ActiveDialog::Update);

        UpdateDialog dlg(m_services.styleManager.get(), ModeFlow::Info::Version, version, changelog, downloadUrl,
                         parentWindow());
        const int result = dlg.exec();

        if (result == UpdateDialog::SkipVersionResult) {
            m_services.settingsManager->setSkippedVersion(version);
            m_services.settingsManager->saveSettings();

            if (m_services.mainWindow) {
                m_services.mainWindow->setUpdateAvailable(false, {});
            }
        }
    }
}

} // namespace ModeFlow::Gui
