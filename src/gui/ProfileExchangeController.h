#pragma once

#include <QObject>

namespace ModeFlow::Core {
class IWorkspaceManager;
class IStyleManager;
} // namespace ModeFlow::Core

namespace ModeFlow::Gui {

/**
 * @brief Controller responsible for managing profile import/export operations.
 * Fully encapsulates file dialog interactions, duplicate checking, and user notifications.
 */
class ProfileExchangeController : public QObject {
    Q_OBJECT
public:
    explicit ProfileExchangeController(Core::IWorkspaceManager* wm, Core::IStyleManager* sm, QWidget* parentWindow);

public slots:
    void doImport();
    void doExport();

signals:
    void exchangeCompleted();

private:
    Core::IWorkspaceManager* m_workspaceManager;
    Core::IStyleManager* m_styleManager;
    QWidget* m_parentWindow;
};

} // namespace ModeFlow::Gui