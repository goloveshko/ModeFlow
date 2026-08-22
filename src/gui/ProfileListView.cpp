#include "ProfileListView.h"

#include <QContextMenuEvent>
#include <QMenu>

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
}

void ProfileListView::setStyleManager(Core::IStyleManager* sm) {
    m_styleManager = sm;
}

void ProfileListView::contextMenuEvent(QContextMenuEvent* event) {
    QMenu menu(window());

    QAction* createAction = menu.addAction(FontAwesome::icon(FontAwesome::Plus, 16), tr("Create..."));
    QAction* applyAction = nullptr;
    QAction* duplicateAction = nullptr;
    QAction* deleteAction = nullptr;

    QModelIndex idx = indexAt(event->pos());
    if (idx.isValid()) {
        menu.addSeparator();
        applyAction = menu.addAction(FontAwesome::icon(FontAwesome::Check, 16), tr("Apply"));
        duplicateAction = menu.addAction(FontAwesome::icon(FontAwesome::Copy, 16), tr("Duplicate"));
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
    if (event->type() == QEvent::PaletteChange || event->type() == QEvent::ThemeChange ||
        event->type() == QEvent::StyleChange) {
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