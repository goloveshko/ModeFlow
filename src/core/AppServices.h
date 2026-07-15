#pragma once

#include <memory>

namespace ModeFlow::Services {
class AudioDeviceManager;
class AudioFeedbackService;
class AppLauncher;
class DisplayManager;
class HotkeyManager;
class StyleManager;
class UpdateService;
class WindowsAutostartManager;
} // namespace ModeFlow::Services

namespace ModeFlow::Gui {
class WorkspaceWindow;
class TrayController;
} // namespace ModeFlow::Gui

namespace ModeFlow::Core {

class ConfigManager;
class LocalizationManager;
class WorkspaceService;
class WorkspaceManagerImpl;
class SettingsManagerImpl;

struct AppServices {
    std::unique_ptr<ConfigManager> configManager;
    std::unique_ptr<LocalizationManager> locManager;
    std::unique_ptr<Services::AppLauncher> appLauncher;

    std::unique_ptr<Services::DisplayManager> displayManager;
    std::unique_ptr<Services::AudioDeviceManager> audioManager;
    std::unique_ptr<Services::WindowsAutostartManager> autostartManager;
    std::unique_ptr<Services::AudioFeedbackService> audioFeedback;

    std::unique_ptr<Services::HotkeyManager> hotkeyManager;
    std::unique_ptr<Services::UpdateService> updateService;
    std::unique_ptr<WorkspaceService> workspaceService;
    std::unique_ptr<Services::StyleManager> styleManager;

    std::unique_ptr<WorkspaceManagerImpl> workspaceImpl;
    std::unique_ptr<SettingsManagerImpl> settingsImpl;

    std::unique_ptr<Gui::WorkspaceWindow> workspaceWindow;
    std::unique_ptr<Gui::TrayController> trayController;
};

} // namespace ModeFlow::Core
