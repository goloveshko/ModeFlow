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
    ~DialogManager() override;

    Core::ActiveDialog activeDialog() const { return m_activeDialog; }
    void setActiveDialog(Core::ActiveDialog dialog);

    QWidget* parentWindow() const;

    // --- 1. Top-Level Modal App Windows ---
    virtual void showAboutDialog();
    virtual void showLogViewerDialog();
    virtual void showSettingsDialog();
    virtual void showUpdateDialog();

    // --- 2. Action Confirmations ---
    virtual bool confirmApplyProfile(const Core::WorkspaceConfig& config);
    virtual bool confirmAction(const QString& title, const QString& text);
    virtual bool confirmAction(QWidget* parent, const QString& title, const QString& text);

    // --- 3. Message Box Alerts ---
    virtual void showInfo(const QString& title, const QString& text);
    virtual void showInfo(QWidget* parent, const QString& title, const QString& text);

    virtual void showWarning(const QString& title, const QString& text);
    virtual void showWarning(QWidget* parent, const QString& title, const QString& text);

    virtual void showError(const QString& title, const QString& text);
    virtual void showError(QWidget* parent, const QString& title, const QString& text);

    // --- 4. File Dialog Pickers ---
    virtual QString getOpenFileName(const QString& caption, const QString& dir = QString(),
                                    const QString& filter = QString());
    virtual QString getOpenFileName(QWidget* parent, const QString& caption, const QString& dir = QString(),
                                    const QString& filter = QString());

    virtual QString getSaveFileName(const QString& caption, const QString& dir = QString(),
                                    const QString& filter = QString());
    virtual QString getSaveFileName(QWidget* parent, const QString& caption, const QString& dir = QString(),
                                    const QString& filter = QString());

    // --- 5. App Launch Configuration Dialog ---
    virtual std::optional<Core::AppLaunchConfig>
    showAppLaunchDialog(const Core::AppLaunchConfig* initialConfig = nullptr, QWidget* parent = nullptr);

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