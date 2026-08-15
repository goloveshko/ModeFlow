#include "ProfileListView.h"

#include <QContextMenuEvent>
#include <QHBoxLayout>
#include <QMenu>
#include <QToolButton>

#include "FluentItemDelegate.h"
#include "FontAwesome.h"

namespace ModeFlow::Gui {

ProfileListView::ProfileListView(QWidget* parent) : QListView(parent) {
    setItemDelegate(new FluentItemDelegate(this));
    setSpacing(4);
}

ProfileListView::~ProfileListView() = default;

void ProfileListView::setWorkspaceManager(Core::IWorkspaceManager* manager) {
    m_workspaceManager = manager;
    setModel(m_workspaceManager->model());

    connect(model(), &QAbstractItemModel::modelReset, this, &ProfileListView::updateRowWidgets);
    connect(model(), &QAbstractItemModel::rowsInserted, this, &ProfileListView::updateRowWidgets);
    connect(model(), &QAbstractItemModel::rowsRemoved, this, &ProfileListView::updateRowWidgets);
    connect(model(), &QAbstractItemModel::dataChanged, this, &ProfileListView::updateRowWidgets);

    updateRowWidgets();
}

void ProfileListView::setStyleManager(Core::IStyleManager* sm) {
    m_styleManager = sm;
    updateRowWidgets();
}

void ProfileListView::updateRowWidgets() {
    if (!model() || !m_workspaceManager)
        return;

    const auto configs = m_workspaceManager->configs();

    for (int row = 0; row < configs.size(); ++row) {
        QModelIndex idx = model()->index(row, 0);

        QWidget* existingWidget = indexWidget(idx);

        if (existingWidget) {
            // Update the existing delete button's icon to use the new theme colors.
            // This preserves our smooth resize optimization while ensuring visual consistency on theme switch.
            if (auto* deleteBtn = existingWidget->findChild<QToolButton*>("btnDeleteProfileInline")) {
                deleteBtn->setIcon(FontAwesome::icon(FontAwesome::Trash, 14));
            }
            continue; // Skip heavy widget recreation
        }

        auto* rowWidget = new QWidget(this);
        rowWidget->setObjectName("profileRowWidget");

        auto* layout = new QHBoxLayout(rowWidget);
        layout->setContentsMargins(0, 0, 8, 0);
        layout->addStretch();

        auto* deleteBtn = new QToolButton(rowWidget);
        deleteBtn->setObjectName("btnDeleteProfileInline");
        deleteBtn->setCursor(Qt::PointingHandCursor);
        deleteBtn->setIcon(FontAwesome::icon(FontAwesome::Trash, 14));
        deleteBtn->setFixedSize(24, 24);
        deleteBtn->setToolTip(tr("Delete profile"));
        layout->addWidget(deleteBtn);

        connect(deleteBtn, &QToolButton::clicked, this, [this, row]() { emit deleteRequested(row); });

        setIndexWidget(idx, rowWidget);
    }
}

void ProfileListView::contextMenuEvent(QContextMenuEvent* event) {
    QMenu menu(window());

    QAction* createAction = menu.addAction(FontAwesome::icon(FontAwesome::Plus, 16), tr("Create profile..."));
    QAction* applyAction = nullptr;
    QAction* duplicateAction = nullptr;
    QAction* deleteAction = nullptr;

    QModelIndex idx = indexAt(event->pos());
    if (idx.isValid()) {
        menu.addSeparator();
        applyAction = menu.addAction(FontAwesome::icon(FontAwesome::Check, 16), tr("Apply Profile"));
        duplicateAction = menu.addAction(FontAwesome::icon(FontAwesome::Copy, 16), tr("Duplicate profile"));
        deleteAction = menu.addAction(FontAwesome::icon(FontAwesome::Trash, 16), tr("Delete"));
    }

    QAction* selectedAction = menu.exec(event->globalPos());

    if (selectedAction == createAction) {
        emit createRequested();
    } else if (selectedAction == applyAction && idx.isValid()) {
        emit applyRequested(idx.row());
    } else if (selectedAction == duplicateAction && idx.isValid()) {
        emit duplicateRequested(idx.row());
    } else if (selectedAction == deleteAction && idx.isValid()) {
        emit deleteRequested(idx.row());
    }
}

void ProfileListView::changeEvent(QEvent* event) {
    if (event->type() == QEvent::LanguageChange) {
        updateRowWidgets();
    }

    if (event->type() == QEvent::PaletteChange || event->type() == QEvent::ThemeChange ||
        event->type() == QEvent::StyleChange) {

        updateRowWidgets();
        viewport()->update();
    }

    QListView::changeEvent(event);
}

void ProfileListView::keyPressEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_Delete) {
        QModelIndex idx = currentIndex();
        if (idx.isValid()) {
            emit deleteRequested(idx.row());
            event->accept();
            return;
        }
    } else if (event->key() == Qt::Key_Insert) {
        emit createRequested();
        event->accept();
        return;
    }

    QListView::keyPressEvent(event);
}

} // namespace ModeFlow::Gui