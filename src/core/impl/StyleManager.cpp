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

namespace ModeFlow::Core {

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

    const std::array<QStringView, 4> preferredOrder = {u"windows11"_sv, u"windowsvista"_sv, u"windows"_sv,
                                                       u"fusion"_sv};

    for (QStringView key : preferredOrder) {
        const QString keyStr = key.toString();
        if (availableKeys.contains(keyStr)) {
            result.append(keyStr);
            availableKeys.remove(keyStr);
        }
    }

    for (const QString& key : availableKeys) {
        result.append(key);
    }

    return result;
}

QString qtStyleDisplayName(const QString& styleKey) {
    const QString key = normalizeStyleKey(styleKey);

    if (key == u"windows11"_s) {
        return QCoreApplication::translate("StyleManager", "Windows 11");
    }
    if (key == u"windowsvista"_s) {
        return QCoreApplication::translate("StyleManager", "Windows Vista");
    }
    if (key == u"windows"_s) {
        return QCoreApplication::translate("StyleManager", "Windows Classic");
    }
    if (key == u"fusion"_s) {
        return QCoreApplication::translate("StyleManager", "Fusion");
    }

    QString displayName = styleKey.trimmed();
    if (displayName.isEmpty()) {
        return QCoreApplication::translate("StyleManager", "Default");
    }

    displayName[0] = displayName[0].toUpper();
    return displayName;
}

} // namespace

StyleManager::StyleManager(Theme theme, const QString& qtStyleKey, QObject* parent)
    : QObject(parent), m_qtStyleKey(Utils::DefaultQtStyleKey.toString()) {
    setTheme(theme, qtStyleKey);
}

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

QList<ThemeData> StyleManager::availableThemes() const {
    QList<ThemeData> themes;

    themes.append({tr("Light"), Theme::Light, QString(), false});
    themes.append({tr("Dark"), Theme::Dark, QString(), false});

    const QList<QString> qtStyles = orderedQtStyleKeys();
    if (!qtStyles.isEmpty()) {
        themes.append({tr("System styles"), Theme::Light, QString(), true});
        for (const QString& styleKey : qtStyles) {
            themes.append({qtStyleDisplayName(styleKey), Theme::Qt, styleKey, false});
        }
    }

    return themes;
}

void StyleManager::applyToWindow(QWidget* window) {
    if (!window)
        return;

    if (m_currentTheme == Theme::Qt) {
        Gui::StyleUtils::resetWindowEffects(window);
        return;
    }

    window->setUpdatesEnabled(false);

    using namespace ModeFlow::Gui;
    bool isDark = (m_currentTheme == Theme::Dark);

    StyleUtils::extendFrame(window);
    StyleUtils::applyMica(window, isDark);
    StyleUtils::setImmersiveDarkMode(window, isDark);

    window->setUpdatesEnabled(true);
    window->update();

    StyleUtils::refreshFrame(window);
}

QStyle* StyleManager::createDefaultNativeStyle() const {
    if (QStyle* style = QStyleFactory::create(Utils::DefaultQtStyleKey.toString())) {
        return style;
    }
    if (QStyle* style = QStyleFactory::create(u"windowsvista"_s)) {
        return style;
    }
    return QStyleFactory::create(u"windows"_s);
}

void StyleManager::applyQtNativeTheme(const QString& styleKey) {
    const QString stylesheet = loadStylesheet(Styles::System);
    Gui::StyleBridge::instance().updateStyle(stylesheet);
    Gui::FontAwesome::invalidateCache();

    const QString targetKey = styleKey.isEmpty() ? Utils::DefaultQtStyleKey.toString() : styleKey;
    QStyle* style = QStyleFactory::create(targetKey);

    if (!style) {
        m_qtStyleKey = Utils::DefaultQtStyleKey.toString();
        style = createDefaultNativeStyle();
    }

    if (style) {
        qApp->setStyle(style);
    }
    qApp->setStyleSheet(stylesheet);
}

void StyleManager::applyFluentTheme(Theme theme) {
    if (QStyle* baseStyle = createDefaultNativeStyle()) {
        qApp->setStyle(baseStyle);
    }

    const bool isDark = (theme == Theme::Dark);
    QString stylesheet = loadStylesheet(Styles::Common);
    stylesheet += loadStylesheet(isDark ? Styles::Dark : Styles::Light);

    Gui::StyleBridge::instance().updateStyle(stylesheet);
    Gui::FontAwesome::invalidateCache();

    qApp->setStyleSheet(stylesheet);
}

void StyleManager::setTheme(Theme theme, const QString& qtStyleKey) {
    m_currentTheme = theme;
    if (!qtStyleKey.isEmpty()) {
        m_qtStyleKey = normalizeStyleKey(qtStyleKey);
    }

    if (theme == Theme::Qt) {
        applyQtNativeTheme(m_qtStyleKey);
    } else {
        applyFluentTheme(theme);
    }
}

Theme StyleManager::currentTheme() const {
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

} // namespace ModeFlow::Core
