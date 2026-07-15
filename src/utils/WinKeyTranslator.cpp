#include "WinKeyTranslator.h"

namespace ModeFlow::Utils {

using namespace Qt::StringLiterals;

QString WinKeyTranslator::translate(const char* context, const char* sourceText, const char* disambiguation,
                                    int n) const {
    Q_UNUSED(disambiguation);
    Q_UNUSED(n);

    if (!sourceText)
        return QString();

    // Translate only in contexts responsible for graphical display
    const bool isGuiContext = (std::strcmp(context, "QShortcut") == 0) || (std::strcmp(context, "QKeySequence") == 0) ||
                              (std::strcmp(context, "QKeySequenceEdit") == 0);

    if (isGuiContext) {
        if (std::strcmp(sourceText, "Meta") == 0)
            return u"Win"_s;
        if (std::strcmp(sourceText, "Ctrl") == 0)
            return u"Ctrl"_s;
        if (std::strcmp(sourceText, "Shift") == 0)
            return u"Shift"_s;
        if (std::strcmp(sourceText, "Alt") == 0)
            return u"Alt"_s;
    }

    return QString();
}

} // namespace ModeFlow::Utils