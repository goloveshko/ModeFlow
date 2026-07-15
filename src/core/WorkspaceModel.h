#pragma once

#include <QAbstractListModel>

#include "ConfigTypes.h"

namespace ModeFlow::Core {

class WorkspaceModel : public QAbstractListModel {
    Q_OBJECT
public:
    enum Roles { ConfigRole = Qt::UserRole + 1, ActiveRole = Qt::UserRole + 2 };

    explicit WorkspaceModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

    void setConfigs(const QList<WorkspaceConfig>& configs);

    void addConfig(const WorkspaceConfig& cfg);

    void removeConfig(int row);

    void updateConfig(int row, const WorkspaceConfig& cfg);

    const QList<WorkspaceConfig>& configs() const;

    int rowOfId(const QString& id) const;

    // Decoupled getter to query the currently active profile ID
    using ActiveIdGetter = std::function<QString()>;
    void setActiveIdGetter(ActiveIdGetter getter) { m_activeIdGetter = getter; }

private:
    QList<WorkspaceConfig> m_configs;
    ActiveIdGetter m_activeIdGetter;
};
} // namespace ModeFlow::Core
