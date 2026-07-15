#include "ProfileExchangeController.h"

#include "IStyleManager.h"
#include "IWorkspaceManager.h"
#include "ProfileSerializer.h"

namespace ModeFlow::Gui {

using namespace Qt::StringLiterals;

ProfileExchangeController::ProfileExchangeController(Core::IWorkspaceManager* wm, Core::IStyleManager* sm,
                                                     QWidget* parentWindow)
    : QObject(parentWindow), m_workspaceManager(wm), m_styleManager(sm), m_parentWindow(parentWindow) {}

void ProfileExchangeController::doImport() {
    QString filePath = m_styleManager->getOpenFileName(m_parentWindow, tr("Import Profiles"), QString(),
                                                       tr("JSON files (*.json);;All files (*)"));
    if (filePath.isEmpty()) {
        return;
    }

    QString error;
    auto imported = Utils::ProfileSerializer::importProfiles(filePath, error);

    if (!error.isEmpty()) {
        m_styleManager->showWarning(m_parentWindow, tr("Import Failed"), error);
        return;
    }

    if (imported.isEmpty()) {
        m_styleManager->showInfo(m_parentWindow, tr("Import"), tr("No profiles found in the file."));
        return;
    }

    auto existing = m_workspaceManager->getAllWorkspaceConfigs();
    int addedCount = 0;
    int skippedCount = 0;

    for (const auto& cfg : imported) {
        bool isDuplicate = false;
        for (const auto& ex : existing) {
            if (ex.id == cfg.id) {
                isDuplicate = true;
                break;
            }
        }

        if (isDuplicate) {
            skippedCount++;
            continue;
        }

        m_workspaceManager->addConfig(cfg);
        addedCount++;
    }

    if (addedCount > 0) {
        m_workspaceManager->saveWorkspaces();
        emit exchangeCompleted();
    }

    QString msg = tr("Imported %1 profile(s).").arg(addedCount);
    if (skippedCount > 0) {
        msg += u"\n"_s + tr("%1 duplicate(s) were skipped.").arg(skippedCount);
    }
    m_styleManager->showInfo(m_parentWindow, tr("Import Successful"), msg);
}

void ProfileExchangeController::doExport() {
    QString filePath = m_styleManager->getSaveFileName(m_parentWindow, tr("Export Profiles"), u"profiles.json"_s,
                                                       tr("JSON files (*.json);;All files (*)"));
    if (filePath.isEmpty()) {
        return;
    }

    auto configs = m_workspaceManager->getAllWorkspaceConfigs();
    if (Utils::ProfileSerializer::exportProfiles(configs, filePath)) {
        m_styleManager->showInfo(m_parentWindow, tr("Export Successful"),
                                 tr("Exported %1 profile(s) to:\n%2").arg(configs.size()).arg(filePath));
    } else {
        m_styleManager->showWarning(m_parentWindow, tr("Export Failed"),
                                    tr("Could not export profiles to:\n%1").arg(filePath));
    }
}

} // namespace ModeFlow::Gui