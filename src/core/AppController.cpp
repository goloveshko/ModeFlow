#include "AppController.h"

#include <QMessageBox>

#include "AboutDialog.h"
#include "AppLauncher.h"
#include "AudioDeviceManager.h"
#include "CommandLineBuilder.h"
#include "Constants.h"
#include "DisplayManager.h"
#include "HotkeyManager.h"
#include "LocalizationManager.h"
#include "LogViewerDialog.h"
#include "Logging.h"
#include "ServiceFactory.h"
#include "ServiceWiring.h"
#include "SettingsDialog.h"
#include "SettingsManager.h"
#include "StartupPreflightChecker.h"
#include "StyleManager.h"
#include "StyleUtils.h"
#include "SystemUtils.h"
#include "TrayController.h"
#include "UpdateDialog.h"
#include "UpdateService.h"
#include "VersionInfo.h"
#include "WorkspaceManager.h"
#include "WorkspaceService.h"
#include "WorkspaceWindow.h"

namespace ModeFlow::Core {

namespace {
using namespace Qt::StringLiterals;
constexpr int StartupSilentRestartDelayMs = 1200;

struct SetterGuard {
    AppController* controller;
    SetterGuard(AppController* c, ActiveDialog d) : controller(c) { controller->setActiveDialog(d); }
    ~SetterGuard() { controller->setActiveDialog(ActiveDialog::None); }
};
} // namespace

AppController::AppController(QObject* parent) : QObject(parent) {
    ServiceFactory::createCoreServices(m_services);
}

AppController::~AppController() = default;

void AppController::init(const StartupOptions& options) {
    m_options = options;

    qCDebug(lcCore) << "Starting application initialization...";
    qCDebug(lcCore) << "Startup mode:" << (m_options.isLogon ? "logon" : "normal")
                    << "Silent restart:" << m_options.isSilentRestart;

    ServiceWiring::wireErrorConnections(m_services, this);

    if (!m_services.configManager->loadConfig()) {
        qCWarning(lcCore) << "Config load failed, defaults will be used.";
    }

    m_services.locManager->switchLanguage(m_services.configManager->language());

    connect(qApp, &QCoreApplication::aboutToQuit, this, &AppController::onAboutToQuit);

    if (m_options.isLogon && !m_options.isSilentRestart) {
        qCDebug(lcCore) << "Logon startup detected. Starting async preflight.";

        m_preflightChecker = std::make_unique<StartupPreflightChecker>();
        connect(m_preflightChecker.get(), &StartupPreflightChecker::finished, this,
                &AppController::finalizeInitialization, Qt::QueuedConnection);

        m_preflightChecker->startChecking();
    } else {
        finalizeInitialization();
    }
}

void AppController::finalizeInitialization() {
    if (m_isFinalizing) {
        qCWarning(lcCore) << "finalizeInitialization called twice, ignoring";
        return;
    }
    m_isFinalizing = true;

    qCDebug(lcCore) << "Finalizing hardware-dependent services...";

    try {
        ServiceFactory::createHardwareServices(m_services, this);

        m_services.trayController->retranslateUi();

        ServiceWiring::wireServiceConnections(m_services, this);

        qCDebug(lcCore) << "The audio subsystem is ready for use:" << m_services.audioManager->isSystemReady();

        applyStartupConfig();
        profilesChanged();

        const bool wasVisible = m_services.configManager->isMainWindowVisible();
        const bool suppressWindowRestore = m_options.isLogon || m_options.isSilentRestart;

        if (wasVisible && !suppressWindowRestore) {
            qCDebug(lcCore) << "Restoring main window visibility state.";
            raiseMainWindow();
        } else {
            qCDebug(lcCore) << "Keeping app in tray.";
        }

        qCDebug(lcCore) << "AppController initialized successfully.";

        setupAutoUpdateChecking();

    } catch (const std::exception& e) {
        qCCritical(lcCore) << "CRITICAL: Exception during finalization:" << e.what();
    } catch (...) {
        qCCritical(lcCore) << "CRITICAL: Unknown exception during finalization!";
    }
}

void AppController::setupAutoUpdateChecking() {
    if (!m_services.configManager->autoUpdateEnabled() || !m_services.updateService) {
        return;
    }

    connect(m_services.updateService.get(), &Services::UpdateService::updateAvailable, this,
            [this](const QString& version, const QUrl&, const QString&) {
                m_pendingUpdateVersion = version;
                if (m_services.workspaceWindow) {
                    m_services.workspaceWindow->setUpdateAvailable(true, version);
                }
            });

    connect(m_services.updateService.get(), &Services::UpdateService::noUpdateAvailable, this, [this]() {
        m_pendingUpdateVersion.clear();
        if (m_services.workspaceWindow) {
            m_services.workspaceWindow->setUpdateAvailable(false, {});
        }
    });

    if (m_services.updateService->isUpdateAvailable()) {
        m_pendingUpdateVersion = m_services.updateService->latestVersion();
    }

    // Pass false to honor 24-hour cache interval on application startup
    m_services.updateService->checkForUpdates(false);

    m_updateTimer.setInterval(Utils::UpdateCheckIntervalMs);
    m_updateTimer.setSingleShot(false);

    connect(&m_updateTimer, &QTimer::timeout, this, [this]() {
        if (m_services.updateService) {
            m_services.updateService->checkForUpdates(false);
        }
    });

    m_updateTimer.start();
}

void AppController::ensureWorkspaceWindow() {
    if (m_services.workspaceWindow) {
        return;
    }

    ServiceFactory::createWorkspaceWindow(m_services);
    ServiceWiring::wireWindowConnections(m_services, this);

    if (!m_pendingUpdateVersion.isEmpty()) {
        m_services.workspaceWindow->setUpdateAvailable(true, m_pendingUpdateVersion);
    }
}

void AppController::raiseMainWindow() {
    if (!m_services.hotkeyManager) {
        qCWarning(lcCore) << "Called before init(), ignoring";
        return;
    }

    ensureWorkspaceWindow();

    m_services.workspaceWindow->raiseWindow();
}

void AppController::handleSettingsChanges(const QString& oldLang, Core::Theme oldTheme) {
    m_services.workspaceService->setAudioConfirmation(m_services.configManager->audioConfirmation());
    m_services.hotkeyManager->setNextProfileHotkey(m_services.configManager->nextProfileHotkey());

    if (m_services.configManager->language() != oldLang) {
        m_services.locManager->switchLanguage(m_services.configManager->language());
    }

    processThemeChange(oldTheme);
}

void AppController::processThemeChange(Core::Theme oldTheme) {
    const Core::Theme newTheme = m_services.configManager->currentTheme();
    const QString newStyleKey = m_services.configManager->currentQtStyleKey();

    const bool needsRecreation =
        (oldTheme == Theme::Qt && newTheme != Theme::Qt) || (oldTheme != Theme::Qt && newTheme == Theme::Qt);

    if (needsRecreation) {
        const bool wasVisible = m_services.workspaceWindow && m_services.workspaceWindow->isVisible();

        if (m_services.workspaceWindow && wasVisible) {
            m_services.workspaceWindow->hide();
        }

        QTimer::singleShot(0, this, [this, wasVisible, newTheme, newStyleKey]() {
            m_services.styleManager->setTheme(newTheme, newStyleKey);

            m_services.workspaceWindow.reset();

            if (wasVisible) {
                raiseMainWindow();
            }
        });
    } else {
        if (m_services.workspaceWindow && m_services.workspaceWindow->isVisible()) {
            Gui::StyleUtils::safeThemeApply(m_services.workspaceWindow.get(), [this, newTheme, newStyleKey]() {
                m_services.styleManager->setTheme(newTheme, newStyleKey);
                m_services.styleManager->applyToWindow(m_services.workspaceWindow.get());
            });
        } else {
            m_services.styleManager->setTheme(newTheme, newStyleKey);
        }
    }

    if (m_services.trayController) {
        m_services.trayController->retranslateUi();
    }
}

void AppController::showAboutDialog() {
    if (m_activeDialog != ActiveDialog::None)
        return;

    m_services.styleManager->forceUnhover();
    SetterGuard guard(this, ActiveDialog::About);

    const bool updateAvailable = m_services.updateService && m_services.updateService->isUpdateAvailable();
    const QString latestVersion = m_services.updateService ? m_services.updateService->latestVersion() : QString();

    Gui::AboutDialog dlg(m_services.styleManager.get(), updateAvailable, latestVersion, parentWindow());
    const int result = dlg.exec();

    if (result == 2) {
        showUpdateDialog();
        QTimer::singleShot(0, this, &AppController::showAboutDialog);
    }
}

void AppController::showLogViewerDialog() {
    if (m_activeDialog != ActiveDialog::None)
        return;

    m_services.styleManager->forceUnhover();
    SetterGuard guard(this, ActiveDialog::LogViewer);

    Gui::LogViewerDialog dlg(m_services.settingsManager.get(), m_services.styleManager.get(), parentWindow());
    dlg.exec();
}

void AppController::showSettingsDialog() {
    if (m_activeDialog != ActiveDialog::None)
        return;

    m_services.styleManager->forceUnhover();

    SetterGuard guard(this, ActiveDialog::Settings);

    const QString oldLang = m_services.configManager->language();
    const Theme oldTheme = m_services.styleManager->currentTheme();

    bool accepted = false;

    {
        Gui::SettingsDialog dlg(m_services.settingsManager.get(), m_services.workspaceManager.get(),
                                m_services.styleManager.get(), parentWindow());
        connect(&dlg, &Gui::SettingsDialog::hotkeyCaptureChanged, m_services.hotkeyManager.get(),
                &Services::HotkeyManager::setCaptureMode, Qt::UniqueConnection);
        accepted = (dlg.exec() == QDialog::Accepted);
    }

    if (accepted) {
        handleSettingsChanges(oldLang, oldTheme);
    }
}

void AppController::showUpdateDialog() {
    if (m_activeDialog != ActiveDialog::None && m_activeDialog != ActiveDialog::About)
        return;

    if (!m_services.updateService || !m_services.updateService->isUpdateAvailable())
        return;

    m_services.styleManager->forceUnhover();

    SetterGuard guard(this, ActiveDialog::About);

    const QString version = m_services.updateService->latestVersion();
    const QString changelog = m_services.updateService->changelog();
    const QUrl downloadUrl = m_services.updateService->downloadUrl();
    const QString currentVersion = Info::Version;

    Gui::UpdateDialog dlg(m_services.styleManager.get(), currentVersion, version, changelog, downloadUrl,
                          parentWindow());
    const int result = dlg.exec();

    if (result == Gui::UpdateDialog::SkipVersionResult) {
        qCDebug(lcCore) << "User chose to skip update version:" << version;
        m_services.settingsManager->setSkippedVersion(version);
        m_services.settingsManager->saveSettings();
        if (m_services.workspaceWindow) {
            m_services.workspaceWindow->setUpdateAvailable(false, {});
        }
    }
}

QWidget* AppController::parentWindow() const {
    return m_services.workspaceWindow ? m_services.workspaceWindow.get() : nullptr;
}

void AppController::applyStartupConfig() {
    const auto& configs = m_services.workspaceManager->configs();
    if (configs.isEmpty())
        return;

    const auto action = m_services.configManager->startupAction();
    if (action == StartupAction::None)
        return;

    QString targetId;
    if (action == StartupAction::LastActive) {
        targetId = m_services.configManager->lastActiveProfileId();
    } else if (action == StartupAction::Specific) {
        targetId = m_services.configManager->startupProfileId();
    }

    if (targetId.isEmpty())
        return;

    WorkspaceConfig targetConfig;
    for (const auto& cfg : configs) {
        if (cfg.id == targetId) {
            targetConfig = cfg;
            break;
        }
    }

    if (targetConfig.id.isEmpty()) {
        targetConfig = configs.first();
        qCWarning(lcCore) << "Target profile not found:" << targetId;
    }

    QString currentDisplay = m_services.displayManager->getCurrentDisplayKey();
    bool displaySwitchWillHappen = (!targetConfig.displayId.isEmpty() && targetConfig.displayId != currentDisplay);

    if (displaySwitchWillHappen && m_options.isLogon) {
        m_restartPending = true;
        qCDebug(lcCore) << "Startup profile requires display switch during logon. Silent restart will follow.";

        connect(m_services.workspaceService.get(), &WorkspaceService::configApplyFinished, this,
                [this](const WorkspaceConfig&, WorkspaceService::ApplyStatus status) {
                    if (status == WorkspaceService::ApplyStatus::Success) {
                        handleStartupSuccess();
                    } else {
                        handleStartupError();
                    }
                });
    }

    m_services.workspaceService->applyWorkspaceConfig(targetConfig);
}

void AppController::handleStartupSuccess() {
    if (m_restartPending) {
        m_restartPending = false;

        qCDebug(lcCore) << "Display switch confirmed during logon startup. Restarting application in a fresh process "
                           "to rebuild Qt screen state after monitor reconfiguration. Delay ="
                        << StartupSilentRestartDelayMs << "ms.";

        QTimer::singleShot(StartupSilentRestartDelayMs, this, [this]() { restartApp(true); });
    }
}

void AppController::handleStartupError() {
    m_restartPending = false;
}

ActiveDialog AppController::getActiveDialog() const {
    return m_activeDialog;
}

void AppController::setActiveDialog(ActiveDialog activeDialog) {
    if (m_activeDialog == activeDialog)
        return;
    m_activeDialog = activeDialog;
    emit activeDialogChanged(activeDialog);
}

void AppController::profilesChanged() {
    const auto configs = m_services.workspaceManager->configs();
    const auto nextHotkey = m_services.settingsManager->nextProfileHotkey();

    // Query both updates and track if any actual system-wide hook mutations occurred
    const bool profilesHotkeyChanged = m_services.hotkeyManager->setProfiles(configs);
    const bool nextHotkeyChanged = m_services.hotkeyManager->setNextProfileHotkey(nextHotkey);

    // Only output the log if there was an actual mutation of system keyboard hooks
    if (profilesHotkeyChanged || nextHotkeyChanged) {
        qCDebug(lcCore) << "Hotkeys updated from MainWindow signal";
    }
}

void AppController::forceUpdateCheck() {
    if (m_activeDialog != ActiveDialog::None && m_activeDialog != ActiveDialog::About)
        return;

    if (!m_services.updateService || m_services.updateService->isCheckingInProgress())
        return;

    // Disconnect and clear any lingering handles from previous manual checks
    for (const auto& conn : m_manualUpdateConns) {
        QObject::disconnect(conn);
    }
    m_manualUpdateConns.clear();

    // Helper to immediately unhook all manual check connections as soon as ANY outcome triggers
    auto cleanupConns = [this]() {
        for (const auto& conn : m_manualUpdateConns) {
            QObject::disconnect(conn);
        }
        m_manualUpdateConns.clear();
    };

    const auto c1 = QObject::connect(m_services.updateService.get(), &Services::UpdateService::updateAvailable, this,
                                     [this, cleanupConns](const QString&, const QUrl&, const QString&) {
                                         cleanupConns();
                                         showUpdateDialog();
                                     });

    const auto c2 = QObject::connect(
        m_services.updateService.get(), &Services::UpdateService::noUpdateAvailable, this, [this, cleanupConns]() {
            cleanupConns();
            if (m_services.workspaceWindow) {
                m_services.workspaceWindow->showToolTipOnMoreButton(tr("You are up to date."));
            }
        });

    const auto c3 = QObject::connect(m_services.updateService.get(), &Services::UpdateService::checkFailed, this,
                                     [this, cleanupConns](const QString& error) {
                                         cleanupConns();
                                         if (m_services.workspaceWindow) {
                                             m_services.workspaceWindow->showToolTipOnMoreButton(
                                                 tr("Update check failed") + u": "_s + error);
                                         }
                                     });

    m_manualUpdateConns = {c1, c2, c3};

    // Force network check on manual trigger
    m_services.updateService->checkForUpdates(true);
}

void AppController::requestAppExit() {
    qCDebug(lcCore) << "Explicit exit requested by user.";
    qApp->quit();
}

void AppController::restartApp(bool silentRestart) {
    qCDebug(lcCore) << "Restarting application. silentRestart =" << silentRestart;

    emit aboutToRestart();

    CommandLineBuilder builder;
    builder.withLog(m_options.enableLogging)
        .withLogon(m_options.isLogon || silentRestart)
        .withSilentRestart(silentRestart);

    const QStringList args = builder.toStringList();
    qCDebug(lcCore) << "Restart args:" << args;

    if (Utils::SystemUtils::restartApplication(args)) {
        requestAppExit();
    } else {
        qCCritical(lcCore) << "Failed to restart application!";
    }
}

void AppController::showNonBlockingError(const QString& message) {
    if (message.isEmpty()) {
        return;
    }

    qCWarning(lcCore) << "User-visible error:" << message;

    if (m_services.trayController) {
        m_services.trayController->showNotification(tr("ModeFlow"), message);
    }
}

void AppController::confirmAndApplyProfile(const WorkspaceConfig& config) {
    m_services.styleManager->forceUnhover();

    bool confirmed = true;

    if (m_services.configManager->askConfirmation()) {
        const QString title = tr("Apply Profile");
        const QString text = tr("Apply profile '%1'?").arg(config.name);

        confirmed = m_services.styleManager->confirmAction(parentWindow(), title, text);
    }

    if (confirmed) {
        m_services.workspaceService->applyWorkspaceConfig(config);
    }
}

void AppController::onDefaultAudioDeviceChanged(const QString& id) {
    qCDebug(lcCore) << "System default audio device changed to:"
                    << m_services.audioManager->getDeviceById(id).displayName;
}

void AppController::onAboutToQuit() {
    qCDebug(lcCore) << "Application is quitting. Flushing configuration state to disk...";

    // Write the synchronized in-memory config state to physical disk
    m_services.configManager->saveConfig();
}

} // namespace ModeFlow::Core
