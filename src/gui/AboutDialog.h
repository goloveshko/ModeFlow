#pragma once

#include "BaseDialog.h"

namespace Ui {
class AboutDialog;
}

namespace ModeFlow::Gui {

class AboutDialog : public BaseDialog {
    Q_OBJECT
public:
    explicit AboutDialog(Core::IStyleManager* sm, bool updateAvailable, const QString& latestVersion,
                         QWidget* parent = nullptr);
    ~AboutDialog();

protected:
    void changeEvent(QEvent* event) override;
    void showEvent(QShowEvent* event) override;

private:
    void openLicense();
    void updateNewVersionButton();

    std::unique_ptr<Ui::AboutDialog> ui;
    bool m_updateAvailable = false;
    QString m_latestVersion;
};
} // namespace ModeFlow::Gui