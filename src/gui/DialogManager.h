#pragma once

#include <QObject>

#include "ConfigTypes.h"

class QWidget;

namespace ModeFlow::Core {
struct AppServices;
}

namespace ModeFlow::Gui {

/**
 * @brief Unified Facade and Presenter for all application dialogs, alerts, and file pickers.
 */
class DialogManager : public QObject {
    Q_OBJECT
    Q_PROPERTY(Core::ActiveDialog activeDialog READ activeDialog WRITE setActiveDialog NOTIFY activeDialogChanged)

public:
    explicit DialogManager(Core::AppServices& services, QObject* parent = nullptr);

    Core::ActiveDialog activeDialog() const { return m_activeDialog; }
    void setActiveDialog(Core::ActiveDialog dialog);

    QWidget* parentWindow() const;

    // --- 1. Top-Level Modal App Windows ---
    void showAboutDialog();
    void showLogViewerDialog();
    void showSettingsDialog();
    void showUpdateDialog();

    // --- 2. Action Confirmations ---
    bool confirmApplyProfile(const Core::WorkspaceConfig& config);
    bool confirmAction(const QString& title, const QString& text);
    bool confirmAction(QWidget* parent, const QString& title, const QString& text);

    // --- 3. Message Box Alerts (Overloaded: 2-arg auto-parent, 3-arg explicit parent first) ---
    void showInfo(const QString& title, const QString& text);
    void showInfo(QWidget* parent, const QString& title, const QString& text);

    void showWarning(const QString& title, const QString& text);
    void showWarning(QWidget* parent, const QString& title, const QString& text);

    void showError(const QString& title, const QString& text);
    void showError(QWidget* parent, const QString& title, const QString& text);

    // --- 4. File Dialog Pickers ---
    QString getOpenFileName(const QString& caption, const QString& dir = QString(), const QString& filter = QString());
    QString getOpenFileName(QWidget* parent, const QString& caption, const QString& dir = QString(),
                            const QString& filter = QString());

    QString getSaveFileName(const QString& caption, const QString& dir = QString(), const QString& filter = QString());
    QString getSaveFileName(QWidget* parent, const QString& caption, const QString& dir = QString(),
                            const QString& filter = QString());

    // --- 5. App Launch Configuration Dialog ---
    std::optional<Core::AppLaunchConfig> showAppLaunchDialog(const Core::AppLaunchConfig* initialConfig = nullptr,
                                                             QWidget* parent = nullptr);

signals:
    void activeDialogChanged(ModeFlow::Core::ActiveDialog activeDialog);
    void settingsAccepted(const QString& oldLang, ModeFlow::Core::Theme oldTheme);

private:
    struct DialogGuard {
        DialogManager* manager;
        DialogGuard(DialogManager* m, Core::ActiveDialog d) : manager(m) { manager->setActiveDialog(d); }
        ~DialogGuard() { manager->setActiveDialog(Core::ActiveDialog::None); }
    };

    QWidget* resolveParent(QWidget* parent) const;

    Core::AppServices& m_services;
    Core::ActiveDialog m_activeDialog = Core::ActiveDialog::None;
};

} // namespace ModeFlow::Gui