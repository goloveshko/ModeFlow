#include "ProfileIconMenu.h"

#include <QEvent>
#include <QGridLayout>
#include <QStyle>
#include <QWidgetAction>

#include "FontAwesome.h"

namespace ModeFlow::Gui {

ProfileIconMenu::ProfileIconMenu(QWidget* parent) : QMenu(parent) {
    setObjectName("profileIconMenu");
    buildMenu();
}

void ProfileIconMenu::setCurrentIcon(const QString& symbol) {
    m_currentSymbol = symbol;
    updateSelection();
}

void ProfileIconMenu::buildMenu() {
    auto* container = new QWidget(this);
    container->setObjectName("profileIconMenuContent");

    auto* grid = new QGridLayout(container);
    grid->setContentsMargins(10, 10, 10, 10);
    grid->setHorizontalSpacing(8);
    grid->setVerticalSpacing(8);

    const auto iconDefinitions = FontAwesome::profileIconDefinitions();
    const int columns = columnsForCount(iconDefinitions.size());

    for (int i = 0; i < iconDefinitions.size(); ++i) {
        const auto& iconDef = iconDefinitions.at(i);
        auto* button = new QToolButton(container);
        button->setObjectName("profileIconOption");
        button->setToolButtonStyle(Qt::ToolButtonIconOnly);
        button->setToolTip(iconDef.label);
        button->setIcon(FontAwesome::icon(iconDef.symbol, 32));
        button->setIconSize(QSize(32, 32));
        button->setFixedSize(42, 42);
        button->setProperty("iconSymbol", iconDef.symbol);
        button->setProperty("selected", false);

        connect(button, &QToolButton::clicked, this, [this, symbol = iconDef.symbol]() {
            emit iconSelected(symbol);
            close();
        });

        grid->addWidget(button, i / columns, i % columns);
        m_buttons.append(button);
    }

    auto* action = new QWidgetAction(this);
    action->setDefaultWidget(container);
    addAction(action);
}

void ProfileIconMenu::updateSelection() {
    for (auto* button : m_buttons) {
        if (button) {
            const bool isSelected = (button->property("iconSymbol").toString() == m_currentSymbol);
            button->setProperty("selected", isSelected);

            // Repolish to apply selected borders in QSS
            button->style()->unpolish(button);
            button->style()->polish(button);
            button->update();
        }
    }
}

int ProfileIconMenu::columnsForCount(int iconCount) const {
    if (iconCount <= 0)
        return 1;
    const int minColumns = qMin(3, iconCount);
    const int maxColumns = qMin(6, iconCount);
    int bestColumns = minColumns;
    int bestScore = std::numeric_limits<int>::max();

    for (int columns = minColumns; columns <= maxColumns; ++columns) {
        const int rows = (iconCount + columns - 1) / columns;
        const int emptyCells = rows * columns - iconCount;
        const int shapePenalty = qAbs(columns - rows) * 3;
        const int emptyPenalty = emptyCells * 2;
        const int widthPenalty = columns > rows ? 0 : 1;
        const int score = shapePenalty + emptyPenalty + widthPenalty;

        if (score < bestScore) {
            bestScore = score;
            bestColumns = columns;
        }
    }
    return bestColumns;
}

void ProfileIconMenu::changeEvent(QEvent* event) {
    if (event->type() == QEvent::LanguageChange) {
        const auto iconDefinitions = FontAwesome::profileIconDefinitions();
        for (int i = 0; i < m_buttons.size() && i < iconDefinitions.size(); ++i) {
            if (m_buttons[i]) {
                m_buttons[i]->setToolTip(iconDefinitions[i].label);
            }
        }
    }

    if (event->type() == QEvent::PaletteChange || event->type() == QEvent::ThemeChange ||
        event->type() == QEvent::StyleChange) {
        for (auto* button : m_buttons) {
            if (button) {
                button->setIcon(FontAwesome::icon(button->property("iconSymbol").toString(), 32));
            }
        }
    }
    QMenu::changeEvent(event);
}

} // namespace ModeFlow::Gui