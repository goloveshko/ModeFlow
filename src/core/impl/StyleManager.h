#pragma once

#include <QList>
#include <QObject>
#include <QWidget>

#include "ConfigTypes.h"
#include "IStyleManager.h"

namespace ModeFlow::Core {

class StyleManager : public QObject, public IStyleManager {
    Q_OBJECT
public:
    explicit StyleManager(Core::Theme theme, const QString& qtStyleKey, QObject* parent = nullptr);

    QList<Core::ThemeData> availableThemes() const;

    void applyToWindow(QWidget* window) override;

    void setTheme(Core::Theme theme, const QString& qtStyleKey = QString()) override;
    Core::Theme currentTheme() const override;
    QString currentQtStyleKey() const override;

    int showMessageBox(QWidget* parent, QMessageBox::Icon icon, const QString& title, const QString& text,
                       const QString& informativeText = QString(), const QStringList& buttons = QStringList(),
                       int defaultButtonIndex = 0) override;

    void showInfo(QWidget* parent, const QString& title, const QString& text) override;
    void showWarning(QWidget* parent, const QString& title, const QString& text) override;
    void showError(QWidget* parent, const QString& title, const QString& text) override;
    bool confirmAction(QWidget* parent, const QString& title, const QString& text) override;

    QString getOpenFileName(QWidget* parent, const QString& caption, const QString& dir,
                            const QString& filter) override;
    QString getSaveFileName(QWidget* parent, const QString& caption, const QString& dir,
                            const QString& filter) override;

    void forceUnhover() override;

private:
    QString loadStylesheet(QStringView fileName) const;
    Core::Theme m_currentTheme = Core::Theme::Light;
    QString m_qtStyleKey;
};

} // namespace ModeFlow::Core