#include "FluentListItemDelegate.h"

#include <QPainter>

#include "FontAwesome.h"
#include "StyleBridge.h"
#include "WorkspaceModel.h"

namespace ModeFlow::Gui {

FluentListItemDelegate::FluentListItemDelegate(QObject* parent) : QStyledItemDelegate(parent) {
    m_iconFont.setFamily(FontAwesome::fontFamily());
    m_iconFont.setPixelSize(16);
    m_iconFont.setHintingPreference(QFont::PreferNoHinting);
}

void FluentListItemDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option,
                                   const QModelIndex& index) const {
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setRenderHint(QPainter::TextAntialiasing);

    const auto& bridge = StyleBridge::instance();
    const QRect rect = option.rect.adjusted(4, 2, -4, -2);
    const bool isSelected = option.state & QStyle::State_Selected;
    const bool isHovered = option.state & QStyle::State_MouseOver;

    const bool isActive = index.data(Core::WorkspaceModel::ActiveRole).toBool();

    if (isSelected) {
        painter->setBrush(bridge.sidebarItemSelectedBg());
        painter->setPen(Qt::NoPen);
        painter->drawRoundedRect(rect, 6, 6);

        painter->setBrush(bridge.sidebarAccent());
        const int barPadding = 12;
        painter->drawRoundedRect(QRect(rect.left(), rect.top() + barPadding, 3, rect.height() - 2 * barPadding), 1.5,
                                 1.5);
    } else if (isHovered) {
        painter->setBrush(bridge.sidebarItemHoverBg());
        painter->setPen(Qt::NoPen);
        painter->drawRoundedRect(rect, 6, 6);
    }

    const QRect iconRect(rect.left() + 10, rect.top(), 24, rect.height());
    QVariant decoration = index.data(Qt::DecorationRole);

    if (!decoration.isValid()) {
    } else if (decoration.typeId() == QMetaType::QIcon || decoration.typeId() == QMetaType::QPixmap) {
        QIcon icon = decoration.value<QIcon>();
        icon.paint(painter, iconRect, Qt::AlignCenter);
    } else {
        QString symbol = decoration.toString();
        if (symbol.isEmpty()) {
            symbol = FontAwesome::defaultProfileIconSymbol();
        }

        painter->setFont(m_iconFont);

        // Visual indicator: Tint the profile icon with accent color if active
        QColor iconColor =
            isSelected ? bridge.sidebarTextSelected() : (isActive ? bridge.sidebarAccent() : bridge.iconNormal());
        painter->setPen(iconColor);
        painter->drawText(iconRect, Qt::AlignCenter, symbol);
    }

    const QString text = index.data(Qt::DisplayRole).toString();
    painter->setFont(option.font);

    QFont textFont = option.font;
    if (isSelected) {
        textFont.setWeight(QFont::DemiBold);
        painter->setFont(textFont);
    }

    // Visual indicator: Tint the profile name text with accent color if active
    QColor textColor =
        isSelected ? bridge.sidebarTextSelected() : (isActive ? bridge.sidebarAccent() : bridge.sidebarTextNormal());
    painter->setPen(textColor);

    // The right-side inset of textRect is calculated as: -(RightContentsMargin + DeleteButtonWidth)
    // - 8 is the right margin of the row layout (layout->setContentsMargins(0, 0, 8, 0))
    // - 24 is the exact width of the delete button (deleteBtn->setFixedSize(24, 24))
    // This defines the precise safety boundary where text elision starts, preventing overlaps.
    const QRect textRect = rect.adjusted(iconRect.right(), 0, -(8 + 24), 0);

    QFontMetrics fm(painter->font());
    const QString elidedText = fm.elidedText(text, Qt::ElideMiddle, textRect.width());

    painter->drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft, elidedText);

    painter->restore();
}

QSize FluentListItemDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const {
    QSize size = QStyledItemDelegate::sizeHint(option, index);
    return QSize(size.width(), 48);
}

} // namespace ModeFlow::Gui