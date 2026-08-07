#pragma once

#include <QObject>

#include "ConfigTypes.h"

class QWidget;

namespace ModeFlow::Core {
struct AppServices;
}

namespace ModeFlow::Gui {

/**
 * @brief Manages top-level modal dialog lifecycles, UI prompts, and active dialog state.
 */
class DialogManager : public QObject {
    Q_OBJECT
    Q_PROPERTY(Core::ActiveDialog activeDialog READ activeDialog WRITE setActiveDialog NOTIFY activeDialogChanged)

public:
    explicit DialogManager(Core::AppServices& services, QObject* parent = nullptr);

    Core::ActiveDialog activeDialog() const { return m_activeDialog; }
    void setActiveDialog(Core::ActiveDialog dialog);

    QWidget* parentWindow() const;

    // UI Confirmation & Alert Helpers
    bool confirmApplyProfile(const Core::WorkspaceConfig& config);
    bool confirmAction(const QString& title, const QString& text);
    void showInfo(const QString& title, const QString& text);
    void showWarning(const QString& title, const QString& text);
    void showError(const QString& title, const QString& text);

public slots:
    void showAboutDialog();
    void showLogViewerDialog();
    void showSettingsDialog();
    void showUpdateDialog();

signals:
    void activeDialogChanged(ModeFlow::Core::ActiveDialog activeDialog);
    void settingsAccepted(const QString& oldLang, ModeFlow::Core::Theme oldTheme);

private:
    struct DialogGuard {
        DialogManager* manager;
        DialogGuard(DialogManager* m, Core::ActiveDialog d) : manager(m) { manager->setActiveDialog(d); }
        ~DialogGuard() { manager->setActiveDialog(Core::ActiveDialog::None); }
    };

    Core::AppServices& m_services;
    Core::ActiveDialog m_activeDialog = Core::ActiveDialog::None;
};

} // namespace ModeFlow::Gui
