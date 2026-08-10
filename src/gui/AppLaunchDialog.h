#pragma once

#include "BaseDialog.h"
#include "ConfigTypes.h"

namespace Ui {
class AppLaunchDialog;
}

namespace ModeFlow::Gui {

class DialogManager;

class AppLaunchDialog : public BaseDialog {
    Q_OBJECT
public:
    explicit AppLaunchDialog(DialogManager* dm, Core::IStyleManager* sm, QWidget* parent = nullptr);
    ~AppLaunchDialog() override;

    void setAppConfig(const Core::AppLaunchConfig& config);
    Core::AppLaunchConfig appConfig() const;

private slots:
    void browseClicked();
    void validateAndAccept();

private:
    std::unique_ptr<Ui::AppLaunchDialog> ui;
    DialogManager* m_dialogManager = nullptr;
};

} // namespace ModeFlow::Gui
