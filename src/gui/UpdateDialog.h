#pragma once

#include <QDialog>
#include <QUrl>

#include "BaseDialog.h"

namespace Ui {
class UpdateDialog;
}

namespace ModeFlow::Gui {

class UpdateDialog : public BaseDialog {
    Q_OBJECT
public:
    explicit UpdateDialog(Core::IStyleManager* sm, const QString& version, const QString& changelog,
                          const QUrl& downloadUrl, QWidget* parent = nullptr);
    ~UpdateDialog();

private:
    std::unique_ptr<Ui::UpdateDialog> ui;
};

} // namespace ModeFlow::Gui
