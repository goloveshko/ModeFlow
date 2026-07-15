#pragma once

#include <QLabel>
#include <QListWidget>

#include "ConfigTypes.h"
#include "IStyleManager.h"

namespace ModeFlow::Gui {

class AppListWidget : public QListWidget {
    Q_OBJECT
public:
    explicit AppListWidget(QWidget* parent = nullptr);
    ~AppListWidget() override;

    void setStyleManager(Core::IStyleManager* sm) { m_styleManager = sm; }

    void setApps(const QList<Core::AppLaunchConfig>& apps);
    QList<Core::AppLaunchConfig> apps() const { return m_apps; }

signals:
    void appsChanged();

protected:
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dragMoveEvent(QDragMoveEvent* event) override;
    void dropEvent(QDropEvent* event) override;
    void contextMenuEvent(QContextMenuEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void changeEvent(QEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

private:
    void refreshList();
    void openEditDialog(int editIndex);
    void addAppToSequenceDirectly(const QString& path);
    void removeApp(int index);
    void appendAppItem(int index, const Core::AppLaunchConfig& app);
    void appendAddPlaceholderItem();

private:
    Core::IStyleManager* m_styleManager = nullptr;
    QList<Core::AppLaunchConfig> m_apps;
};

} // namespace ModeFlow::Gui