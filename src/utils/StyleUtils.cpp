#include "StyleUtils.h"

#include <QAbstractButton>
#include <QCoreApplication>
#include <QStyle>

#include "SystemUtils.h"

#ifdef Q_OS_WIN
#include <dwmapi.h>
#endif

#define DWMWA_MICA_EFFECT 1029

namespace ModeFlow::Gui {

bool StyleUtils::isCompositionEnabled() {
    BOOL enabled = FALSE;
    HRESULT hr = DwmIsCompositionEnabled(&enabled);
    return (hr == S_OK && enabled);
}

void StyleUtils::setImmersiveDarkMode(QWidget* w, bool enabled) {
    if (!w)
        return;
    HWND hwnd = reinterpret_cast<HWND>(w->winId());
    BOOL value = enabled ? TRUE : FALSE;
    // DWMWA_USE_IMMERSIVE_DARK_MODE = 20
    ::DwmSetWindowAttribute(hwnd, 20, &value, sizeof(value));
}

void StyleUtils::applyMica(QWidget* w, bool darkAlt) {
    using namespace ModeFlow::Utils;
    if (!SystemUtils::isWin11() || !isCompositionEnabled())
        return;

    HWND hwnd = reinterpret_cast<HWND>(w->winId());

    if (SystemUtils::isWin11_22H2()) {
        const int entry = DWMWA_SYSTEMBACKDROP_TYPE;
        int reset = DWMSBT_NONE;
        ::DwmSetWindowAttribute(hwnd, entry, &reset, sizeof(int));

        int value = darkAlt ? DWMSBT_MAINWINDOW : DWMSBT_TABBEDWINDOW;
        ::DwmSetWindowAttribute(hwnd, entry, &value, sizeof(int));
    } else {
        // Fallback for Win11 21H2 (old flag 1029)
        const int entry = DWMWA_MICA_EFFECT;
        int value = 0x01;
        ::DwmSetWindowAttribute(hwnd, entry, &value, sizeof(int));
    }
}

void StyleUtils::resetWindowEffects(QWidget* w) {
    if (!w)
        return;

    HWND hwnd = reinterpret_cast<HWND>(w->winId());
    w->setAttribute(Qt::WA_StyledBackground, true);

    w->setAttribute(Qt::WA_TranslucentBackground, false);

    MARGINS margins = {0, 0, 0, 0};
    ::DwmExtendFrameIntoClientArea(hwnd, &margins);

    BOOL darkMode = FALSE;
    ::DwmSetWindowAttribute(hwnd, 20, &darkMode, sizeof(darkMode));

    if (ModeFlow::Utils::SystemUtils::isWin11()) {
        const int entry = DWMWA_SYSTEMBACKDROP_TYPE;
        int backdrop = DWMSBT_NONE;
        ::DwmSetWindowAttribute(hwnd, entry, &backdrop, sizeof(backdrop));
    } else {
        const int entry = DWMWA_MICA_EFFECT;
        int disabled = 0x00;
        ::DwmSetWindowAttribute(hwnd, entry, &disabled, sizeof(disabled));
    }

    refreshFrame(w);
}

void StyleUtils::extendFrame(QWidget* w) {
    if (!w)
        return;
    w->setAttribute(Qt::WA_StyledBackground, true);
    w->setAttribute(Qt::WA_TranslucentBackground, true);
    MARGINS margins = {-1};
    ::DwmExtendFrameIntoClientArea(reinterpret_cast<HWND>(w->winId()), &margins);
}

void StyleUtils::refreshFrame(QWidget* w) {
    if (!w)
        return;
    HWND hwnd = reinterpret_cast<HWND>(w->winId());

    ::SetWindowPos(hwnd, nullptr, 0, 0, 0, 0,
                   SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);

    if (!Utils::SystemUtils::isWin11()) {
        ::SendMessage(hwnd, WM_NCACTIVATE, FALSE, 0);
        ::SendMessage(hwnd, WM_NCACTIVATE, TRUE, 0);
    }
}

void StyleUtils::forceUnhover(QWidget* w) {
    if (!w)
        return;

    QEvent leaveEvent(QEvent::Leave);
    QCoreApplication::sendEvent(w, &leaveEvent);

    w->setAttribute(Qt::WA_UnderMouse, false);

    if (auto* btn = qobject_cast<QAbstractButton*>(w)) {
        btn->setDown(false);
    }

    repolish(w);
}

void StyleUtils::safeThemeApply(QWidget* w, std::function<void()> applyLogic) {
    HWND hwnd = reinterpret_cast<HWND>(w->winId());

    // Freeze native redraw while updating QSS and window attributes.
    ::SendMessage(hwnd, WM_SETREDRAW, FALSE, 0);

    applyLogic();

    // Re-enable redraw and force a full frame refresh.
    ::SendMessage(hwnd, WM_SETREDRAW, TRUE, 0);
    ::RedrawWindow(hwnd, nullptr, nullptr, RDW_ERASE | RDW_FRAME | RDW_INVALIDATE | RDW_ALLCHILDREN);
}

void StyleUtils::repolish(QWidget* widget) {
    if (!widget)
        return;
    widget->style()->unpolish(widget);
    widget->style()->polish(widget);
    widget->update();
}

} // namespace ModeFlow::Gui
