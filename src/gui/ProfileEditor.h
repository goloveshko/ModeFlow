#pragma once

#include "ConfigTypes.h"

class QLineEdit;
class QCheckBox;
class QComboBox;
class QGroupBox;
class QPushButton;
class QAction;

namespace ModeFlow::Core {
class IWorkspaceManager;
class ISettingsManager;
} // namespace ModeFlow::Core

namespace ModeFlow::Gui {

class HotkeyEdit;
class AppListWidget;
class ProfileIconMenu;

/**
 * @brief Aggregate struct of pointers to the profile details widgets.
 * Allows clean decoupling of the details controller from the main window's private UI structure.
 */
struct ProfileEditorWidgets {
    QLineEdit* editName;
    HotkeyEdit* keyEditSpecific;
    QCheckBox* checkSkipInCycle;
    QComboBox* comboDisplay;
    QComboBox* comboAudio;
    AppListWidget* listApps;
    QPushButton* btnCapture;
    QGroupBox* groupGeneral;
    QGroupBox* groupHardware;
    QGroupBox* groupAuto;
};

/**
 * @brief Controller responsible for managing the right-side profile details form.
 * Encapsulates device combo-box loading, form data mapping, autosave triggers, and icon suggestions.
 */
class ProfileEditor : public QObject {
    Q_OBJECT
public:
    ProfileEditor(const ProfileEditorWidgets& widgets, Core::IWorkspaceManager* wm, Core::ISettingsManager* sm,
                  ProfileIconMenu* iconMenu, QObject* parent = nullptr);

    void loadProfile(const Core::WorkspaceConfig& cfg);
    void saveProfile(Core::WorkspaceConfig& cfg);
    void updateUI(bool hasProfiles);
    void captureCurrentSettings();

    void refreshVisualState();
    void setCurrentProfileIconSymbol(const QString& symbol);
    QString currentProfileIconSymbol() const { return m_iconSymbol; }

signals:
    void profileChanged();
    void hotkeyCaptureChanged(bool active);
    void validateSpecificHotkey();

private:
    void setupConnections();
    void updateMonitorDevices(const QString& savedDisplayId);
    void updateAudioDevices(const QString& savedAudioId);
    void populateDeviceCombo(QComboBox* combo, const QList<Core::DeviceEntry>& devices, const QString& currentId);
    void updateProfileIconButton();

    ProfileEditorWidgets m_widgets;
    Core::IWorkspaceManager* m_workspaceManager;
    Core::ISettingsManager* m_settingsManager;
    ProfileIconMenu* m_profileIconMenu;

    QAction* m_iconAction = nullptr;
    QString m_iconSymbol;
    bool m_iconIsManual = false;
    bool m_isUpdating = false;
};

} // namespace ModeFlow::Gui