#pragma once

#include <QObject>
#include <QStringView>
#include <QTranslator>

#include "ConfigTypes.h"
#include "WinKeyTranslator.h"

namespace ModeFlow::Core {

class LocalizationManager : public QObject {
    Q_OBJECT

public:
    explicit LocalizationManager(QObject* parent = nullptr);

    QList<LanguageData> availableLanguages() const;

    static QString normalizedLocale(QStringView requested);

    void switchLanguage(const QString& localeCode);

    const QString& currentLocale() const { return m_currentLocale; }

signals:
    void signalLanguageChanged();
    void translationError(const QString& message);

private:
    QTranslator m_translator;
    QTranslator m_qtTranslator;
    Utils::WinKeyTranslator m_winKeyTranslator;
    QString m_currentLocale;
};
} // namespace ModeFlow::Core
