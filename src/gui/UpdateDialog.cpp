#include "UpdateDialog.h"

#include "ui_UpdateDialog.h"

#include <QDesktopServices>

#include "IStyleManager.h"

namespace ModeFlow::Gui {

using namespace Qt::StringLiterals;

UpdateDialog::UpdateDialog(Core::IStyleManager* sm, const QString& version, const QString& changelog,
                           const QUrl& downloadUrl, QWidget* parent)
    : BaseDialog(sm, parent), ui(std::make_unique<Ui::UpdateDialog>()) {
    ui->setupUi(this);

    auto titleFont = ui->titleLabel->font();
    titleFont.setPointSize(titleFont.pointSize() + 2);
    titleFont.setBold(true);
    ui->titleLabel->setFont(titleFont);
    ui->titleLabel->setText(tr("Update Available — v%1").arg(version));

    if (!changelog.isEmpty()) {
        ui->changelogBrowser->setMarkdown(changelog);
    } else {
        ui->changelogBrowser->setPlainText(tr("No changelog available."));
    }

    connect(ui->downloadBtn, &QPushButton::clicked, this, [this, downloadUrl]() {
        QDesktopServices::openUrl(downloadUrl);
        accept();
    });

    connect(ui->closeBtn, &QPushButton::clicked, this, &UpdateDialog::reject);
}

UpdateDialog::~UpdateDialog() = default;

} // namespace ModeFlow::Gui
