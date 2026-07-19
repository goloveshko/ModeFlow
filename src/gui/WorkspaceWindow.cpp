#include "WorkspaceWindow.h"

#include "ui_WorkspaceWindow.h"

#include <QToolTip>

#include "AppLaunchDialog.h"
#include "Constants.h"
#include "FontAwesome.h"
#include "HotkeyValidation.h"
#include "ISettingsManager.h"
#include "IStyleManager.h"
#include "IWorkspaceManager.h"
#include "ProfileDetailsController.h"
#include "ProfileExchangeController.h"
#include "ProfileIconMenu.h"
#include "ProfileListView.h"

namespace ModeFlow::Gui {

using namespace Qt::StringLiterals;

namespace {
int resolvedIconExtent(const QSize& iconSize, int fallback) {
    return iconSize.width() > 0 ? iconSize.width() : fallback;
}
} // namespace

WorkspaceWindow::WorkspaceWindow(Core::IWorkspaceManager* workspaceManager, Core::ISettingsManager* settingsManager,
                                 Core::IStyleManager* sm, QWidget* parent)
    : BaseDialog(sm, parent), ui(std::make_unique<Ui::WorkspaceWindow>()), m_workspaceManager(workspaceManager),
      m_settingsManager(settingsManager) {
    Q_ASSERT(m_workspaceManager);
    Q_ASSERT(m_settingsManager);

    ui->setupUi(this);

    Qt::WindowFlags flags = windowFlags();
    flags |= Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint | Qt::WindowMinMaxButtonsHint;
    setWindowFlags(flags);

    init();
}

WorkspaceWindow::~WorkspaceWindow() = default;

void WorkspaceWindow::init() {
    refreshVisualState();

    m_autosaveTimer = new QTimer(this);
    m_autosaveTimer->setSingleShot(true);
    m_autosaveTimer->setInterval(Utils::ProfileAutosaveDebounceMs);

    ui->configList->setWorkspaceManager(m_workspaceManager);
    ui->configList->setStyleManager(m_styleManager);
    ui->listApps->setStyleManager(m_styleManager);

    m_profileIconMenu = new ProfileIconMenu(this);

    m_exchangeController = std::make_unique<ProfileExchangeController>(m_workspaceManager, m_styleManager, this);

    ProfileDetailsWidgets widgets;
    widgets.editName = ui->editName;
    widgets.keyEditSpecific = ui->keyEditSpecific;
    widgets.checkSkipInCycle = ui->checkSkipInCycle;
    widgets.comboDisplay = ui->comboDisplay;
    widgets.comboAudio = ui->comboAudio;
    widgets.listApps = ui->listApps;
    widgets.btnCapture = ui->btnCapture;
    widgets.groupGeneral = ui->groupGeneral;
    widgets.groupHardware = ui->groupHardware;
    widgets.groupAuto = ui->groupAuto;

    m_detailsController = std::make_unique<ProfileDetailsController>(widgets, m_workspaceManager, m_settingsManager,
                                                                     m_profileIconMenu, this);

    initMoreMenu();
    setupConnections();

    restoreWindowGeometry();
    restoreSelection();
    updateUI();
}

void WorkspaceWindow::initMoreMenu() {
    if (m_moreMenu) {
        m_moreMenu->deleteLater();
    }

    m_moreMenu = new QMenu(this);
    m_moreMenu->setObjectName("moreMenu");

    m_importAction = m_moreMenu->addAction(FontAwesome::icon(FontAwesome::FileImport, 20), tr("Import Profiles"),
                                           m_exchangeController.get(), &ProfileExchangeController::doImport);
    m_exportAction = m_moreMenu->addAction(FontAwesome::icon(FontAwesome::FileExport, 20), tr("Export Profiles"),
                                           m_exchangeController.get(), &ProfileExchangeController::doExport);
    m_moreMenu->addSeparator();
    m_checkUpdatesAction = m_moreMenu->addAction(FontAwesome::icon(FontAwesome::CloudArrowDown, 20),
                                                 tr("Check for Updates..."), this, &WorkspaceWindow::forceUpdateCheck);
    m_logViewerAction = m_moreMenu->addAction(FontAwesome::icon(FontAwesome::FileLines, 20), tr("View Log"), this,
                                              &WorkspaceWindow::showLogViewer);

    ui->btnMore->setMenu(m_moreMenu);
    ui->btnMore->setPopupMode(QToolButton::InstantPopup);
}

void WorkspaceWindow::setupConnections() {
    connect(ui->configList->selectionModel(), &QItemSelectionModel::currentRowChanged, this,
            &WorkspaceWindow::on_selectionChanged);

    connect(ui->configList, &ProfileListView::createRequested, this, &WorkspaceWindow::addClicked);
    connect(ui->configList, &ProfileListView::deleteRequested, this, &WorkspaceWindow::deleteProfileByRow);
    connect(ui->configList, &ProfileListView::duplicateRequested, this, &WorkspaceWindow::duplicateProfileByRow);
    connect(ui->configList, &ProfileListView::applyRequested, this, &WorkspaceWindow::applyProfileByRow);

    connect(ui->btnCreateProfile, &QPushButton::clicked, this, &WorkspaceWindow::addClicked);

    connect(ui->btnSettings, &QToolButton::clicked, this, &WorkspaceWindow::showSettingsDialog);
    connect(ui->btnAbout, &QToolButton::clicked, this, &WorkspaceWindow::showAboutDialog);
    connect(ui->btnUpdate, &QToolButton::clicked, this, &WorkspaceWindow::showUpdateDialog);

    connect(m_exchangeController.get(), &ProfileExchangeController::exchangeCompleted, this, [this]() {
        notifySettingsChanged();
        updateUI();
        restoreSelection();
    });

    connect(m_detailsController.get(), &ProfileDetailsController::profileChanged, this,
            &WorkspaceWindow::scheduleAutosave);
    connect(m_detailsController.get(), &ProfileDetailsController::hotkeyCaptureChanged, this,
            &WorkspaceWindow::hotkeyCaptureChanged);
    connect(m_detailsController.get(), &ProfileDetailsController::validateSpecificHotkey, this,
            &WorkspaceWindow::validateSpecificHotkey);

    connect(m_autosaveTimer, &QTimer::timeout, this, &WorkspaceWindow::autosaveCurrentProfile);
}

void WorkspaceWindow::raiseWindow() {
    if (isMinimized()) {
        showNormal();
    } else if (!isVisible()) {
        show();
    }
    raise();
    activateWindow();
}

void WorkspaceWindow::validateSpecificHotkey() {
    if (m_isUpdating)
        return;

    const int row = currentRow();
    if (row < 0)
        return;

    const auto& configs = m_workspaceManager->getAllWorkspaceConfigs();
    const QString currentId = m_workspaceManager->model()->configs().at(row).id;

    // Access the hotkey edit widget from the controller's widgets struct or expose a helper
    // Actually, we can access the widget safely since we know where it is
    auto* keyEdit = ui->keyEditSpecific;

    if (HotkeyValidation::validateProfileHotkey(keyEdit, m_settingsManager->nextProfileHotkey(), configs, currentId,
                                                m_styleManager, this)) {
        saveCurrentToModel(row);
        scheduleAutosave();
    }
}

void WorkspaceWindow::restoreSelection() {
    int row = m_workspaceManager->selectedRow();

    if (row == -1 && m_workspaceManager->model()->rowCount() > 0) {
        row = 0;
    }

    if (row != -1) {
        ui->configList->setCurrentIndex(m_workspaceManager->model()->index(row, 0));
    }
}

void WorkspaceWindow::addClicked() {
    int row = currentRow();
    if (row != -1)
        saveCurrentToModel(row);

    m_workspaceManager->createDefaultProfile();

    int lastRow = m_workspaceManager->model()->rowCount() - 1;
    persistProfiles();
    setCurrentRowSilently(lastRow);

    captureCurrentSettings();
    updateUI();
}

void WorkspaceWindow::deleteClicked() {
    QModelIndex index = currentIndex();
    if (!index.isValid())
        return;

    int rowToDelete = index.row();

    m_styleManager->forceUnhover();

    if (!m_styleManager->confirmAction(
            this, tr("Delete"),
            tr("Delete configuration '%1'?").arg(m_workspaceManager->model()->configs()[rowToDelete].name)))
        return;

    m_isUpdating = true;
    ui->configList->setCurrentIndex(QModelIndex());
    m_isUpdating = false;

    m_workspaceManager->removeConfig(rowToDelete);
    persistProfiles();

    int rowCount = m_workspaceManager->model()->rowCount();
    if (rowCount > 0) {
        setCurrentRowSilently(std::clamp(rowToDelete, 0, rowCount - 1));
    } else {
        setCurrentRowSilently(-1);
        updateUI();
    }
}

void WorkspaceWindow::on_selectionChanged(const QModelIndex& current, const QModelIndex& previous) {
    if (m_isUpdating)
        return;

    if (previous.isValid()) {
        // High-end UX optimization: If we have pending unsaved edits (autosave timer is active),
        // stop the timer and force write the edits now before switching rows.
        // If no edits were made, we avoid redundant disk I/O and heavy hotkey re-registrations!
        if (m_autosaveTimer && m_autosaveTimer->isActive()) {
            m_autosaveTimer->stop();
            autosaveCurrentProfile();
        }
    }

    if (current.isValid()) {
        ui->settingsLayout->setEnabled(true);
        loadRowToUi(current.row());
    } else {
        ui->settingsLayout->setEnabled(false);
    }
}

void WorkspaceWindow::on_btnCapture_clicked() {
    captureCurrentSettings();
}

void WorkspaceWindow::saveCurrentToModel(int row) {
    if (row < 0 || row >= m_workspaceManager->model()->rowCount())
        return;

    Core::WorkspaceConfig cfg = m_workspaceManager->model()->configs().at(row);
    m_detailsController->saveProfile(cfg);

    m_workspaceManager->model()->updateConfig(row, cfg);
}

void WorkspaceWindow::autosaveCurrentProfile() {
    const int row = currentRow();
    if (m_isUpdating || row < 0 || row >= m_workspaceManager->model()->rowCount()) {
        return;
    }

    saveCurrentToModel(row);
    persistProfiles();
}

void WorkspaceWindow::scheduleAutosave() {
    if (m_isUpdating || currentRow() < 0 || !m_autosaveTimer) {
        return;
    }

    saveCurrentToModel(currentRow());
    m_autosaveTimer->start();
}

void WorkspaceWindow::loadRowToUi(int row) {
    if (row < 0 || row >= m_workspaceManager->model()->rowCount())
        return;
    m_isUpdating = true;
    const auto& cfg = m_workspaceManager->model()->configs().at(row);

    m_detailsController->loadProfile(cfg);

    m_isUpdating = false;
}

void WorkspaceWindow::notifySettingsChanged() {
    emit profilesChanged();
}

bool WorkspaceWindow::persistProfiles() {
    if (!m_workspaceManager->saveWorkspaces()) {
        return false;
    }

    notifySettingsChanged();
    return true;
}

void WorkspaceWindow::setCurrentRowSilently(int row) {
    m_isUpdating = true;

    if (row >= 0 && row < m_workspaceManager->model()->rowCount()) {
        ui->configList->setCurrentIndex(m_workspaceManager->model()->index(row, 0));
        ui->settingsLayout->setEnabled(true);
        loadRowToUi(row);
    } else {
        ui->configList->setCurrentIndex(QModelIndex());
        ui->settingsLayout->setEnabled(false);
    }

    m_isUpdating = false;
}

void WorkspaceWindow::captureCurrentSettings() {
    if (currentRow() < 0)
        return;

    m_detailsController->captureCurrentSettings();
}

void WorkspaceWindow::changeEvent(QEvent* event) {
    if (event->type() == QEvent::LanguageChange) {
        QSignalBlocker blocker(this);
        ui->retranslateUi(this);
        initMoreMenu();
        loadRowToUi(currentRow());
    }
    if (event->type() == QEvent::PaletteChange || event->type() == QEvent::ThemeChange ||
        event->type() == QEvent::StyleChange) {
        QSignalBlocker blocker(this);
        refreshVisualState();
        update();
    }
    if (event->type() == QEvent::WindowStateChange) {
        m_settingsManager->setMainWindowMaximized(isMaximized());
    }
    BaseDialog::changeEvent(event);
}

void WorkspaceWindow::showEvent(QShowEvent* event) {
    BaseDialog::showEvent(event);

    if (!m_styleManager)
        return;

    // DWM/Mica title-bar settings can be dropped after hide/show cycles.
    // Re-apply once the window is shown so the native frame is in a stable state.
    if (isVisible()) {
        m_settingsManager->setMainWindowVisible(true);

        refreshVisualState();
        ui->configList->viewport()->update();
        update();

        if (m_firstShow) {
            m_firstShow = false;
            if (m_settingsManager->isMainWindowMaximized()) {
                showMaximized();
            }
        }
    }
}

void WorkspaceWindow::closeEvent(QCloseEvent* event) {
    emit hotkeyCaptureChanged(false);

    // Stop the autosave timer safely.
    // If the timer is active (meaning there are pending unsaved edits), force write them now.
    // If no edits were made, we avoid redundant disk I/O and heavy hotkey re-registrations on hide!
    if (m_autosaveTimer) {
        if (m_autosaveTimer->isActive()) {
            m_autosaveTimer->stop();
            autosaveCurrentProfile();
        } else {
            m_autosaveTimer->stop();
        }
    }

    int row = currentRow();
    m_workspaceManager->setSelectedRow(row);

    event->accept();
}

void WorkspaceWindow::hideEvent(QHideEvent* event) {
    saveWindowGeometry();
    BaseDialog::hideEvent(event);
}

void WorkspaceWindow::updateUI() {
    bool enable = m_workspaceManager->model()->rowCount() > 0;

    m_detailsController->updateUI(enable);
}

void WorkspaceWindow::refreshVisualState() {
    const int toolbarIconSize = resolvedIconExtent(ui->btnSettings->iconSize(), 16);

    ui->btnCreateProfile->setIcon(FontAwesome::icon(FontAwesome::Plus, toolbarIconSize));
    ui->btnSettings->setIcon(FontAwesome::icon(FontAwesome::Settings, toolbarIconSize));
    ui->btnMore->setIcon(FontAwesome::icon(FontAwesome::EllipsisVertical, toolbarIconSize));
    ui->btnAbout->setIcon(FontAwesome::icon(FontAwesome::Info, toolbarIconSize));
    ui->btnUpdate->setIcon(FontAwesome::icon(FontAwesome::CloudArrowDown, toolbarIconSize));

    if (m_importAction)
        m_importAction->setIcon(FontAwesome::icon(FontAwesome::FileImport, 20));
    if (m_exportAction)
        m_exportAction->setIcon(FontAwesome::icon(FontAwesome::FileExport, 20));
    if (m_checkUpdatesAction)
        m_checkUpdatesAction->setIcon(FontAwesome::icon(FontAwesome::CloudArrowDown, 20));
    if (m_logViewerAction)
        m_logViewerAction->setIcon(FontAwesome::icon(FontAwesome::FileLines, 20));

    if (m_detailsController) {
        m_detailsController->refreshVisualState();
    }

    // Force the profile list viewport to repaint itself.
    // This dynamically triggers the delegate to draw/move the active status dot!
    ui->configList->viewport()->update();
}

int WorkspaceWindow::currentRow() const {
    return ui->configList->currentIndex().row();
}

QModelIndex WorkspaceWindow::currentIndex() const {
    return ui->configList->currentIndex();
}

bool WorkspaceWindow::toggleVisibility() {
    if (isVisible()) {
        emit hotkeyCaptureChanged(false);
        hide();
        return false;
    }

    raiseWindow();
    return true;
}

void WorkspaceWindow::setUpdateAvailable(bool available, const QString& version) {
    if (available) {
        ui->btnUpdate->setToolTip(tr("Update available: v%1 — click to view").arg(version));
        ui->btnUpdate->setVisible(true);
    } else {
        ui->btnUpdate->setVisible(false);
    }
}

void WorkspaceWindow::showToolTipOnMoreButton(const QString& text) {
    if (!isVisible())
        return;

    int x = ui->btnMore->width() / 2;
    int y = -4;

    QPoint globalPos = ui->btnMore->mapToGlobal(QPoint(x, y));

    QToolTip::showText(globalPos, text, ui->btnMore);
}

void WorkspaceWindow::deleteProfileByRow(int row) {
    if (row < 0 || row >= m_workspaceManager->model()->rowCount())
        return;

    m_styleManager->forceUnhover();

    if (!m_styleManager->confirmAction(
            this, tr("Delete"), tr("Delete configuration '%1'?").arg(m_workspaceManager->model()->configs()[row].name)))
        return;

    m_isUpdating = true;
    ui->configList->setCurrentIndex(QModelIndex());
    m_isUpdating = false;

    m_workspaceManager->removeConfig(row);
    persistProfiles();

    int rowCount = m_workspaceManager->model()->rowCount();
    if (rowCount > 0) {
        setCurrentRowSilently(std::clamp(row, 0, rowCount - 1));
    } else {
        setCurrentRowSilently(-1);
        updateUI();
    }
}

void WorkspaceWindow::duplicateProfileByRow(int row) {
    if (row < 0 || row >= m_workspaceManager->model()->rowCount())
        return;

    saveCurrentToModel(currentRow());

    m_workspaceManager->duplicateProfile(row);

    int lastRow = m_workspaceManager->model()->rowCount() - 1;
    setCurrentRowSilently(lastRow);
}

void WorkspaceWindow::applyProfileByRow(int row) {
    if (row < 0 || row >= m_workspaceManager->model()->rowCount())
        return;

    const auto& cfg = m_workspaceManager->model()->configs().at(row);

    emit activateProfile(cfg);
}

void WorkspaceWindow::saveWindowGeometry() {
    const bool maximized = m_settingsManager->isMainWindowMaximized();
    if (!maximized) {
        m_settingsManager->setMainWindowPos(pos());
        m_settingsManager->setMainWindowSize(size());
    }
}

void WorkspaceWindow::restoreWindowGeometry() {
    const QPoint savedPos = m_settingsManager->mainWindowPos();
    const QSize savedSize = m_settingsManager->mainWindowSize();

    auto centerOnPrimary = [this]() {
        if (auto* primary = QGuiApplication::primaryScreen()) {
            const QRect screenGeom = primary->geometry();
            const int x = screenGeom.left() + (screenGeom.width() - 600) / 2;
            const int y = screenGeom.top() + (screenGeom.height() - 450) / 2;
            move(x, y);
            resize(600, 450);
        } else {
            resize(600, 450);
        }
    };

    if (!savedPos.isNull() && savedSize.isValid()) {
        // Multi-monitor safety check: verify if the saved position lies within any active monitor bounds.
        // This prevents the window from being rendered off-screen if a monitor was disconnected.
        bool posIsVisibleOnAnyMonitor = false;
        for (auto* screen : QGuiApplication::screens()) {
            if (screen->geometry().contains(savedPos)) {
                posIsVisibleOnAnyMonitor = true;
                break;
            }
        }

        if (posIsVisibleOnAnyMonitor) {
            move(savedPos);
            resize(savedSize);
        } else {
            centerOnPrimary(); // Fallback if old monitor is missing
        }
    } else {
        centerOnPrimary(); // Default size for first launch
    }
}

} // namespace ModeFlow::Gui
