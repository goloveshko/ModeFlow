#pragma once

#include <QDialog>

namespace ModeFlow::Core {
class IStyleManager;
}

namespace ModeFlow::Gui {

class BaseDialog : public QDialog {
    Q_OBJECT
public:
    explicit BaseDialog(Core::IStyleManager* styleManager, QWidget* parent = nullptr);
    ~BaseDialog() override;

    void setVisible(bool visible) override;

protected:
    void showEvent(QShowEvent* event) override;
    void changeEvent(QEvent* event) override;

    Core::IStyleManager* styleManager() const { return m_styleManager; }

    virtual bool allowMinimize() const { return false; }
    virtual bool allowMaximize() const { return false; }

    virtual bool allowAutoAdjustSize() const { return true; }

protected:
    Core::IStyleManager* m_styleManager;
    bool m_flagsConfigured = false;
};

} // namespace ModeFlow::Gui