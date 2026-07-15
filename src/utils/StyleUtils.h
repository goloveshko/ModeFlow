#pragma once

#include <QWidget>

namespace ModeFlow::Gui {
class StyleUtils {
public:
    // Applies Mica effect to a specific HWND
    static void applyMica(QWidget* w, bool darkAlt);

    // Resets window effects to default Qt/Windows appearance
    static void resetWindowEffects(QWidget* w);

    // Enables dark window border
    static void setImmersiveDarkMode(QWidget* w, bool enabled);

    // Extends frame and makes Qt background transparent
    static void extendFrame(QWidget* w);

    // Forces Windows to redraw non-client area (frame)
    static void refreshFrame(QWidget* w);

    // Forces the widget out of hover/click state
    static void forceUnhover(QWidget* w);

    static void safeThemeApply(QWidget* w, std::function<void()> applyLogic);
    static void repolish(QWidget* widget);

private:
    static bool isCompositionEnabled();
};
} // namespace ModeFlow::Gui