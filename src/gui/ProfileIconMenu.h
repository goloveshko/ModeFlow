#pragma once

#include <QList>
#include <QMenu>
#include <QToolButton>

namespace ModeFlow::Gui {

/**
 * @brief Self-contained Fluent-style popup menu for selecting profile icons.
 * Completely encapsulates grid layout calculations, selections, and theme-change repaints.
 */
class ProfileIconMenu : public QMenu {
    Q_OBJECT
public:
    explicit ProfileIconMenu(QWidget* parent = nullptr);

    void setCurrentIcon(const QString& symbol);
    QString currentIcon() const { return m_currentSymbol; }

signals:
    void iconSelected(const QString& symbol);

protected:
    void changeEvent(QEvent* event) override;

private:
    void buildMenu();
    void updateSelection();
    int columnsForCount(int iconCount) const;

    QString m_currentSymbol;
    QList<QToolButton*> m_buttons;
};

} // namespace ModeFlow::Gui