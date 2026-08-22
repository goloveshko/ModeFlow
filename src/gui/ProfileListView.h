#pragma once

#include <QLabel>
#include <QListView>

#include "IStyleManager.h"
#include "IWorkspaceManager.h"

namespace ModeFlow::Gui {

class ProfileListView : public QListView {
    Q_OBJECT
public:
    explicit ProfileListView(QWidget* parent = nullptr);
    ~ProfileListView() override;

    void setWorkspaceManager(Core::IWorkspaceManager* manager);
    void setStyleManager(Core::IStyleManager* sm);

signals:
    void createRequested();
    void deleteRequested(int row);
    void duplicateRequested(int row);
    void applyRequested(int row);

protected:
    void contextMenuEvent(QContextMenuEvent* event) override;
    void changeEvent(QEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

private:
    Core::IWorkspaceManager* m_workspaceManager = nullptr;
    Core::IStyleManager* m_styleManager = nullptr;
};

} // namespace ModeFlow::Gui