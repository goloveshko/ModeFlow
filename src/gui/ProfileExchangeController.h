#pragma once

#include <QObject>

namespace ModeFlow::Core {
class IWorkspaceManager;
}

namespace ModeFlow::Gui {

class DialogManager;

/**
 * @brief Controller responsible for managing profile import/export operations.
 * Uses DialogManager as a unified UI facade for file dialogs and status alerts.
 */
class ProfileExchangeController : public QObject {
    Q_OBJECT
public:
    explicit ProfileExchangeController(Core::IWorkspaceManager* wm, DialogManager* dialogManager,
                                       QObject* parent = nullptr);
public slots:
    void doImport();
    void doExport();

signals:
    void exchangeCompleted();

private:
    Core::IWorkspaceManager* m_workspaceManager;
    DialogManager* m_dialogManager;
};

} // namespace ModeFlow::Gui
