#include "BaseDialog.h"

#include <QEvent>

#include "IStyleManager.h"
#include "SystemUtils.h"

namespace ModeFlow::Gui {

BaseDialog::BaseDialog(Core::IStyleManager* styleManager, QWidget* parent)
    : QDialog(parent), m_styleManager(styleManager) {
    Q_ASSERT(m_styleManager);

    m_styleManager->applyToWindow(this);
}

BaseDialog::~BaseDialog() = default;

void BaseDialog::setVisible(bool visible) {
    if (visible && !m_flagsConfigured) {
        m_flagsConfigured = true;

        Qt::WindowFlags flags = windowFlags();
        flags |= Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint;

        if (allowMinimize() || allowMaximize()) {
            flags |= Qt::WindowMinMaxButtonsHint;
        }
        setWindowFlags(flags);
    }
    QDialog::setVisible(visible);
}

void BaseDialog::showEvent(QShowEvent* event) {
    QDialog::showEvent(event);

    if (isVisible() && m_styleManager) {
        Utils::SystemUtils::configureWindowButtons(this, allowMinimize(), allowMaximize());

        m_styleManager->applyToWindow(this);
    }
}

void BaseDialog::changeEvent(QEvent* event) {
    if (m_styleManager && (event->type() == QEvent::PaletteChange || event->type() == QEvent::ThemeChange ||
                           event->type() == QEvent::StyleChange)) {

        m_styleManager->applyToWindow(this);
        update();

        if (allowAutoAdjustSize() && isVisible()) {
            QMetaObject::invokeMethod(this, &QWidget::adjustSize, Qt::QueuedConnection);
        }
    }
    QDialog::changeEvent(event);
}

} // namespace ModeFlow::Gui