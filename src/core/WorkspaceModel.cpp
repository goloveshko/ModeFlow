#include "WorkspaceModel.h"

#include "FontAwesome.h"

namespace ModeFlow::Core {

WorkspaceModel::WorkspaceModel(QObject* parent) : QAbstractListModel(parent) {}

int WorkspaceModel::rowCount(const QModelIndex& parent) const {
    return m_configs.size();
}

QVariant WorkspaceModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || index.row() >= m_configs.size())
        return {};

    const auto& cfg = m_configs[index.row()];
    if (role == Qt::DisplayRole)
        return cfg.name;
    if (role == Qt::DecorationRole) {
        const QString iconSymbol =
            cfg.iconSymbol.isEmpty() ? Gui::FontAwesome::defaultProfileIconSymbol() : cfg.iconSymbol;
        return iconSymbol;
    }
    if (role == ConfigRole)
        return QVariant::fromValue(cfg);

    if (role == ActiveRole) {
        if (m_activeIdGetter) {
            return cfg.id == m_activeIdGetter();
        }
        return false;
    }
    return {};
}

void WorkspaceModel::setConfigs(const QList<WorkspaceConfig>& configs) {
    beginResetModel();
    m_configs = configs;
    endResetModel();
}

void WorkspaceModel::addConfig(const WorkspaceConfig& cfg) {
    beginInsertRows(QModelIndex(), m_configs.size(), m_configs.size());
    m_configs.append(cfg);
    endInsertRows();
}

void WorkspaceModel::removeConfig(int row) {
    beginRemoveRows(QModelIndex(), row, row);
    m_configs.removeAt(row);
    endRemoveRows();
}

void WorkspaceModel::updateConfig(int row, const WorkspaceConfig& cfg) {
    if (row < 0 || row >= m_configs.size())
        return;
    m_configs[row] = cfg;
    emit dataChanged(index(row), index(row));
}

const QList<WorkspaceConfig>& WorkspaceModel::configs() const {
    return m_configs;
}

int WorkspaceModel::rowOfId(const QString& id) const {
    if (id.isEmpty())
        return -1;
    for (int i = 0; i < m_configs.size(); ++i) {
        if (m_configs[i].id == id)
            return i;
    }
    return -1;
}

} // namespace ModeFlow::Core
