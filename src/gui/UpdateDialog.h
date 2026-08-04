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
    enum DialogResult { CloseResult = QDialog::Rejected, DownloadResult = QDialog::Accepted, SkipVersionResult = 2 };

    explicit UpdateDialog(Core::IStyleManager* sm, const QString& currentVersion, const QString& latestVersion,
                          const QString& changelog, const QUrl& downloadUrl, QWidget* parent = nullptr);
    ~UpdateDialog() override;

private:
    std::unique_ptr<Ui::UpdateDialog> ui;
};

} // namespace ModeFlow::Gui
