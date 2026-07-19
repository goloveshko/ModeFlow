#include "ServiceFactory.h"

#include "AppLauncher.h"
#include "AudioDeviceManager.h"
#include "AudioFeedbackService.h"
#include "ConfigManager.h"
#include "DisplayManager.h"
#include "HotkeyManager.h"
#include "LocalizationManager.h"
#include "SettingsManagerImpl.h"
#include "StyleManager.h"
#include "TrayController.h"
#include "UpdateService.h"
#include "WindowsAutostartManager.h"
#include "WorkspaceManagerImpl.h"
#include "WorkspaceService.h"
#include "WorkspaceWindow.h"

namespace ModeFlow::Core {

void ServiceFactory::createCoreServices(AppServices& services) {
    services.configManager = std::make_unique<ConfigManager>();
    services.locManager = std::make_unique<LocalizationManager>();
    services.appLauncher = std::make_unique<Services::AppLauncher>();
}

void ServiceFactory::createHardwareServices(AppServices& services, QObject* parent) {
    services.displayManager = std::make_unique<Services::DisplayManager>();
    services.audioManager = std::make_unique<Services::AudioDeviceManager>();
    services.autostartManager = std::make_unique<Services::WindowsAutostartManager>();
    services.audioFeedback = std::make_unique<Services::AudioFeedbackService>();

    services.styleManager = std::make_unique<Services::StyleManager>(services.configManager->currentTheme(),
                                                                     services.configManager->currentQtStyleKey());

    services.workspaceService = std::make_unique<WorkspaceService>(
        services.displayManager.get(), services.audioManager.get(), services.appLauncher.get());

    services.workspaceImpl = std::make_unique<WorkspaceManagerImpl>(
        services.configManager.get(), services.displayManager.get(), services.audioManager.get());

    services.trayController =
        std::make_unique<Gui::TrayController>(services.workspaceImpl.get());

    services.settingsImpl =
        std::make_unique<SettingsManagerImpl>(services.configManager.get(), services.autostartManager.get(),
                                              services.locManager.get(), services.styleManager.get());

    services.hotkeyManager = std::make_unique<Services::HotkeyManager>(
        services.configManager.get(), services.displayManager.get(), services.audioManager.get());

    services.updateService = std::make_unique<Services::UpdateService>(services.configManager.get());
}

void ServiceFactory::createWorkspaceWindow(AppServices& services) {
    services.workspaceWindow = std::make_unique<Gui::WorkspaceWindow>(
        services.workspaceImpl.get(), services.settingsImpl.get(), services.styleManager.get());
}

} // namespace ModeFlow::Core
