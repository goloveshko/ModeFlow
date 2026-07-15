#include "StyleManager.h"

#include <QApplication>
#include <QFile>
#include <QFileDialog>
#include <QPushButton>
#include <QStyle>
#include <QStyleFactory>

#include "Constants.h"
#include "FontAwesome.h"
#include "Logging.h"
#include "StyleBridge.h"
#include "StylePaths.h"
#include "StyleUtils.h"
#include "SystemUtils.h"

namespace ModeFlow::Services {

using namespace Qt::StringLiterals;

namespace {

QString normalizeStyleKey(const QString& styleKey) {
    return styleKey.trimmed().toLower();
}

QList<QString> orderedQtStyleKeys() {
    const QStringList rawKeys = QStyleFactory::keys();
    QSet<QString> availableKeys;
    for (const QString& rawKey : rawKeys) {
        const QString key = normalizeStyleKey(rawKey);
        if (!key.isEmpty()) {
            availableKeys.insert(key);
        }
    }
    QList<QString> result;

    const auto appendIfAvailable = [&result, &availableKeys](const QString& key) {
        if (availableKeys.contains(key) && !result.contains(key)) {
            result.append(key);
        }
    };

    appendIfAvailable(Utils::DefaultQtStyleKey.toString());
    if (availableKeys.contains(u"windows"_s)) {
        appendIfAvailable(u"windows"_s);
    } else {
        appendIfAvailable(u"windowsvista"_s);
    }
    appendIfAvailable(u"fusion"_s);

    return result;
}

QString qtStyleDisplayName(const QString& styleKey) {
    const QString key = normalizeStyleKey(styleKey);
    if (key == Utils::DefaultQtStyleKey) {
        return QCoreApplication::translate("StyleManager", "System");
    }
    if (key == u"windowsvista"_s || key == u"windows"_s) {
        return QCoreApplication::translate("StyleManager", "Windows");
    }
    if (key == u"fusion"_s) {
        return QCoreApplication::translate("StyleManager", "Fusion");
    }

    QString displayName = styleKey.trimmed();
    if (displayName.isEmpty()) {
        return QCoreApplication::translate("StyleManager", "System");
    }

    displayName[0] = displayName[0].toUpper();
    return displayName;
}

} // namespace

StyleManager::StyleManager(QObject* parent) : QObject(parent), m_qtStyleKey(Utils::DefaultQtStyleKey.toString()) {}

QString StyleManager::loadStylesheet(QStringView fileName) const {
    QFile styleFile(fileName.toString());
    if (styleFile.open(QFile::ReadOnly | QFile::Text)) {
        QString stylesheet = QString::fromUtf8(styleFile.readAll());
        styleFile.close();
        return stylesheet;
    } else {
        qCWarning(lcService) << "Failed to load stylesheet:" << fileName << styleFile.errorString();
        return QString();
    }
}

QList<Core::ThemeData> StyleManager::availableThemes() const {
    QList<Core::ThemeData> themes;

    themes.append({tr("Light"), Core::Theme::Light, QString(), false});
    themes.append({tr("Dark"), Core::Theme::Dark, QString(), false});

    const QList<QString> qtStyles = orderedQtStyleKeys();
    if (!qtStyles.isEmpty()) {
        themes.append({tr("System styles"), Core::Theme::Light, QString(), true});
        for (const QString& styleKey : qtStyles) {
            themes.append({qtStyleDisplayName(styleKey), Core::Theme::Qt, styleKey, false});
        }
    }

    return themes;
}

void StyleManager::applyToWindow(QWidget* window) {
    if (m_currentTheme == Core::Theme::Qt)
        return;

    if (!window)
        return;

    window->setUpdatesEnabled(false);

    using namespace ModeFlow::Gui;
    bool isDark = (m_currentTheme == Core::Theme::Dark);

    StyleUtils::extendFrame(window);
    StyleUtils::applyMica(window, isDark);
    StyleUtils::setImmersiveDarkMode(window, isDark);

    window->setUpdatesEnabled(true);
    window->update();

    StyleUtils::refreshFrame(window);
}

void StyleManager::setTheme(Core::Theme theme, const QString& qtStyleKey) {
    m_currentTheme = theme;
    if (!qtStyleKey.isEmpty()) {
        m_qtStyleKey = normalizeStyleKey(qtStyleKey);
    }

    // For Qt themes, use standard Windows style and remove window effects
    if (theme == Core::Theme::Qt) {
        const QString stylesheet = loadStylesheet(Core::Styles::System);
        Gui::StyleBridge::instance().updateStyle(stylesheet);
        // Clear themed glyphs before Qt emits palette/style change events.
        Gui::FontAwesome::invalidateCache();

        const QString styleKey = m_qtStyleKey.isEmpty() ? Utils::DefaultQtStyleKey.toString() : m_qtStyleKey;
        if (QStyle* style = QStyleFactory::create(styleKey)) {
            qApp->setStyle(style);
        } else {
            m_qtStyleKey = Utils::DefaultQtStyleKey.toString();
            qApp->setStyle(QStyleFactory::create(Utils::DefaultQtStyleKey.toString()));
        }
        qApp->setStyleSheet(stylesheet);
        return;
    }

    bool isDark = theme == Core::Theme::Dark;

    QString stylesheet = loadStylesheet(Core::Styles::Common);
    stylesheet += loadStylesheet(isDark ? Core::Styles::Dark : Core::Styles::Light);

    Gui::StyleBridge::instance().updateStyle(stylesheet);
    // Widgets can refresh icons while the new stylesheet is being applied,
    // so invalidate the cache before Qt starts dispatching theme-related events.
    Gui::FontAwesome::invalidateCache();

    qApp->setStyleSheet(stylesheet);
}

Core::Theme StyleManager::currentTheme() const {

    return m_currentTheme;
}

QString StyleManager::currentQtStyleKey() const {
    return m_qtStyleKey;
}

int StyleManager::showMessageBox(QWidget* parent, QMessageBox::Icon icon, const QString& title, const QString& text,
                                 const QString& informativeText, const QStringList& buttons, int defaultButtonIndex) {
    QMessageBox msg(icon, title, text, QMessageBox::NoButton, parent);

    if (!informativeText.isEmpty()) {
        msg.setInformativeText(informativeText);
    }

    QList<QPushButton*> addedButtons;

    if (buttons.isEmpty()) {
        addedButtons.append(msg.addButton(QMessageBox::Ok));
    } else {
        for (const auto& btnText : buttons) {
            addedButtons.append(msg.addButton(btnText, QMessageBox::AcceptRole));
        }

        if (defaultButtonIndex >= 0 && defaultButtonIndex < addedButtons.size()) {
            msg.setDefaultButton(addedButtons[defaultButtonIndex]);
        }
    }

    applyToWindow(&msg);
    msg.exec();

    QAbstractButton* clicked = msg.clickedButton();
    for (int i = 0; i < addedButtons.size(); ++i) {
        if (static_cast<QAbstractButton*>(addedButtons[i]) == clicked) {
            return i;
        }
    }

    return -1;
}

void StyleManager::showInfo(QWidget* parent, const QString& title, const QString& text) {
    showMessageBox(parent, QMessageBox::Information, title, text);
}

void StyleManager::showWarning(QWidget* parent, const QString& title, const QString& text) {
    showMessageBox(parent, QMessageBox::Warning, title, text);
}

void StyleManager::showError(QWidget* parent, const QString& title, const QString& text) {
    showMessageBox(parent, QMessageBox::Critical, title, text);
}

bool StyleManager::confirmAction(QWidget* parent, const QString& title, const QString& text) {
    QMessageBox msg(QMessageBox::Question, title, text, QMessageBox::Yes | QMessageBox::No, parent);
    applyToWindow(&msg);
    return msg.exec() == QMessageBox::Yes;
}

QString StyleManager::getOpenFileName(QWidget* parent, const QString& caption, const QString& dir,
                                      const QString& filter) {
    QFileDialog dialog(parent, caption, dir, filter);
    dialog.setOption(QFileDialog::DontUseNativeDialog, true);

    dialog.setSizeGripEnabled(false);

    // Inject Mica backdrop, borders, and rounded corners natively
    applyToWindow(&dialog);

    if (dialog.exec() == QDialog::Accepted) {
        return dialog.selectedFiles().value(0);
    }
    return QString();
}

QString StyleManager::getSaveFileName(QWidget* parent, const QString& caption, const QString& dir,
                                      const QString& filter) {
    QFileDialog dialog(parent, caption, dir, filter);
    dialog.setOption(QFileDialog::DontUseNativeDialog, true);
    dialog.setAcceptMode(QFileDialog::AcceptSave);

    dialog.setSizeGripEnabled(false);

    // Inject Mica backdrop, borders, and rounded corners natively
    applyToWindow(&dialog);

    if (dialog.exec() == QDialog::Accepted) {
        return dialog.selectedFiles().value(0);
    }
    return QString();
}

void StyleManager::forceUnhover() {
    if (QWidget* trigger = QApplication::widgetAt(QCursor::pos())) {
        ModeFlow::Gui::StyleUtils::forceUnhover(trigger);
    }
}

} // namespace ModeFlow::Services
