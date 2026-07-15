#pragma once

#include <QMenu>
#include <QSystemTrayIcon>

#include "ConfigTypes.h"

namespace ModeFlow::Core {
class IWorkspaceManager;
} // namespace ModeFlow::Core

namespace ModeFlow::Gui {

class TrayController : public QObject {
    Q_OBJECT

public:
    explicit TrayController(Core::IWorkspaceManager* workspaceManager, QObject* parent = nullptr);
    ~TrayController() override;

    void showNotification(const QString& title, const QString& message);

public slots:
    void retranslateUi();
    void activeDialogChanged(Core::ActiveDialog activeDialog);

private slots:
    void rebuildMenu();
    void onMenuAboutToShow();
    void populateProfileSubmenu();
    void populateMonitorSubmenu();
    void populateAudioSubmenu();

private:
    void setEnabledAction(const QString& actionStr, bool enabled);
    void populateDeviceSubmenu(QMenu* menu, const QList<Core::DeviceEntry>& devices, const QString& currentId,
                               std::function<void(const QString&)> emitFn);

private:
    std::unique_ptr<QSystemTrayIcon> m_trayIcon;
    std::unique_ptr<QMenu> m_trayMenu;

    std::unique_ptr<QMenu> m_profileMenu;
    std::unique_ptr<QMenu> m_monitorMenu;
    std::unique_ptr<QMenu> m_audioMenu;

    Core::IWorkspaceManager* m_workspaceManager;
    Core::ActiveDialog m_activeDialog = Core::ActiveDialog::None;

signals:
    void showMainWindow();
    void menuAboutToShow();
    void showSettingsDialog();
    void activateProfile(const Core::WorkspaceConfig& config);
    void switchDisplay(const QString& displayId);
    void switchAudio(const QString& audioId);
    void signalExitRequested();
};
} // namespace ModeFlow::Gui
