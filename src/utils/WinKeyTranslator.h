#pragma once

#include <QTranslator>

namespace ModeFlow::Utils {

class WinKeyTranslator : public QTranslator {
    Q_OBJECT
public:
    QString translate(const char* context, const char* sourceText, const char* disambiguation = nullptr,
                      int n = -1) const override;
};

} // namespace ModeFlow::Utils