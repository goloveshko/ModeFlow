#pragma once

#include <QDialog>

#include "BaseDialog.h"
#include "ConfigTypes.h"

namespace Ui {
class AppLaunchDialog;
}

namespace ModeFlow::Gui {

class AppLaunchDialog : public BaseDialog {
    Q_OBJECT
public:
    explicit AppLaunchDialog(Core::IStyleManager* sm, QWidget* parent = nullptr);
    ~AppLaunchDialog();

    void setAppConfig(const Core::AppLaunchConfig& config);
    Core::AppLaunchConfig appConfig() const;

private slots:
    void browseClicked();
    void validateAndAccept();

private:
    std::unique_ptr<Ui::AppLaunchDialog> ui;
};

} // namespace ModeFlow::Gui
