#pragma once

#include "ConfigTypes.h"

class QAbstractItemModel;

namespace ModeFlow::Core {

class WorkspaceModel;

/**
 * @brief Interface for configuration provider.
 *
 * This interface abstracts access to application configuration,
 * allowing MainWindow to work with any configuration backend
 * without depending on the concrete ConfigManager implementation.
 *
 * This follows the Dependency Inversion Principle (DIP):
 * - High-level modules (MainWindow) should not depend on low-level modules (ConfigManager)
 * - Both should depend on abstractions (IConfigManager)
 */
class IWorkspaceManager {
public:
    virtual ~IWorkspaceManager() = default;

    virtual QAbstractItemModel* model() const = 0;

    virtual void addConfig(const WorkspaceConfig& cfg) = 0;
    virtual void removeConfig(int row) = 0;
    virtual void updateConfig(int row, const WorkspaceConfig& cfg) = 0;

    virtual WorkspaceConfig captureCurrentHardwareState() const = 0;

    virtual bool saveWorkspaces() = 0;

    virtual QList<WorkspaceConfig> configs() const = 0;

    virtual void setSelectedRow(int row) = 0;
    virtual int selectedRow() const = 0;
    virtual int activeRow() const = 0;

    virtual QList<DeviceEntry> getAvailableDisplays() const = 0;
    virtual QList<DeviceEntry> getAvailableAudioOutputs() const = 0;

    virtual QString generateDefaultName() = 0;
    virtual void createDefaultProfile() = 0;
    virtual void duplicateProfile(int row) = 0;
    virtual QString suggestedProfileIconSymbol(const QString& profileName) const = 0;
};
} // namespace ModeFlow::Core

Q_DECLARE_INTERFACE(ModeFlow::Core::IWorkspaceManager, "com.ModeFlow.IWorkspaceManager");
