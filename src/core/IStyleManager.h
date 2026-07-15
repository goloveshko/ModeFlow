#pragma once

#include <QMessageBox>

#include "ConfigTypes.h"

class QWidget;
class QString;

namespace ModeFlow::Core {

class IStyleManager {
public:
    virtual ~IStyleManager() = default;

    // Apply Mica, theme, and transparency to a specific window
    virtual void applyToWindow(QWidget* window) = 0;

    virtual Theme currentTheme() const = 0;
    virtual QString currentQtStyleKey() const = 0;
    virtual void setTheme(Theme theme, const QString& qtStyleKey = QString()) = 0;

    // Styled dialogs (to avoid writing QMessageBox invocation logic everywhere)
    virtual void showInfo(QWidget* parent, const QString& title, const QString& text) = 0;
    virtual void showWarning(QWidget* parent, const QString& title, const QString& text) = 0;
    virtual void showError(QWidget* parent, const QString& title, const QString& text) = 0;
    virtual bool confirmAction(QWidget* parent, const QString& title, const QString& text) = 0;

    virtual int showMessageBox(QWidget* parent, QMessageBox::Icon icon, const QString& title, const QString& text,
                               const QString& informativeText = QString(), const QStringList& buttons = QStringList(),
                               int defaultButtonIndex = 0) = 0;

    virtual QString getOpenFileName(QWidget* parent, const QString& caption, const QString& dir,
                                    const QString& filter) = 0;
    virtual QString getSaveFileName(QWidget* parent, const QString& caption, const QString& dir,
                                    const QString& filter) = 0;

    virtual void forceUnhover() = 0;
};

} // namespace ModeFlow::Core

Q_DECLARE_INTERFACE(ModeFlow::Core::IStyleManager, "com.modeflow.IStyleManager")
