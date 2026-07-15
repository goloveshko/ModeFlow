#pragma once

#include <QStyledItemDelegate>

namespace ModeFlow::Gui {

class NavigationDelegate : public QStyledItemDelegate {
    Q_OBJECT
public:
    using QStyledItemDelegate::QStyledItemDelegate;

    explicit NavigationDelegate(QObject* parent = nullptr);

    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override;

private:
    QFont m_iconFont;
};

} // namespace ModeFlow::Gui