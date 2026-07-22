#pragma once

#include "IWorkspaceManager.h"

namespace ModeFlow::Core {
class ConfigManager;
}
namespace ModeFlow::Services {
class DisplayManager;
class AudioDeviceManager;
} // namespace ModeFlow::Services

namespace ModeFlow::Core {

class WorkspaceManager : public IWorkspaceManager {
public:
    WorkspaceManager(ConfigManager* cm, Services::DisplayManager* dm, Services::AudioDeviceManager* am);
    ~WorkspaceManager() override;

    WorkspaceModel* model() const override;
    void addConfig(const WorkspaceConfig& cfg) override;
    void removeConfig(int row) override;
    void updateConfig(int row, const WorkspaceConfig& cfg) override;
    WorkspaceConfig captureCurrentHardwareState() const override;
    QList<WorkspaceConfig> getAllWorkspaceConfigs() const override;
    bool saveWorkspaces() override;
    void setSelectedRow(int row) override;
    int selectedRow() const override;
    int activeRow() const override;
    QList<DeviceEntry> getAvailableDisplays() const override;
    QList<DeviceEntry> getAvailableAudioOutputs() const override;

    QString generateDefaultName() override;
    void createDefaultProfile() override;
    void duplicateProfile(int row) override;
    QString suggestedProfileIconSymbol(const QString& profileName) const override;

private:
    ConfigManager* m_config;
    Services::DisplayManager* m_display;
    Services::AudioDeviceManager* m_audio;

    std::unique_ptr<WorkspaceModel> m_model;
};

} // namespace ModeFlow::Core
