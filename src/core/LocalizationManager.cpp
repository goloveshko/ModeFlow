
#include "LocalizationManager.h"

#include <QApplication>
#include <QFile>

#include "Logging.h"

namespace ModeFlow::Core {

using namespace Qt::StringLiterals;

LocalizationManager::LocalizationManager(QObject* parent) : QObject(parent) {}

QList<LanguageData> LocalizationManager::availableLanguages() const {
    return {{u"English"_s, u"en_US"_s}, {u"Русский"_s, u"ru_RU"_s}};
}

QString LocalizationManager::normalizedLocale(QStringView requested) {
    const QString locale = requested.toString().trimmed();
    if (locale == u"ru_RU"_s) {
        return u"ru_RU"_s;
    }

    return u"en_US"_s;
}

void LocalizationManager::switchLanguage(const QString& localeCode) {
    if (m_currentLocale == localeCode)
        return;

    const QString targetLocale = normalizedLocale(localeCode);

    qApp->removeTranslator(&m_translator);
    qApp->removeTranslator(&m_qtTranslator);
    qApp->removeTranslator(&m_winKeyTranslator);

    if (targetLocale == u"en_US"_s) {
        m_currentLocale = targetLocale;
        qApp->installTranslator(&m_winKeyTranslator);

        emit signalLanguageChanged();
        return;
    }

    QString path = u":/i18n/ModeFlow_%1.qm"_s.arg(targetLocale);

    if (!QFile::exists(path)) {
        qCWarning(lcCore) << "Translation file not found:" << path;
        emit translationError(tr("Translation file missing for %1").arg(targetLocale));
        m_currentLocale = u"en_US"_s;
        qApp->installTranslator(&m_winKeyTranslator);
        emit signalLanguageChanged();
        return;
    }

    if (m_translator.load(path)) {
        qApp->installTranslator(&m_translator);
        m_currentLocale = targetLocale;
    } else {
        qCWarning(lcCore) << "Failed to load translation:" << path;
        emit translationError(tr("Failed to load translation for %1").arg(targetLocale));
        m_currentLocale = u"en_US"_s;
    }

    // Load the Qt translation from the external "translations" folder
    QString lang = targetLocale.section('_', 0, 0);
    if (m_currentLocale == targetLocale && m_qtTranslator.load("qtbase_" + lang, ":/qt/translations")) {
        qApp->installTranslator(&m_qtTranslator);
    }

    qApp->installTranslator(&m_winKeyTranslator);

    emit signalLanguageChanged();
}
} // namespace ModeFlow::Core
